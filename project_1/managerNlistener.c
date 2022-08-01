#include "./managerUtilities/managerUtilities.h"


extern int errno;

static int flagInt; // used in signal handler for SIGINT
static int flagChld; // used in signal handler for SIGCHLD


static void catchInt(int sigint){
  // signal handler for SIGINT
  flagInt=1;
}

static void catchHLD(int sighld){
  // signal handler for SIGCHLD
  flagChld = 1;
}


int main ( int argc, char *argv[] ){
  int pipefd[2];
  int pid,status;

  // initialize the global variables
  flagInt=0;
  flagChld=0;

  // setup the signal handlers
  struct sigaction intHandler = {0};
  intHandler.sa_handler = catchInt ;
  sigaction(SIGINT, &intHandler , NULL );

  struct sigaction hldHandler = {0};
  hldHandler.sa_handler = catchHLD ;
  sigaction(SIGCHLD,&hldHandler,NULL);

  // create the pipe for the "communication" between manager and listener
  if (pipe(pipefd)==-1){
    perror("manager: pipe failed");
    exit(1);
  }

  char *watchingDir = NULL; // to store the given directory from the command line for the inotifywait

  if(argc >= 2){
    if( strcmp(argv[1],"-p")==0 && argc == 3){
      watchingDir = malloc( (strlen(argv[2]) + 1 ) * sizeof(char) );
      if (watchingDir == NULL) {
        perror("malloc : memory not available");
        exit(2);
      }
      strcpy(watchingDir,argv[2]);
    }
    else {
      fprintf(stderr, "Usage: %s [-p path]\n",argv[0]);
      exit(1);
    }
  }

  if(watchingDir != NULL){
    printf("> inotifywait watches folder: %s <\n",watchingDir);
  }
  else{
    printf("> inotifywait watches current folder (default) <\n");
  }

  int pidListener;
  pid = fork();
  if(pid < 0) {
    perror("manager: fork() failed");
    exit(1);
  }
  else if (pid == 0) { // listener
    dup2(pipefd[1], STDOUT_FILENO); // replace standard output with output part of pipe
    close(pipefd[1]);
    close(pipefd[0]);

    signal(SIGINT, SIG_IGN); // ignore SIGINT, listener should "ends" with SIGKILL

    if(watchingDir != NULL){ // directory given from the command line
      if( execlp("inotifywait", "inotifywait","-m", watchingDir,"-e","create","-e", "moved_to", (char *)NULL) == -1 ){
        perror("listener: inotifywait failed!");
        exit(1);
      }
    }
    else{ // no command line argument, inotifywait watch the project's/current folder
      if( execlp("inotifywait", "inotifywait","-m", ".","-e","create","-e", "moved_to", (char *)NULL) == -1 ){
        perror("listener: inotifywait failed!");
        exit(1);
      }
    }

    exit(0);
  }
  else{ // parent / manager
    pidListener=pid; // save listener's pid
    close(pipefd[1]);
  }

  printf("> Manager's pid: %d <\n",getpid());
  printf("> Listener's pid: %d <\n",pidListener);


  queue *queue=initQueue(); // queque that used to store the available workers
  hashtable *htWorkers=htInitialize(30); // initialize and allocate the hashtable (with 30 buckets) in order to save the workers pids, the correspondings fifos (named pipes) and the file descriptors of them


  char *buffer = NULL;
  int firstTime=1;
  int numberOfWorkers=0;
  int namedPipesCounter=0;


  while(1){
    if (flagInt){ // check if SIGINT has been received
      // then kill all the forked processes
      while(queue->numberOfNodes != htWorkers->numberOfNodes){ // wait until all workers finish their "jobs"
        sleep(1);
        if(flagChld){ // check if SIGCHLD received
          flagChld = 0;  // set the corresponding flag in to zero (false)
          collectAvailableWorkers(queue); // push all available workers in to queue
        }
      }

      printf("> Queue nodes: %d <\n",queue->numberOfNodes);
      printf("> Hashtable nodes: %d <\n",htWorkers->numberOfNodes);
      queue = deleteQueue(queue); // delete queque
      kill(pidListener, SIGKILL); // kill listener
      printf("> Listener with pid: %d killed <\n",pidListener);
      htDelete(htWorkers); // kill workers, close fds, close the fifos and delete the hash table
      break;
    }


    if(buffer != NULL){
      free(buffer);
      buffer = NULL;
    }

    printf("> WAIT FOR FILE (MOVE/CREATE) <\n");
    int bytesAv=0;
    int bytesRead;
    char byte;

    // read from the pipe whatever inotifywait has written

    if ( (bytesRead = read(pipefd[0],&byte,1) ) <= 0) { // read 1 byte from the pipe
      perror("manager");
    }

    if(bytesRead<=0 && errno == EINTR){ // check if read() interrupted
      // yes, read() system call has been interrupted by a signal
      if(flagChld){ // check if SIGCHLD received
        flagChld = 0; // set the corresponding flag in to zero (false)
        collectAvailableWorkers(queue); // push all available workers in to queue
      }
      continue;
    }

    ioctl(pipefd[0],FIONREAD,&bytesAv); // after take the available bytes from the pipe

    buffer=malloc(bytesAv+2); // allocate the buffer in order to take the "message" from the pipe (+1 for the '\0')

    if (buffer == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    for(int i=0;i<bytesAv+2;i++) // initialize the buffer
      buffer[i]='\0';

    buffer[0] = byte; // store the byte that already received/read


    if (read(pipefd[0],buffer+1,bytesAv) <= 0) { // read the rest bytes from the pipe and save these in to buffer
      perror("manager");
    }


    char *temp = malloc( (strlen(buffer) + 1) * sizeof(char) );
    if (temp == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    char* fileNameTemp;
    int filesNum=0;

    // count the number of file names that received
    strcpy(temp,buffer);
    fileNameTemp = strtok (temp,"\n");
    while (fileNameTemp != NULL){
      filesNum++;
      fileNameTemp = strtok (NULL, "\n");
    }

    char **fileNames; // in order to save the files names

    fileNames = malloc( (filesNum) * sizeof(char*) );
    if (fileNames == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    // separate the file names and save these
    filesNum=0;
    strcpy(temp,buffer);
    fileNameTemp = strtok (temp,"\n");
    while (fileNameTemp != NULL){
      fileNames[filesNum] = malloc( (strlen(fileNameTemp) + 1) * sizeof(char));
      if (fileNames[filesNum] == NULL) {
        perror("malloc : memory not available");
        exit(2);
      }
      strcpy(fileNames[filesNum++],fileNameTemp);
      fileNameTemp = strtok (NULL, "\n");
    }

    free(temp);

    // print the file names that received from the inotifywait through the pipe
    printf("-----------------\n");
    printf("> Files: \n");
    for(int i = 0 ; i<filesNum ; i++ ){
      printf("File no.%d -> %s\n",i+1,fileNames[i]);
    }
    printf("-----------------\n");


    if(!firstTime && !flagInt){ // check if this is not the first time and the SIGINT not received (flagInt)
      for(int i = 0; i < filesNum; i++){ // for every file name that received
        if(empty(queue)){ // check if workers queue is empty
          // then there aren't available workers, should fork worker
          printf("> Queque is empty, should fork worker <\n");
          forkWorkers(htWorkers,queue, 1, &namedPipesCounter); // fork one worker, store his pid, fd and fifo name in to hash table and push him in to the queue
        }

        popNSend(htWorkers,queue,fileNames[i]); // pop the worker's pid from the queue and send him the file name

        if(flagChld){ // check if SIGCHLD received
          flagChld = 0; // set the corresponding flag in to zero (false)
          collectAvailableWorkers(queue); // push all available workers in to queue
        }

      }
    }

    if(firstTime && !flagInt){ // in first time create/fork as many workers as there are the given file names (from the inotifywait)
      numberOfWorkers=filesNum;

      forkWorkers(htWorkers,queue, numberOfWorkers,&namedPipesCounter); // fork as many workers as there are the given file names, store their pids, fds and fifos names in to hash table and push them in to the queue

      for(int i = 0; i < filesNum; i++){
        popNSend(htWorkers,queue,fileNames[i]); // pop the worker's pid from the queue and send him the file name
      }
      firstTime=0;
    }

    // free all allocate space for the file names
    for (int i = 0; i < filesNum; i++)
      free(fileNames[i]);
    free(fileNames);

  }


  int child;
  while ( (child = wait(&status)) > 0);

  close(pipefd[0]); // close pipe's file descriptor

  // free all allocated space
  if(watchingDir != NULL)
    free(watchingDir);

  free(htWorkers);
}

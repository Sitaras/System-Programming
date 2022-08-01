#include "managerUtilities.h"
#define PERMS 0666


void forkWorkers(hashtable *ht,queue *q,int numberOfWorkers,int *namedPipesCounter){
  // create the name pipe/s (fifo) for the "communication" between manager and worker/s.
  // After, fork and exec the workers.
  // Finally, manager opens fifo in order to write,
  // push the worker's pid in to queue and
  // store in to hash table the worker's pid, the fifo's name and the corresponding fd


  int writeFifo,pid;

  // create the named pipe/s for the "communication" between manager and worker/s.
  char** fifosNames;
  fifosNames = malloc((numberOfWorkers) * sizeof(char*));

  if (fifosNames == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  for (int i = 0; i < (numberOfWorkers); i++){
    fifosNames[i] = malloc( 100 * sizeof(char) );
    if (fifosNames[i] == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }
  }

  for(int i = 0; i < numberOfWorkers; i++) {
    sprintf(fifosNames[i], "/tmp/fifo.%d", (*namedPipesCounter)++);

    if ( (mkfifo(fifosNames[i], PERMS) < 0) && (errno != EEXIST) ) {
      perror("can't create fifo");
    }

  }


  for(int i = 0; i < numberOfWorkers; i++) {
    pid = fork();
    if(pid < 0) {
      perror("manager: fork() failed");
      exit(1);
    }
    else if (pid == 0) { // exec worker
      if(execl("./worker", "./worker", fifosNames[i], NULL)==-1){
        perror("worker failed!");
        exit(1);
      }

      exit(0);
    }
    else{ // parent / manager
      // open the fifo ( Open for writing only )
      // avoid to interrupted from signal
      while ( ( writeFifo = open(fifosNames[i], O_WRONLY ) ) == -1 && errno == EINTR)
        continue;

      if(writeFifo < 0){ // error checking
        perror("manager: Could not open write fifo");
      }
      pushQueue(q,pid); // push worker's pid in to queue
      htInsert(ht,pid,fifosNames[i],writeFifo); // store in to hash table the worker's pid, the fifo's name and the corresponding fd
    }
  }

  for (int i = 0; i < numberOfWorkers; i++)
    free(fifosNames[i]);
  free(fifosNames);

}



void popNSend(const hashtable *ht,queue *q,char* fileName){
  // pop the worker's pid from the queue and send him the file name

  int pidTemp = popQueue(q); // pop the worker's pid from the queue

  kill(pidTemp,SIGCONT); // the corresponding worker is stopped (SIGSTOP), send SIGCONT in order to continue

  // then send him the file name

  printf("> Worker with pid %d popped from queque <\n",pidTemp);

  printf("> Manager sends message : %s, Bytes: %ld to Worker with pid: %d <\n", fileName,strlen(fileName),pidTemp);


  if(htSearch(ht,pidTemp) == NULL){
    fprintf(stderr, "manager: worker not found in to the hash table. Exit to avoid segmentation fault \n");
    exit(1);
  }

  int sizeOfString = strlen(fileName); // store file name's size/length
  int bytes;

  while ( (bytes = write(htSearch(ht,pidTemp)->fifoWriteFd, &sizeOfString, sizeof(int) ) ) == -1 && errno == EINTR) // send the size/length of file name at the corresponding worker (avoid to interrupted from signal)
    continue;

  if(bytes != sizeof(int))
    perror("manager: file name write error");

  while ( (bytes = write(htSearch(ht,pidTemp)->fifoWriteFd, fileName, strlen(fileName)) ) == -1 && errno == EINTR) // send at the corresponding worker the file name (avoid to interrupted from signal)
    continue;

  if(bytes != strlen(fileName))
    perror("manager: file name write error");
}



void collectAvailableWorkers(queue *queue){
  int pidTemp, status;
  while ( ( pidTemp = waitpid( -1, &status,  WUNTRACED | WNOHANG ) ) > 0 ){ // "catch" all workers (childs) that stopped (with SIGSTOP) and push them in to the queue
    if(WIFSTOPPED(status)){ // check for SIGSTOP only
      // then worker has been stopped with SIGSTOP (is available), push it in to queue
      pushQueue(queue,pidTemp);
      printf("> Worker with pid %d pushed in to queque <\n",pidTemp);
    }
  }
}

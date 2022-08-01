#include "dataServerUtilities.h"
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

extern pthread_mutex_t queueMtx;
extern pthread_cond_t nonEmpty;
extern pthread_cond_t nonFull;

extern queue *q;

void* communicationThread(void* newsock){
  int clientFd=*(int *) newsock;
  int sizeOfString=0;

  if (read(clientFd,&sizeOfString,sizeof(int)) <= 0) { // read wanted directory name's size from the client
    perror("server: int read error");
  }

  char* buffer=malloc((sizeOfString+1)*sizeof(char)); // allocate the buffer in order to read the directory name from the client

  if (buffer == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  for(int i=0;i<sizeOfString+1;i++) // initialize the buffer
    buffer[i]='\0';

  if ( read(clientFd, buffer, sizeOfString) <= 0) { // read the wanted directory name from the client
    perror("server: file name read error");
  }

  printf("[Communication Thread: %ld]: About to scan directory %s.\n",pthread_self(),buffer);

  openDirectoryAndPushFiles(buffer,q,clientFd); // open the wanted directory and push the file paths/names that has at the queue

  // free allocate space
  free(buffer);
  free(newsock);
  newsock=NULL;

  printf("[Communication Thread: %ld]: just ended.\n",pthread_self());
  pthread_exit(NULL);
}


void openDirectoryAndPushFiles(char* dirOrFile,queue* queue,int clientFd){
  // this function works recursively with the extractDirectoryContents()
  // in order to extract the files that has the directory
  // and push them in to queue

  // in order to check if it is file or directory
  struct stat pathForStat;
  stat(dirOrFile, &pathForStat);

  if(S_ISREG(pathForStat.st_mode)){ // check if given path concerns a file

    int err;
    if ((err = pthread_mutex_lock(&queueMtx))) { // lock queue's mutex
      perror2("pthread_mutex_lock", err);
      exit(1);
    }

    // try to push file path and the file descriptor of the corresponding client in to queue,
    // if function returns 1 this means that the queue if full.
    // if function returns 0 this means that a new node just pushed.
    while(pushQueue(queue,clientFd,dirOrFile)){
      // function returns 1 -> queue if full, so...
      printf("[Communication Thread: %ld]: suspended, Queue is full.\n",pthread_self());
      pthread_cond_wait(&nonFull, &queueMtx); // suspend the communication thread
    }

    printf("[Communication Thread: %ld]: Added file %s to the queue.\n",pthread_self(),dirOrFile);

    pthread_cond_signal(&nonEmpty); // awake a worker thread (if it was suspended because queue was empty)

    if ((err = pthread_mutex_unlock(&queueMtx))) { // unlock queue's mutex
      perror2("pthread_mutex_unlock", err);
      exit(1);
    }

    return;
  }
  else if (S_ISDIR(pathForStat.st_mode)){ // check if given path concerns a directory
    DIR *dir = opendir(dirOrFile);
    if (dir == NULL){
      perror("opendir:");
    }
    extractDirectoryContents(dir,dirOrFile,queue,clientFd); // then extract the contents (files and directories) of the directory
    closedir(dir);
  }
  else{
    perror("server: not file or directory:");
  }

}


void extractDirectoryContents(DIR *dir,char* dirName,queue* queue,int clientFd){
  // this function works recursively with the openDirectoryAndPushFiles().
  // extract all the contents (files and directories) of the given directory

  struct dirent *directoryEntry;
  char path[4096];

  while ((directoryEntry = readdir(dir)) != NULL){
    if(strcmp(directoryEntry->d_name,".")==0 || strcmp(directoryEntry->d_name,"..")==0 )
      continue;

    strcpy(path, dirName);
    strcat(path, "/");
    strcat(path, directoryEntry->d_name);

    openDirectoryAndPushFiles(path,queue,clientFd);
  }

}


void sendFile(int clientFd,char* filePath,int blockSize){
  // send the corresponding file to the client

  int sizeOfString = strlen(filePath); // store file path's size/length

  if (write(clientFd, &sizeOfString, sizeof(int)) < 0) { // send the size/length of file path at the corresponding client
    perror("server: int write error");
    exit(1);
  }

  if (write(clientFd, filePath, strlen(filePath)) < 0) { // send at the corresponding client the file path
    perror("server: string write error");
    exit(1);
  }

  if (write(clientFd, &blockSize, sizeof(int)) < 0) { // send at the corresponding client the block size
    perror("server: int write error");
    exit(1);
  }

  sendFileData(clientFd,filePath,blockSize); // send at the corresponding client the file's data per block

}



void sendFileData(int clientFd,char* filePath,int blockSize){
  // read file's data (from the given file) and send these to the corresponding client per block.

  int fileFd;
  if ( (fileFd = open(filePath, O_RDONLY)) < 0)  { // open the corresponding file and get the file descriptor in order to read from it
    perror("server: can't open given file");
  }

  // "calculate" the file's size
  int fileSize = lseek(fileFd, 0, SEEK_END);
  lseek(fileFd, 0, SEEK_SET);


  if (write(clientFd, &fileSize, sizeof(int)) < 0) { // send the file's size to the client
    perror("server: int write error ");
    exit(1);
  }


  char *fileData=malloc(blockSize*sizeof(char));
  int sumOfBytes = 0; // to store the sum of bytes that server has sent
  int bytes;

  while(sumOfBytes < fileSize){ // until all file's data sent to the client
    int readWriteBytes;
    if( fileSize-sumOfBytes < blockSize){ // check if the remaining bytes are less than the block size
      // then send only the remaining bytes
      readWriteBytes=fileSize-sumOfBytes;
    }
    else{ // at this case the remaining bytes are greater (or equal) than the block size
      // so send block size bytes
      readWriteBytes=blockSize;
    }

    if (read(fileFd,fileData,readWriteBytes)<=0) { // read the bytes from the file
      perror("server: file read error");
    }

    if ((bytes=write(clientFd,fileData,readWriteBytes)) < 0) { // and send these bytes at the corresponding client
      perror("server: write error");
      exit(1);
    }

    sumOfBytes+=bytes; // add at the sum the bytes that sent
  }

  free(fileData);

}

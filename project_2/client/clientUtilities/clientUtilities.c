#include "clientUtilities.h"

char* readFileData(int clientFd, int *sizeOfFile){
  // read file data from the server (per block) and return whole data of the corresponding file (file descriptor given)

  int fileSize;
  int blockSize;

  if (read(clientFd,&blockSize,sizeof(int)) <= 0) { // read from the server the block size
    perror("client: int read error");
  }

  if (read(clientFd,&fileSize,sizeof(int)) <= 0) { // read from the server the file's size
    perror("client: int read error");
  }

  *sizeOfFile=fileSize;

  char* fileData=malloc(fileSize*sizeof(char)); // allocate a "buffer" in order to read the file's data

  if (fileData == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  int sumOfBytes=0; // to store the sum of bytes that client has received
  int bytes;

  while(sumOfBytes < fileSize){ // until all file data received

    char *data;
    int readBytes;
    if( fileSize-sumOfBytes < blockSize ){ // check if the remaining bytes (to read) are less than the block size
      // then read only the remaining bytes
      readBytes=fileSize-sumOfBytes;
    }
    else{ // at this case the remaining bytes ara greater (or equal) than the block size
      // so read block size bytes
      readBytes=blockSize;
    }

    data=malloc(readBytes*sizeof(char));

    if (data == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    if ((bytes=read(clientFd,data,readBytes)) < 0) { // read data bytes from the server
      perror("client: read error");
      exit(1);
    }

    memcpy(fileData + sumOfBytes, data, bytes); // append these bytes at the buffer
    sumOfBytes+=bytes; // add at the sum the bytes that read

    free(data);
  }

  return fileData;
}


void createDirsAndFiles(char *filePath,char *fileData,int fileSize){
  // create the file (with his data) and the corresponding directories according to the given file path

  char *tempFileData=malloc((strlen(filePath)+1)*sizeof(char));
  strcpy(tempFileData,filePath);

  if (tempFileData == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  char *tempDirPath=malloc((strlen(filePath)+1)*sizeof(char));

  if (tempDirPath == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  // count the number of slashes
  int counter=0;
  char *temp = strtok(tempFileData,"/");

  while (temp != NULL){
    counter++;
    temp = strtok (NULL, "/");
  }

  strcpy(tempFileData,filePath);
  temp = strtok(tempFileData,"/");

  int numberOfSlashes=counter;
  counter=0;

  while (temp != NULL){
    counter++;
    if(counter == numberOfSlashes){ // check if this is the last slash
      // then temp has the file name, so...

      unlink(filePath); // if file already exists, remove it

      int fileCreateFd=open(filePath, O_WRONLY | O_CREAT, 0644); // create file

      if(fileCreateFd < 0){ // error checking
        perror("Create file: ");
      }

      if (write(fileCreateFd, fileData, fileSize) != fileSize) { // write the received data at the file
        perror("client: couldn't write in to the file");
      }

      close(fileCreateFd); // close file descriptor
      break;
    }

    // just form every time the corresponding path in order to create the directories and finally the file
    if(counter == 1){
      strcpy(tempDirPath,temp);
    }
    else{
      strcat(tempDirPath,"/");
      strcat(tempDirPath,temp);
    }

    struct stat st = {0};
    if (stat(tempDirPath, &st) == -1) { // check if the directory not exists
      // then create it
      mkdir(tempDirPath, 0700);
    }

    temp = strtok (NULL, "/");
  }

  // free the allocated space
  free(tempFileData);
  free(fileData);
  free(tempDirPath);

}

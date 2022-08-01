#include "clientUtilities/clientUtilities.h"


int main ( int argc, char *argv[] ){

  if(argc < 7 || argc >= 8){
    fprintf(stderr, "Usage: %s -i <server_ip> -p <server_port> -d <directory>\n",argv[0]);
    exit(1);
  }

  char* serverIP;
  char* directory;
  int port;

  printf("Client’s parameters are:\n");
  for(int i = 1 ; i < argc ; i++){
    if(strcmp(argv[i],"-i")==0 && (argc > i+1)){
      serverIP=malloc((strlen(argv[i+1])+1)*sizeof(char));
      if (serverIP == NULL) {
        perror("malloc : memory not available");
        exit(2);
      }
      strcpy(serverIP,argv[i+1]);
      printf("serverIP: %s\n", serverIP);
    }
    else if(strcmp(argv[i],"-p")==0 && (argc > i+1)){
      port=atoi(argv[i+1]);
      printf("port: %d\n", port);
    }
    else if(strcmp(argv[i],"-d")==0 && (argc > i+1)){
      directory=malloc((strlen(argv[i+1])+1)*sizeof(char));
      if (directory == NULL) {
        perror("malloc : memory not available");
        exit(2);
      }
      strcpy(directory,argv[i+1]);
      printf("directory: %s\n", directory);
    }
  }

  int sock;
  unsigned int serverlen;
  struct sockaddr_in server;
  struct sockaddr *serverptr;
  struct hostent *rem;

  if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) { // Create TCP socket
    perror("socket");
    exit(1);
  }

  if ((rem = gethostbyname(serverIP)) == NULL) { // Find server address
    perror("gethostbyname");
    exit(1);
  }

  server.sin_family = PF_INET; // Internet domain
  bcopy((char *) rem -> h_addr, (char *) &server.sin_addr,rem -> h_length);
  server.sin_port = htons(port); // Server’s Internet address and port
  serverptr = (struct sockaddr *) &server;
  serverlen = sizeof server;

  if (connect(sock, serverptr, serverlen) < 0) { // Request connection
    perror("connect");
    exit(1);
  }
  printf("Connecting to %s on port %d\n", serverIP, port);

  int sizeOfString = strlen(directory);

  if (write(sock, &sizeOfString, sizeof(int)) < 0) {
    perror("write");
    exit(1);
  }

  if (write(sock, directory, strlen(directory)) < 0) {
    perror("write");
    exit(1);
  }


  int endOfCom=0; // used to update/inform the client if his communication with the server over or not, every time the value will be sended from the server.
                  // (0 -> the communication, between the server and the client, doesn't end yet, then the client is ready to receive the file that is in the wanted directory |
                  //  1-> the communication, between the server and the client, just ended)

  char *finalDir=malloc((strlen(directory)+1)*sizeof(char)); // to store the "final" directory that the client actually wants to receive from server and not the path to it,
                                                             // p.ex. path for the wanted directory: "server/Folder1" | wanted directory: "Folder1"
  if (finalDir == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  // take from the given path the "final" directory and store it
  strcpy(finalDir,directory);
  char *temp = strtok(directory,"/");
  while (temp != NULL){
    strcpy(finalDir,temp);
    temp = strtok (NULL, "/");
  }

  while(!endOfCom){ // until the communication with the server ends

    if (read(sock,&endOfCom,sizeof(int)) <= 0) { // read the int flag from the server
      perror("worker: int read error");
    }

    if(endOfCom){// check if the flag is ON (equal with 1)
       // then exit from the loop,
       // the communication with the server just ended
      break;
    }

    if (read(sock,&sizeOfString,sizeof(int)) <= 0) { // read file path's size from the server
      perror("worker: int read error");
    }

    char* filePath=malloc((sizeOfString+1)*sizeof(char)); // allocate the buffer in order to read the file path/name

    if (filePath == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    for(int i=0;i<sizeOfString+1;i++) // initialize the buffer
      filePath[i]='\0';

    if ( read(sock, filePath, sizeOfString) <= 0) { // read the file name/path from the server
      perror("worker: file name/path read error");
    }

    int fileSize;
    char *fileData=readFileData(sock,&fileSize); // read file's data from the server

    char *finalFilePath=malloc((strlen(filePath)+1)*sizeof(char)); // store the actually wanted path
    if (finalFilePath == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }
    strcpy(finalFilePath,finalDir);

    // from the file path that the client received from the server
    // skip the unnecessary part and save/store only the wanted path.
    // p.ex command line argument: -d server/Folder1  | received file path: "server/Folder1/file1" | wanted path: "Folder1/file1" ---> store this to the finalFilePath
    char *temp = strtok(filePath,"/");
    int concatFlag=0;
    while (temp != NULL){
      if(concatFlag){
        // form the wanted file path
        strcat(finalFilePath,"/");
        strcat(finalFilePath,temp);
      }
      if(strcmp(temp,finalDir)==0){ // skip all the unnecessary part of path until wanted directory found
        concatFlag=1; // then begin to form the wanted file path
      }
      temp = strtok (NULL, "/");
    }

    createDirsAndFiles(finalFilePath,fileData,fileSize); // finally, create the file that has been received from the server (and the corresponding directories)
    printf("Received: %s\n",finalFilePath);

    // free the allocated space
    free(filePath);
    free(finalFilePath);
  }


  // free the allocated space
  free(directory);
  free(serverIP);
  free(finalDir);

  close(sock);

  return 0;

}

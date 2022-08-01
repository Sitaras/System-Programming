#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <dirent.h>
#include "./hashTableUrl/hashTableUrl.h"


int main ( int argc, char *argv[] ){
  if (argc < 2)
    exit(1);

  int fifoReadFd;
  signal(SIGINT, SIG_IGN); // ignore SIGINT, worker should "terminates" with SIGKILL

  printf("> Worker just created with pid: %d <\n",getpid());

  char *dir;
  char action[100];
  char *fileName;
  char *temp;
  char *buffer;
  int firstTime=0;

  if ( (fifoReadFd = open(argv[1], O_RDONLY))  < 0)  { // open the fifo (Open for reading only)
    perror("worker: can't open read fifo");
  }

  while (1) {
    if(firstTime){ // not pause the process at the first time
      kill(getpid(), SIGSTOP); // pause the process
    }
    firstTime=1;
    hashtableUrl *htUrls=htInitializeUrl(50); // initialize and allocate the hashtable (with 50 buckets)
                                              // in order to save and count the appearances of the urls locations
                                              // contained in the file that will be given by fifo.
                                              // Finally, write the appearances of the urls locations (that are saved in to hash table)
                                              // at a new .out file.

    int sizeOfString; // to store file name's length/size

    if (read(fifoReadFd,&sizeOfString,sizeof(int)) <= 0) { // read file name's length/size
      perror("worker: int read error");
    }

    printf("> Worker will receive a file name with length/size: %d <\n",sizeOfString);

    buffer=malloc((sizeOfString+1)*sizeof(char)); // allocate the buffer in order to read the file name

    if (buffer == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    for(int i=0;i<sizeOfString+1;i++) // initialize the buffer
      buffer[i]='\0';

    if ( read(fifoReadFd, buffer, sizeOfString) <= 0) { // read the file name
      perror("worker: file name read error");
    }

    printf("> Worker with pid: %d received the message : %s\n",getpid(),buffer);

    // separate the message in order to get the directory name, the action and the file name
    int loopCounter=0;
    temp = strtok(buffer," ");
    while (temp != NULL){
      loopCounter++;
      if (loopCounter==1){ // store the directory name
        dir = malloc( (strlen(temp)+1) * sizeof(char) );
        if (dir == NULL) {
          perror("malloc : memory not available");
          exit(2);
        }
        strcpy(dir,temp);
      }
      else if (loopCounter==2) // store the action
        strcpy(action,temp);
      else if (loopCounter==3){ // store the file name
        fileName = malloc( (strlen(temp)+1) * sizeof(char) );
        if (fileName == NULL) {
          perror("malloc : memory not available");
          exit(2);
        }
        strcpy(fileName,temp);
      }

      temp = strtok (NULL, " ");
    }

    char *path = malloc( (strlen(dir) + strlen(fileName) + 1) * sizeof(char) ); // used to store the file's path

    if (path == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    strcpy(path,dir);
    sscanf(fileName,"%s\n",fileName);
    strcat(path,fileName);

    int fileFd;
    if ( (fileFd = open(path, O_RDONLY))  < 0)  { // open the corresponding file and get the file descriptor in order to read from it
      perror("worker: can't open given file");
    }


    int bytes;
    char character;
    char url[2048]; // used to save the url location
    char keepWandDot[5]; // used to save the "www."

    int flagH=0,flagT=0,flagP=0,flagDots=0,flagSlashes=0; // these flags are used to detect the "http://"
    int skipWandDot=0; // used to detect the "www."
    int parseUrl=0; // flag that used to save the url location
    int index=0; // index that used for url array

    do{
      // read one by one character from the file
      bytes=read(fileFd, &character,1);

      if( parseUrl && skipWandDot==4 ){ // check if flag for the url location parsing is ON and if already saved 4 characters
        // then begin to parse/save the url location

        if( keepWandDot[0]!='\0' && strcmp(keepWandDot,"www.")!=0){ // check if 4 characters that saved isn't equal with "www."
          // then these characters are part of the url location, so copy them at the url string (where the url location will be finally saved)
          strcpy(url,keepWandDot);
          index=skipWandDot;
          for(int i=0;i<5;i++) // re-initialize keepWandDot
            keepWandDot[i]='\0';
        }

        if(character == '/' || character == ' ' || character == '\n'){ // check if the character is equal with: '/',' ' or '\n'
          // then url location parsing ends
          url[index] = '\0';
          htUniqueInsertUrl(htUrls,url); // insert it to the hash table (or if it already exists increase the appearances by one)
          // re-initialize in order to parse/save the next url location (if exists)
          index=0;
          parseUrl=0;
          skipWandDot=0;
        }
        else{
          url[index++] = character; // save character in to buffer
        }
      }


      if(character == 'h'){ // check if character that read from file is the letter 'h'
        flagH=1;
      }
      else if ( flagH && character == 't' ){  // check if character that read from file is the letter 't'
        flagT++;
      }
      else if ( flagT == 2 && character == 'p' ){  // check if character that read from file is the letter 'p'
        flagP=1;
      }
      else if ( flagP && character == ':' ){  // check if character that read from file is the ':'
        flagDots=1;
      }
      else if ( flagDots && character == '/' ){  // check if character that read from file is the '/'
        flagSlashes++;
      }
      else{
        // current character is out of the wanted sequence "http://",
        // so set the flags in to zero again
        flagH=0;
        flagT=0;
        flagP=0;
        flagDots=0;
        flagSlashes=0;
      }

      if( flagH && flagT == 2 && flagP && flagP && flagDots && flagSlashes==2){ // check if all flags tha corresponding the detection of "http://" is ON
        // then "http://" detected, so initialize the buffers and the flags in order to begin the parse of the url location

        for(int i=0;i<2048;i++) // re-initialize url buffer
          url[i]='\0';

        for(int i=0;i<5;i++) // re-initialize keepWandDot
          keepWandDot[i]='\0';

        skipWandDot=0;
        parseUrl=1;
        index=0;

        // set the flags that corresponding the "http://" in to zero
        flagH=0;
        flagT=0;
        flagP=0;
        flagDots=0;
        flagSlashes=0;
        continue;
      }

      if(parseUrl && skipWandDot < 4 ){ // check if flag for the url location parsing is ON and if there aren't 4 characters saved yet
        keepWandDot[skipWandDot++] = character;// then save the character in to buffer
      }

    } while(bytes>0); // until whole file read

    char *outputFile=malloc( (strlen("output/") + strlen(fileName) + strlen(".out") + 1 ) * sizeof(char)); // used to save the name of the output file

    if (outputFile == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    strcpy(outputFile,"output/");
    strcat(outputFile,fileName);
    strcat(outputFile,".out");

    int fileCreateFd=open(outputFile, O_WRONLY | O_APPEND | O_CREAT, 0644); // create a new .out file in order to write in to it the url locations and their appearances number that found


    if(fileCreateFd < 0){ // error checking
      perror("worker: Could not create file");
    }

    printf("> Worker: Creates file: %s <\n",outputFile);

    htWriteUrl(htUrls,fileCreateFd); // write in to file the urls locations and their appearances number that found

    close(fileCreateFd);

    // free all allocated space
    htDeleteUrl(htUrls);
    free(htUrls);
    free(buffer);
    free(dir);
    free(fileName);
    free(path);
    free(outputFile);
  }


  return 0;

}

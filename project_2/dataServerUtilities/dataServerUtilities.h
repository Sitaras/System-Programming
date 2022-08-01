#pragma once
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include "../hashTableClients/hashTableClients.h"
#include "../filesQueue/filesQueue.h"


void openDirectoryAndPushFiles(char*,queue*,int);
void extractDirectoryContents(DIR *,char* ,queue* ,int );
void sendFileData(int ,char* ,int );
void sendFile(int ,char* ,int );
void* communicationThread(void* );

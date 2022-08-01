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
#include "../hashTableClients/hashTableClients.h"
#include "../hashTableClients/hashTableListClients/hashTableListClients.h"
#include "../filesQueue/filesQueue.h"
#include "../dataServerUtilities/dataServerUtilities.h"

typedef struct threadPool threadPool;


struct threadPool {
  pthread_t *workerThreads;
  int threadPoolSize;
  int deleteThreadPool;
};

threadPool* initThreadPool(int);
void* workerThread(void *);
void threadPoolDelete(threadPool *);

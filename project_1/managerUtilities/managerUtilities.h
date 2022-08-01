#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "../hashTableSaveWorkers/hashTableSaveWorkers.h"
#include "../workersQueue/workersQueue.h"

void forkWorkers(hashtable *,queue *, int, int *);
void popNSend(const hashtable *,queue *,char* );
void collectAvailableWorkers(queue *);

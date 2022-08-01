#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <stdint.h>
#include <pthread.h>

typedef struct listNode listNode;
typedef struct node node;

// hash table's linked list

struct node{
  int clientFd; // to save client's file descriptor
  pthread_t comThreadId; // to save the communication thread's id
  char clientName[200]; // to store the client's name
  pthread_mutex_t mtxClient; // to ensure that at the corresponding socket of client writes data only one worker thread at a time
};

struct listNode {
  node n;
  listNode* next;
};

listNode* allocateListNode(int,char *,pthread_t);
listNode* listInsert(listNode*,int,char *,pthread_t);
listNode* listSearchFd(listNode * ,int );
listNode* listDelete(listNode *);

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

typedef struct listNode listNode;
typedef struct node node;

// hash table's linked list

struct node{
  int pid;
  char writeFifo[100];
  int fifoWriteFd;
};

struct listNode {
  node n;
  listNode* next;
};

listNode* allocateListNode(int,char*,int);
listNode* listInsert(listNode*,int,char*,int);
listNode* listSearchPid(listNode * ,int );
void listPrint(listNode *);
listNode* listDelete(listNode *);

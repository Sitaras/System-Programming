#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct queueNode queueNode;
typedef struct queue queue;

// Queue to store the file paths with the corresponding file descriptors of every client.

struct queueNode {
  char* fileName;
  int clientFd;
  queueNode* next;
};

struct queue {
  queueNode *first, *last;
  int numberOfNodes; // nodes counter
  int maxNumberOfNodes; // maximum number of nodes that queue can have
};

queue* initQueue(int);
queueNode* allocateNode(int, char*);
int pushQueue(queue* ,int,char* ); // return 1 when queue is full and return 0 when a node pushed in to queue
queueNode* popQueue(queue* );
queue* deleteQueue(queue* );
int empty(const queue* );

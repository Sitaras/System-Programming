#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct queueNode queueNode;
typedef struct queue queue;


struct queueNode {
  int pid;
  queueNode* next;
};

struct queue {
  queueNode *first, *last;
  int numberOfNodes;
};

queue* initQueue();
queueNode* allocateNode(int);
void pushQueue(queue* ,int );
int popQueue(queue* );
queue* deleteQueue(queue* );
int empty(const queue* );
void printQueue(const queue* );

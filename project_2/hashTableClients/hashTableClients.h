#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "./hashTableListClients/hashTableListClients.h"

typedef struct hashtable hashtable;
typedef struct hashtableNode hashtableNode;


struct hashtableNode{
  listNode* head;
};

struct hashtable{
  hashtableNode* table;
  int buckets;
  int numberOfNodes;
};

int hashFunction(const hashtable*,int);
hashtable* htInitialize(int);
int htInsert(hashtable* ,int,char*,pthread_t);
node* htSearchFd(const hashtable*,int);
void htDelete(hashtable*);

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "./hashTableListSaveWorkers/hashTableListSaveWorkers.h"

typedef struct hashtable hashtable;
typedef struct hashtableNode hashtableNode;

// hash table's linked list

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
int htInsert(hashtable* ,int,char*,int);
node* htSearch(const hashtable*,int);
void htPrint(const hashtable*);
void htDelete(hashtable*);

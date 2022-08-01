#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "./hashTableListUrl/hashTableListUrl.h"

typedef struct hashtableUrl hashtableUrl;
typedef struct hashtableNodeUrl hashtableNodeUrl;

// hash table with separate chaining (each cell of hash table point to a linked list)

struct hashtableNodeUrl{
  listNodeUrl* head;
};

struct hashtableUrl{
  hashtableNodeUrl* table;
  int buckets;
};

int hashFunctionUrl(const hashtableUrl*,char*);
hashtableUrl* htInitializeUrl(int);
int htUniqueInsertUrl(hashtableUrl*, char*);
nodeUrl* htSearchUrl(const hashtableUrl*,char*);
void htPrintUrl(const hashtableUrl*);
void htWriteUrl(const hashtableUrl*,int );
void htDeleteUrl(hashtableUrl*);

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct nodeUrl nodeUrl;
typedef struct listNodeUrl listNodeUrl;


// hash table's linked list

struct nodeUrl{
  char url[2048];
  int appearances;
};

struct listNodeUrl {
  nodeUrl nu;
  listNodeUrl* next;
};

listNodeUrl* allocateListNodeUrl(char*);
listNodeUrl* listInsertUrl(listNodeUrl*,char*);
listNodeUrl* listSearchUrl(listNodeUrl * ,char * );
void listPrintUrl(listNodeUrl *);
void listWriteUrl(listNodeUrl *,int);
listNodeUrl* listDeleteUrl(listNodeUrl *);

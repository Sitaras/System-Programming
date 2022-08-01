#include "hashTableSaveWorkers.h"


// hash table with separate chaining (each bucket of hash table point to a linked list)

int hashFunction(const hashtable* ht,int id){
  return (id % ht->buckets);
}

hashtable* htInitialize(int buckets) {
  // allocate and initialize the hash table with the given number of buckets
  hashtable* ht=malloc(sizeof(hashtable));
  if (ht == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  ht->table=malloc(sizeof(hashtableNode)*buckets);
  if (ht->table == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  ht->buckets=buckets;
  for (int i=0;i<buckets;i++){
    ht->table[i].head=NULL;
  }
  ht->numberOfNodes=0; // nodes counter
  return ht;
}


node* htSearch(const hashtable* ht, int pid){
  // search to find the worker's pid
  int index=hashFunction(ht, pid);
  listNode* ln=listSearchPid(ht->table[index].head,pid);
  if (ln != NULL){
    // found, then return corresponding node
    return &(ln->n);
  }
  else {
    // not found
    return NULL;
  }
}


int htInsert(hashtable* ht, int pid,char* writeFifo,int fifoWriteFd){
  // insert worker's pid, file descriptor and name of fifo in to hash table
  int index=hashFunction(ht,pid);
  ht->table[index].head=listInsert(ht->table[index].head,pid,writeFifo,fifoWriteFd);
  ht->numberOfNodes++;
  return 1;
}


void htPrint(const hashtable* ht){
  printf("------------ \n");
  printf("> Hash Table - Workers:\n\n");
  for (int i=0;i<ht->buckets;i++){
    printf("\n>> Bucket %d: ",i+1);
    listPrint(ht->table[i].head);
  }
  printf("------------ \n\n");
}


void htDelete(hashtable* ht){
  // delete whole hash table
  for (int i=0;i<ht->buckets;i++){
    ht->table[i].head=listDelete(ht->table[i].head);
  }
  free(ht->table);
}

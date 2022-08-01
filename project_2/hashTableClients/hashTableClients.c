#include "hashTableClients.h"

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


node* htSearchFd(const hashtable* ht, int clientFd){
  // search to find the client's fd
  int index=hashFunction(ht, clientFd);
  listNode* ln=listSearchFd(ht->table[index].head,clientFd);
  if (ln != NULL){
    // found, then return corresponding node
    return &(ln->n);
  }
  else {
    // not found
    return NULL;
  }
}


int htInsert(hashtable* ht, int clientFd,char* clientName,pthread_t comThreadId){
  // insert client socket's fd, client's name and the communication thread's id
  int index=hashFunction(ht,clientFd);
  ht->table[index].head=listInsert(ht->table[index].head,clientFd,clientName,comThreadId);
  ht->numberOfNodes++;
  return 1;
}



void htDelete(hashtable* ht){
  // delete whole hash table
  printf("> Communication Threads Number: %d <\n",ht->numberOfNodes);
  for (int i=0;i<ht->buckets;i++){
    ht->table[i].head=listDelete(ht->table[i].head);
  }
  free(ht->table);
}

#include "hashTableUrl.h"


// hash table with separate chaining (each bucket of hash table point to a linked list)

int hashFunctionUrl(const hashtableUrl* ht,char* url){
  // djb2
  unsigned long hash = 5381;
  int c;
  while ((c = *url++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return (hash % ht->buckets);
}


hashtableUrl* htInitializeUrl(int buckets) {
  // allocate and initialize the hash table with the given number of buckets
  hashtableUrl* ht=malloc(sizeof(hashtableUrl));
  if (ht == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  ht->table=malloc(sizeof(hashtableNodeUrl)*buckets);
  if (ht->table == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  ht->buckets=buckets;
  for (int i=0;i<buckets;i++){
    ht->table[i].head=NULL;
  }
  return ht;
}


nodeUrl* htSearchUrl(const hashtableUrl* ht, char *url){
  // search to find the url
  int index=hashFunctionUrl(ht, url);
  listNodeUrl* ln=listSearchUrl(ht->table[index].head,url);
  if (ln != NULL){
    // found, then return corresponding node
    return &(ln->nu);
  }
  else {
    // not found
    return NULL;
  }
}


int htUniqueInsertUrl(hashtableUrl* ht,char* url){
  // if a new url location given, then insert it to the hash table.
  // Otherwise, if the given url location has already been inserted at the hash table
  // then just increase the number of his appearances.
  nodeUrl *nUrl = htSearchUrl(ht,url);
  if( nUrl == NULL ){
    // not exists
    int index=hashFunctionUrl(ht,url);
    ht->table[index].head=listInsertUrl(ht->table[index].head,url);
    return 1;
  }
  else{
    // already exists
    nUrl->appearances++;
    return 2;
  }
  return 0;
}


void htPrintUrl(const hashtableUrl* ht){
  printf("------------ \n");
  printf("> Hash Table - Urls:\n\n");
  for (int i=0;i<ht->buckets;i++){
    printf(">> Bucket %d: ",i+1);
    listPrintUrl(ht->table[i].head);
  }
  printf("------------ \n\n");
}

void htWriteUrl(const hashtableUrl* ht,int fileFd){
  // write all the urls locations (that contained in to the hash table )
  // and their appearances to the given file (file descriptor of the file given)
  for (int i=0;i<ht->buckets;i++){
    listWriteUrl(ht->table[i].head,fileFd);
  }
}


void htDeleteUrl(hashtableUrl* ht){
  // delete whole hash table
  for (int i=0;i<ht->buckets;i++){
    ht->table[i].head=listDeleteUrl(ht->table[i].head);
  }
  free(ht->table);
}

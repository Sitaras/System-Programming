#include "hashTableListClients.h"
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

// hash table's linked list


listNode* allocateListNode(int clientFd,char* clientName,pthread_t comThreadId){
  // allocate and initialize the list node
  listNode* node=malloc(sizeof(listNode));
  if (node == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  node->n.clientFd=clientFd;
  node->n.comThreadId=comThreadId;
  strcpy(node->n.clientName,clientName);
  node->next=NULL;
  pthread_mutex_init(&(node->n.mtxClient), NULL);
  return node;
}


listNode* listInsert(listNode *list,int clientFd,char* clientName,pthread_t comThreadId){
  if (list==NULL){
    // list is empty
    listNode* node=allocateListNode(clientFd,clientName,comThreadId);
    list=node;
    return list;
  }
  // insert a new node at the beginning of the list
  listNode* node=allocateListNode(clientFd,clientName,comThreadId);
  node->next=list;
  list=node;

  return list;
}


listNode* listSearchFd(listNode *list, int clientFd){
  // search to find the client's fd
  listNode* current=list;
  while (current!=NULL){
    if (current->n.clientFd==clientFd)
      return current; // found, return the list node
    current=current->next;
  }
  return NULL; // not found
}


listNode* listDelete(listNode *list){
    // delete whole list
    // close the connections with all clients and destroy the correspondings mutexes
    if(list==NULL) return NULL;

    listNode *current = list;
    listNode *next;
    while(current!=NULL){

      int endOfCom=1; // set the value to 1 (ON)
      int err;
      if (write(current->n.clientFd, &endOfCom, sizeof(int)) < 0) { // send it to the corresponding client in order to update/inform him that his communication with the server just over
        perror("write");
        exit(1);
      }

      close(current->n.clientFd); // close connection

      if ((err = pthread_join(current->n.comThreadId, NULL))) { // wait communication thread to end
        perror2("pthread_join", err);
        exit(1);
      }

      if ((err = pthread_mutex_destroy(&(current->n.mtxClient)))) { // destroy the corresponding mutex
        perror2("pthread_mutex_destroy", err);
        exit(1);
      }
      printf("Connection with client '%s' just closed.\n",current->n.clientName );
      next=current->next;
      free(current);
      current=next;
    }
    list=NULL;
    return list;
}

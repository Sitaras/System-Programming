#include "hashTableListSaveWorkers.h"

// hash table's linked list


listNode* allocateListNode(int pid, char *writeFifo, int fifoWriteFd){
  // allocate and initialize the list node
  listNode* node=malloc(sizeof(listNode));
  if (node == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  node->n.pid=pid;
  strcpy(node->n.writeFifo,writeFifo);
  node->n.fifoWriteFd=fifoWriteFd;
  node->next=NULL;
  return node;
}


listNode* listInsert(listNode *list,int pid, char *writeFifo, int fifoWriteFd){
  if (list==NULL){
    // list is empty
    listNode* node=allocateListNode(pid,writeFifo,fifoWriteFd);
    list=node;
    return list;
  }
  // insert a new node at the beginning of the list
  listNode* node=allocateListNode(pid,writeFifo,fifoWriteFd);
  node->next=list;
  list=node;

  return list;
}


listNode* listSearchPid(listNode *list, int pid){
  // search to find the worker's pid
  listNode* current=list;
  while (current!=NULL){
    if (current->n.pid==pid)
      return current; // found, return the list node
    current=current->next;
  }
  return NULL; // not found
}


void listPrint(listNode *list){
    if(list==NULL) return;
    listNode *temp=list;
    while(temp!=NULL){
      printf(" | Worker with pid: %d and fifo name: %s | ",temp->n.pid,temp->n.writeFifo);
      temp=temp->next;
    }
}


listNode* listDelete(listNode *list){
    // delete whole list
    if(list==NULL) return NULL;

    listNode *current = list;
    listNode *next;

    while(current!=NULL){
      close(current->n.fifoWriteFd); // close fd
      printf("Close fifo: %s\n",current->n.writeFifo);
      if ( unlink(current->n.writeFifo) < 0) { // close fifo
         perror("client: can't unlink");
      }
      kill(current->n.pid, SIGKILL); // kill the corresponding worker
      printf("Worker with pid: %d killed\n",current->n.pid);
      next=current->next;
      free(current);
      current=next;
    }
    list=NULL;
    return list;
}

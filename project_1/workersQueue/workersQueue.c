#include "workersQueue.h"

queue* initQueue(){
  // allocate and initialize the queue
  queue* q = malloc(sizeof(queue));
  if (q == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  q->first=NULL;
  q->last=NULL;
  q->numberOfNodes=0; // nodes counter
  return q;
}

queueNode* allocateNode(int pid){
  // allocate and initialize a queue node
  queueNode *node = malloc(sizeof(queueNode));
  if (node == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  node->pid=pid;
  node->next=NULL;
  return node;
}

void pushQueue(queue* queue,int pid){
  // push node in to queue with the given worker's pid
  queueNode *node = allocateNode(pid);
  queue->numberOfNodes++;
  if(queue->first==NULL){
    queue->first=node;
    queue->last=node;
  }
  else{
    queue->last->next=node;
    queue->last=node;
  }
}

int popQueue(queue* queue){
  // pop node from the queue
  if(queue == NULL || queue->first == NULL )
    return -1;

  queue->numberOfNodes--;
  int pid = queue->first->pid;
  queueNode *toDelete=queue->first;
  queue->first=queue->first->next;
  free(toDelete);
  return pid;
}


queue* deleteQueue(queue* queue){
  // delete whole queue
  printf("\n");
  while(queue->first!=NULL){
    queueNode *temp=queue->first->next;
    printf("> Queque: Delete worker with pid: %d\n",queue->first->pid);
    free(queue->first);
    queue->first=temp;
  }
  free(queue);
  return NULL;
}


int empty(const queue* queue){
  if(queue->first == NULL){
    return 1;
  }
  return 0;
}


void printQueue(const queue* queue){
  if(queue == NULL || queue->first == NULL ){
    printf("> Queque is empty (nothing to print)<\n");
    return ;
  }
  printf("------------ \n");
  printf("> Queque:\n");
  queueNode *temp=queue->first;
  while(temp!=NULL){
    printf("  worker with pid: %d\n",temp->pid);
    temp=temp->next;
  }
  printf("------------ \n");
}

#include "filesQueue.h"

queue* initQueue(int maxNumberOfNodes){
  // allocate and initialize the queue
  queue* q = malloc(sizeof(queue));
  if (q == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  q->first=NULL;
  q->last=NULL;
  q->numberOfNodes=0; // nodes counter
  q->maxNumberOfNodes=maxNumberOfNodes; // init the maximum number of nodes that queue can have
  return q;
}

queueNode* allocateNode(int clientFd, char *fileName){
  // allocate and initialize a queue node
  queueNode *node = malloc(sizeof(queueNode));
  if (node == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  node->clientFd=clientFd;
  node->fileName=malloc((strlen(fileName)+1)*sizeof(char));
  strcpy(node->fileName,fileName);
  node->next=NULL;
  return node;
}

int pushQueue(queue* queue, int clientFd, char *fileName){
  if(queue->numberOfNodes == queue->maxNumberOfNodes){ // check if queue is full
    // then maximum number of nodes reached, don't push a new node
    return 1;
  }

  // push node in to queue with the given client's Fd and the corresponding file name
  queueNode *node = allocateNode(clientFd,fileName);
  if(queue->first==NULL){
    queue->first=node;
    queue->last=node;
  }
  else{
    queue->last->next=node;
    queue->last=node;
  }
  queue->numberOfNodes++;
  return 0;
}

queueNode* popQueue(queue* queue){
  // pop node from the queue
  if(queue == NULL || queue->first == NULL )
    return NULL;

  queue->numberOfNodes--;
  queueNode *toDelete=queue->first;
  queue->first=queue->first->next;
  return toDelete;
}


queue* deleteQueue(queue* queue){
  // delete whole queue
  printf("\n");
  while(queue->first!=NULL){
    queueNode *temp=queue->first->next;
    free(queue->first->fileName);
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

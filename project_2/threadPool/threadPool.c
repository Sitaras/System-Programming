#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

#include "threadPool.h"

extern pthread_mutex_t queueMtx;
extern pthread_cond_t nonEmpty;
extern pthread_cond_t nonFull;

extern pthread_mutex_t htMtx;
extern pthread_cond_t nonClient;

extern queue *q;
extern hashtable *htClients;

extern int blockSize;


threadPool* initThreadPool(int threadPoolSize){
  // allocate and initialize the thread Pool
  threadPool* tp = malloc(sizeof(threadPool));
  if (tp == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  tp->deleteThreadPool=0;
  tp->threadPoolSize=threadPoolSize;

  tp->workerThreads = malloc(threadPoolSize * sizeof(pthread_t));
  if (tp->workerThreads == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }

  int err;
  for(int i = 0; i < tp->threadPoolSize; i++) { // create the worker threads
    if ((err = pthread_create(&(tp->workerThreads[i]), NULL, workerThread, (void*)tp))) {
      perror2("pthread_create", err);
      exit(1);
    }
  }

  return tp;
}



void* workerThread(void *pool){
    threadPool *tp = (threadPool *)pool;

    int err;
    while(1) {

      if ((err = pthread_mutex_lock(&queueMtx))) { // lock queue's mutex
        perror2("pthread_mutex_lock", err);
        exit(1);
      }

      while((empty(q)) && (!tp->deleteThreadPool)) { // check if queue is empty and flag for the delete of Thread Pool is false
        printf("[Worker Thread: %ld]: suspended, Queue is empty\n",pthread_self());
        pthread_cond_wait(&nonEmpty, &queueMtx); // then suspend the worker thread
      }

      if(tp->deleteThreadPool) { // check if delete flag is on
        break; // then exit from the loop
      }

      queueNode *qNode = popQueue(q); // pop a file path from the queue and "assign it" to the current worker thread
      printf("[Worker Thread: %ld]: Received task: <%s, %d>.\n",pthread_self(),qNode->fileName,qNode->clientFd);

      pthread_cond_signal(&nonFull); // awake a communication thread (if it was suspended because queue was full)

      if ((err = pthread_mutex_unlock(&queueMtx))) { // unlock queue's mutex
        perror2("pthread_mutex_unlock", err);
        exit(1);
      }

      int endOfCom=0; // used to update/inform the client that his communication with the server isn't over yet,
                      // more specifically informs the client that will receive a file from the server

      if ((err = pthread_mutex_lock(&htMtx))) { // lock hash table's mutex
        perror2("pthread_mutex_lock", err);
        exit(1);
      }

      struct node *tempPtr;

      while((tempPtr=htSearchFd(htClients,qNode->clientFd))==NULL) // check if the wanted client's qNode not already inserted from the main thread in to hash table
        pthread_cond_wait(&nonClient, &htMtx); // then suspend the worker thread

      if ((err = pthread_mutex_unlock(&htMtx))) { // unlock hash table's mutex
        perror2("pthread_mutex_unlock", err);
        exit(1);
      }

      // at the socket of each client writes data only one worker thread at a time, so find the corresponding mutex for this client and lock it
      if ((err = pthread_mutex_lock(&(tempPtr->mtxClient)))) { // lock client's mutex
        perror2("pthread_mutex_lock", err);
        exit(1);
      }

      if (write(qNode->clientFd, &endOfCom, sizeof(int)) < 0) { // send the integer variable to inform the client that will receive a file from the server
        perror("write");
        exit(1);
      }

      printf("[Worker Thread: %ld]: About to read and send file %s.\n",pthread_self(),qNode->fileName);
      sendFile(qNode->clientFd,qNode->fileName,blockSize); // send the requested file to the client (which is assigned to current thread)


      if ((err = pthread_mutex_unlock(&(tempPtr->mtxClient)))) { // unlock client's mutex
        perror2("pthread_mutex_unlock", err);
        exit(1);
      }

      free(qNode->fileName);
      free(qNode);
    }

    if ((err = pthread_mutex_unlock(&queueMtx))) { // at this case the flag for the delete of Thread Pool is true, so unlock the mutex before the termination of the worker thread
      perror2("pthread_mutex_unlock", err);
      exit(1);
    }

    pthread_exit(NULL);
}




void threadPoolDelete(threadPool *tp){
    int err;

    if ((err = pthread_mutex_lock(&queueMtx))) { // Lock mutex
      perror2("pthread_mutex_lock", err);
      exit(1);
    }

    tp->deleteThreadPool = 1; // set delete flag to ON

    pthread_cond_broadcast(&nonEmpty); // awake all worker threads in order to exit/terminate

    if ((err = pthread_mutex_unlock(&queueMtx))) { // Unlock mutex
      perror2("pthread_mutex_unlock", err);
      exit(1);
    }

    for(int i = 0; i < tp->threadPoolSize; i++){ // wait all worker threads to end
      if ((err = pthread_join(tp->workerThreads[i], NULL))) {
        perror2("pthread_join", err);
        exit(1);
      }
    }

    printf("> Worker Threads Number: %d <\n",tp->threadPoolSize);
    free(tp->workerThreads);
}

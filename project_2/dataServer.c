#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

#include "dataServerUtilities/dataServerUtilities.h"
#include "threadPool/threadPool.h"

static int flagInt; // used in signal handler for SIGINT

static void catchInt(int sigint){
  // signal handler for SIGINT
  flagInt=1;
}

pthread_mutex_t queueMtx; // mutex for synchronizing the communication threads and the worker threads when they "access" the queue (push/pop)
pthread_cond_t nonEmpty; // condition variable, used at queue, determines that the queue is empty
pthread_cond_t nonFull; // condition variable, used at queue, determines that the queue is full

pthread_mutex_t htMtx; // mutex for synchronizing the main thread and the worker threads when they "access" the hash table (insert/search)
pthread_cond_t nonClient; // condition variable, used at hash table, determines that the node about the wanted client do not exist in hash table


queue *q; // to store the file paths (with the file descriptors of the corresponding clients).
          // It is a shared resource from which the communication threads push file paths and the worker threads pop the file paths.
hashtable *htClients; // hash table, to store, for each client of the server:
                      // his file descriptor, his name, the corresponding communication Thread's id
                      // and init a mutex to ensure that at the corresponding socket of client writes data only one worker thread at a time.

int blockSize;

int main ( int argc, char *argv[] ){

  // initialize mutexes and the condition variables
  pthread_mutex_init(&queueMtx, NULL);
  pthread_cond_init(&nonEmpty, NULL);
  pthread_cond_init(&nonFull, NULL);

  pthread_mutex_init(&htMtx, NULL);
  pthread_cond_init(&nonClient, NULL);


  // initialize the global variable
  flagInt=0;

  // setup the SIGINT handler, in order to terminate the clients and after the server
  struct sigaction intHandler = {0};
  intHandler.sa_handler = catchInt;
  sigaction(SIGINT,&intHandler,NULL);

  int port;
  int threadPoolSize;
  int queueSize;

  if(argc < 9 || argc >= 10 ){
    fprintf(stderr, "Usage: %s -p <port> -s <thread_pool_size> -q <queue_size> -b <block_size>\n",argv[0]);
    exit(1);
  }

  printf("> Server's pid: %d <\n",getpid());

  printf("Server's parameters are:\n");
  for(int i = 1 ; i < argc ; i++){
    if(strcmp(argv[i],"-p")==0 && (argc > i+1)){
      port=atoi(argv[i+1]);
      printf("port: %d\n", port);
    }
    else if(strcmp(argv[i],"-s")==0 && (argc > i+1)){
      threadPoolSize=atoi(argv[i+1]);
      printf("thread_pool_size: %d\n", threadPoolSize);
    }
    else if(strcmp(argv[i],"-q")==0 && (argc > i+1)){
      queueSize=atoi(argv[i+1]);
      printf("queue_size : %d\n", queueSize);
    }
    else if(strcmp(argv[i],"-b")==0 && (argc > i+1)){
      blockSize=atoi(argv[i+1]);
      printf("Block_size: %d\n", blockSize);
    }
  }



  int sock;
  struct sockaddr_in server, client;
  socklen_t clientlen;
  struct sockaddr *serverptr=(struct sockaddr *)&server;
  struct sockaddr *clientptr=(struct sockaddr *)&client;
  struct hostent *rem;

  // initialize the hash table, the queue and the thread pool
  htClients=htInitialize(15);
  q = initQueue(queueSize);
  threadPool *tp = initThreadPool(threadPoolSize);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Create TCP socket
    perror("socket");

  server.sin_family = AF_INET; // Internet domain
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port); // The given port

  if (bind(sock, serverptr, sizeof(server)) < 0) // Bind socket to address
    perror("bind");

  if (listen(sock, 50) < 0) // Listen for connections
    perror("listen");

  printf("Server was successfully initialized...\n");
  printf("Listening for connections to port %d\n", port);
  printf("Waiting for connections...\n");

  pthread_t comThreadId;
  int err;
  int *newsock=NULL;
  while(1) {

    clientlen = sizeof(client);
    newsock=malloc(sizeof(int));

    if (newsock == NULL) {
      perror("malloc : memory not available");
      exit(2);
    }

    if ((*newsock = accept(sock, clientptr, &clientlen))< 0) // accept connection
      perror("accept");

    if (flagInt){ // check if SIGINT has been received
      // then ...
      threadPoolDelete(tp); // wait for all worker threads to end (join) and delete the thread pool
      free(tp);
      htDelete(htClients); // terminate the clients, after close the connections with all clients and destroy the correspondings mutexes
      free(htClients);
      deleteQueue(q);
      break;
    }

    if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr,sizeof(client.sin_addr.s_addr), client.sin_family))== NULL) { // find client's name
      herror("gethostbyaddr");
      exit(1);
    }

    printf("Accepted connection from %s\n", rem->h_name);

    int clientFd=*newsock;
    if ((err = pthread_create(&comThreadId, NULL, communicationThread, (void *) newsock))) { // create the communication thread
      perror2("pthread_create", err);
      exit(1);
    }

    if ((err = pthread_mutex_lock(&htMtx))) { // lock hash table's mutex
      perror2("pthread_mutex_lock", err);
      exit(1);
    }


    htInsert(htClients,clientFd,rem->h_name,comThreadId); // store the details for the current client that was accepted from the server.
                                                          // So store the corresponding file descriptor, his name, the communication Thread's id and init a mutex for the communication between server and him
                                                          // to ensure that at the corresponding socket of client writes data only one worker thread at a time.

    pthread_cond_broadcast(&nonClient);

    if ((err = pthread_mutex_unlock(&htMtx))) { // unlock hash table's mutex
      perror2("pthread_mutex_unlock", err);
      exit(1);
    }

  }

  if(newsock!=NULL)
    free(newsock);


  if ((err = pthread_mutex_destroy(&queueMtx))) { // destroy mutex variable
    perror2("pthread_mutex_destroy", err);
    exit(1);
  }

  if ((err = pthread_cond_destroy(&nonEmpty))) { // destroy condition variable
    perror2("pthread_cond_destroy", err);
    exit(1);
  }

  if ((err = pthread_cond_destroy(&nonFull))) { // destroy condition variable
    perror2("pthread_cond_destroy", err);
    exit(1);
  }

  if ((err = pthread_mutex_destroy(&htMtx))) { // destroy mutex variable
    perror2("pthread_mutex_destroy", err);
    exit(1);
  }

  if ((err = pthread_cond_destroy(&nonClient))) { // destroy condition variable
    perror2("pthread_cond_destroy", err);
    exit(1);
  }

  return 0;
}

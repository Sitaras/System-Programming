FILESQUEUE=./filesQueue
HASHTABLECLIENTS=./hashTableClients
HASHTABLELIST=./hashTableClients/hashTableListClients
DATASERVERUTILITIES=./dataServerUtilities
CLIENTUTILITIES=./client/clientUtilities
THREADPOOL=./threadPool


CC=gcc -lpthread -pthread
CFLAGS=-g -Wall -I$(THREADPOOL) -I$(CLIENTUTILITIES) -I$(HASHTABLELIST) -I$(HASHTABLECLIENTS) -I$(FILESQUEUE) -I$(DATASERVERUTILITIES)

OBJ1=dataServer.o $(THREADPOOL)/threadPool.o $(DATASERVERUTILITIES)/dataServerUtilities.o $(HASHTABLECLIENTS)/hashTableClients.o $(HASHTABLELIST)/hashTableListClients.o $(FILESQUEUE)/filesQueue.o
OBJ2=client/remoteClient.o $(CLIENTUTILITIES)/clientUtilities.o

EXEC1=dataServer
EXEC2=client/remoteClient

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(EXEC1)

$(EXEC2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $(EXEC2)


.PHONY: clean

clean:
	rm -f $(OBJ1) $(EXEC1) $(OBJ2) $(EXEC2)

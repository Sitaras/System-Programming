WORKERSQUEUE=./workersQueue
HASHTABLESAVEWORKERS=./hashTableSaveWorkers
HASHTABLELIST=./hashTableSaveWorkers/hashTableListSaveWorkers
HASHTABLEURL=./hashTableUrl
HASHTABLELISTURL=./hashTableUrl/hashTableListUrl
MANAGERUTILITIES=./managerUtilities


CC=gcc
CFLAGS=-g -Wall -I$(HASHTABLELIST) -I$(HASHTABLESAVEWORKERS) -I$(HASHTABLELISTURL) -I$(HASHTABLEURL) -I$(WORKERSQUEUE) -I$(MANAGERUTILITIES)

OBJ1=managerNlistener.o $(MANAGERUTILITIES)/managerUtilities.o $(HASHTABLESAVEWORKERS)/hashTableSaveWorkers.o $(HASHTABLELIST)/hashTableListSaveWorkers.o $(WORKERSQUEUE)/workersQueue.o
OBJ2=worker.o $(HASHTABLEURL)/hashTableUrl.o $(HASHTABLELISTURL)/hashTableListUrl.o

EXEC1=sniffer
EXEC2=worker

all: $(EXEC1) $(EXEC2)

$(EXEC1): $(OBJ1)
	$(CC) $(CFLAGS) $(OBJ1) -o $(EXEC1)

$(EXEC2): $(OBJ2)
	$(CC) $(CFLAGS) $(OBJ2) -o $(EXEC2)


.PHONY: clean

clean:
	rm -f $(OBJ1) $(EXEC1) $(OBJ2) $(EXEC2)
	rm -f ./output/*.out

clean-output:
	rm -f ./output/*.out

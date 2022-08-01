#include "hashTableListUrl.h"

// hash table's linked list


listNodeUrl* allocateListNodeUrl(char *url){
  // allocate and initialize the node
  listNodeUrl* node=malloc(sizeof(listNodeUrl));
  if (node == NULL) {
    perror("malloc : memory not available");
    exit(2);
  }
  strcpy(node->nu.url,url);
  node->nu.appearances=1;
  node->next=NULL;
  return node;
}


listNodeUrl* listInsertUrl(listNodeUrl *list, char *url){
  if (list==NULL){
    // list is empty
    listNodeUrl* node=allocateListNodeUrl(url);
    list=node;
    return list;
  }
  // insert a new node at the beginning of the list
  listNodeUrl* node=allocateListNodeUrl(url);
  node->next=list;
  list=node;
  return list;
}


listNodeUrl* listSearchUrl(listNodeUrl *list, char *url){
  // search to find the url
  listNodeUrl* current=list;
  while (current!=NULL){
      if (strcmp(current->nu.url,url)==0)
          return current; // found, return the list node
      current=current->next;
  }
  return NULL; // not found
}


void listPrintUrl(listNodeUrl *list){
    if(list==NULL) return;
    listNodeUrl *temp=list;
    while(temp!=NULL){
        printf(" | URL: %s - Appearances: %d | ",temp->nu.url,temp->nu.appearances);
        temp=temp->next;
    }
}

void listWriteUrl(listNodeUrl *list,int fileFd){
    // write all the urls locations
    // and their appearances to the given file (file descriptor of the file has been given)
    if(list==NULL) return;
    listNodeUrl *temp=list;
    while(temp!=NULL){
        char tempLine[2048];
        for(int i=0; i<2048; i++){
          tempLine[i]='\0';
        }
        strcpy(tempLine,temp->nu.url);
        sprintf(tempLine, "%s %d\n",tempLine,temp->nu.appearances);
        if (write(fileFd, tempLine, sizeof(tempLine)) != sizeof(tempLine)) { // write the url location and the corresponding appearances number to the .out file 
          perror("Couldn't write in to the file");
        }
        temp=temp->next;
    }
}


listNodeUrl* listDeleteUrl(listNodeUrl *list){
    // delete whole list
    if(list==NULL) return NULL;

    listNodeUrl *current = list;
    listNodeUrl *next;

    while(current!=NULL){
        next=current->next;
        free(current);
        current=next;
    }
    list=NULL;
    return list;
}

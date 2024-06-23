#include "array.h"
#include "util.h"
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H
#define MAX_REQUESTER_THREADS 10
#define MAX_RESOLVER_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#define MAX_INPUT_FILES 100

typedef struct {
    sem_t resOut;
    sem_t reqLog;
    sem_t countSem;
    array* arr;
    int counter;
    int fdRqLog;
    int fdRsLog;
    char** filenames;
    int doneWriting;
} necVar;

void* Readparent(void* arg);
int Writeparent(char* arg1, void* arg);
void* writeParentParent(void* arg);

#endif
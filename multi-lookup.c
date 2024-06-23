#include "array.h"
#include "util.h"
#include "multi-lookup.h"
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


void* Readparent(void* arg){
    necVar* state = (necVar*) arg;
    char final[(MAX_NAME_LENGTH+MAX_IP_LENGTH) + 3] = "";
    char result[(MAX_NAME_LENGTH)];
    char Ip[(MAX_IP_LENGTH)];
    char *hostname = result;
    int resolvedHosts = 0, dnsRes = 0;
    struct timeval start,end;
    gettimeofday(&start, NULL);
    while (1){
        if (state->doneWriting && state->arr->index == 0){
            break;
        } 
        else if (array_get(state->arr,&hostname) != -1){
            dnsRes = dnslookup(result,Ip,MAX_IP_LENGTH);
            if (dnsRes){
                strcpy(Ip,"NOT_RESOLVED");
                resolvedHosts-=1;
            }
            snprintf(final, sizeof(final), "%s,%s\n",result, Ip);
            sem_wait(&state->resOut);
            write(state->fdRsLog, final, strlen(final));
            sem_post(&state->resOut); 
            resolvedHosts+=1;
        }       

    }
    gettimeofday(&end, NULL);
    printf("Thread %ld resolved %d hostnames in %f seconds\n",pthread_self(), resolvedHosts, ((end.tv_sec - start.tv_sec)*1000000.00 + (end.tv_usec - start.tv_usec))/1000000.00);
    return NULL;
}
int Writeparent(char* arg1, void* arg){
    necVar* state = (necVar*) arg;
    FILE *fptr = fopen(arg1, "r");
    char buffer[MAX_NAME_LENGTH];
    int store = 0;
    if (fptr){
        while (fgets(buffer, MAX_NAME_LENGTH, fptr)){
            store = strlen(buffer);
            if (store > MAX_NAME_LENGTH){
                continue;
            }
            buffer[store-1] = '\0';
            array_put(state->arr, buffer);
            sem_wait(&state->reqLog);
            write(state->fdRqLog, buffer, store);
            sem_post(&state->reqLog);
        }
        fclose(fptr);
    }
    else {
        fprintf(stderr, "Invalid filename %s\n", arg1);
        return 0;
    }
    return 1;
}
void* writeParentParent(void* arg){
    necVar* state = (necVar*) arg;
    char** filenames = state->filenames;
    int fileReadCount = 0;
    int store = state->counter;
    struct timeval start,end;
    gettimeofday(&start, NULL);
    while (store > -1){
        sem_wait(&state->countSem);
        state->counter = state->counter-1;
        store = state->counter;
        sem_post(&state->countSem);
        if (store >= 0){
            fileReadCount+=Writeparent(filenames[store],state);
        }
    }
    gettimeofday(&end, NULL);
    printf("Thread %ld serviced %d files in %f seconds\n",pthread_self(), fileReadCount, ((end.tv_sec - start.tv_sec)*1000000.00 + (end.tv_usec - start.tv_usec))/1000000.00);
    return NULL;
}

int main(int argc, char *argv[]){    
    if (argc > 105 || argc < 6){
        fprintf(stderr, "Invalid Number of Arguments\n");
        fprintf(stderr, "Input Structure: ./___  #RequesterThreads(int) #ResolverThreads(int) RequesterLogFile(String) ResolverLogFile(String) InputFilenames[](string)\n");
        return -1;
    }
    //init bounded buffer
    struct timeval start,end;
    necVar* ptr = malloc(sizeof(necVar));
    int numReqs = atoi(argv[1]);
    int numRes = atoi(argv[2]);
    if (numReqs > MAX_REQUESTER_THREADS || numReqs > MAX_REQUESTER_THREADS) {
        fprintf(stderr, "Too many threads: limit 10 requester 10 resolver\n");
        return -1;
    }
    FILE* f = fopen(argv[3],"wb+");
    ptr->fdRqLog = fileno(f);
    FILE* g = fopen(argv[4],"wb+");
    ptr->fdRsLog = fileno(g);
    ptr->counter = argc - 5;
    if (argc-5 > MAX_INPUT_FILES) {
        fprintf(stderr, "Too many input files\n");
        return -1;
    }
    ptr->filenames = malloc(MAX_NAME_LENGTH * (ptr->counter));
    for (int i = 0; i < ptr->counter; i++){
        ptr->filenames[i] = argv[5+i];
    }
    ptr->arr = malloc(sizeof(array));
    ptr->arr->index=0;
    array_init(ptr->arr);
    //init synchronization semaphores for main
    sem_init(&ptr->resOut,0,1);
    sem_init(&ptr->reqLog,0,1);
    sem_init(&ptr->countSem,0,1);
    //test filesnames
    //filenames = malloc(4*sizeof(char*));
    pthread_t reader[numRes];
    pthread_t writer[numReqs];
    ptr->doneWriting = 0;
    gettimeofday(&start, NULL);
    for (int i = 0; i < numReqs; i++){
        pthread_create(&writer[i],NULL,writeParentParent,ptr);
    }
    for (int i = 0; i < numRes; i++){
        pthread_create(&reader[i],NULL,Readparent,ptr);        
    }    
    for (int i = 0; i < numReqs; i++){
        pthread_join(writer[i],NULL);
    }
    ptr->doneWriting = 1;
    if (ptr->arr->index == 0){
        for (int i = 0; i < numRes; i++){
            pthread_cancel(reader[i]);
        }
    }

    for (int i = 0; i < numRes; i++){
        pthread_join(reader[i],NULL);
    }
    gettimeofday(&end, NULL);
    printf("./multi-lookup: total time is %f seconds\n",((end.tv_sec - start.tv_sec)*1000000.00 + (end.tv_usec - start.tv_usec))/1000000.00);
    array_free(ptr->arr);
    fclose(f);
    fclose(g);
    free(ptr->filenames);
    free(ptr);
    return 0;
}
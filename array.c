#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "array.h"
#include <pthread.h>
#include <semaphore.h>

sem_t writer;
sem_t thingsToRead;
sem_t spaceToWrite;
int array_init(array *s){
    s->index = 0;
    if (sem_init(&writer, 0,1) != 0) {
        printf("ERROR");
        return -1;
        // exit(EXIT_FAILURE);
    }
    if (sem_init(&thingsToRead, 0,0) != 0) {
        printf("ERROR");
        return -1;
        // exit(EXIT_FAILURE);
    }
    if (sem_init(&spaceToWrite, 0,ARRAY_SIZE) != 0) {
        printf("ERROR");
        return -1;
        // exit(EXIT_FAILURE);
    }
    return 0;
}
int  array_put(array *s, char *hostname){
    // int store = strlen(hostname);
    // if (store > MAX_NAME_LENGTH){
    //     return -1;
    // }
    sem_wait(&spaceToWrite);
    sem_wait(&writer);
    // hostname[store-1] = '\0';
    strcpy(s->array[s->index],hostname);
    s->index = s->index+1;
    sem_post(&writer);
    sem_post(&thingsToRead);
    return 0;
}
int  array_get (array *s, char **hostname){
    
    sem_wait(&thingsToRead);
    sem_wait(&writer);
    s->index = s->index-1;
    strcpy(*hostname,s->array[s->index]);
    sem_post(&writer);
    sem_post(&spaceToWrite);
    return 0;
}
void array_free(array *s){
    free(s);
    sem_destroy(&thingsToRead);
    sem_destroy(&writer);
    sem_destroy(&spaceToWrite);
}
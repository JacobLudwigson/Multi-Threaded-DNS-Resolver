#ifndef ARRAY_H
#define ARRAY_H

#define ARRAY_SIZE 8                       // max elements in array
#define MAX_NAME_LENGTH 50
//circular queue
typedef struct {
    char array[ARRAY_SIZE][MAX_NAME_LENGTH];                  // storage array for integers
    int index;                                // array index indicating where the top is
} array;

int  array_init(array *s);                  // init the array
int  array_put(array *s, char *hostname);     // place element on the top of the array
int  array_get (array *s, char **hostname);    // remove element from the top of the array
void array_free(array *s);                  // free the array's resources

#endif
#ifndef QUEUE_H

// implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define STATUS_SUCCESS 0 
#define STATUS_FAIL 1 

enum overload_handling { 
    BLOCK=0,
    DT,
    DH
};

// Queue Node
struct Qnode {
    int data;
    struct timeval arrival;
};

// The Queue implement as array with consumer producer - User must use the lock.
struct Queue {
    struct Qnode** queue; // All request waiting for threads to handle requests.
    pthread_mutex_t lock;
    pthread_cond_t wait_data; // wait condition when queue is empty : consumer waits for producer to add data.
    pthread_cond_t wait_room; // wait condition when queue is full : producer waits consumer to remove data.
    int max_threads; // maximum threads that can handle requests. 
    int max_requests; // max requests that can be handle.
    int active_threads; // number of thread that currently handle requests.
    int producer;
    int consumer;
    enum overload_handling schedalg;
};

// A utility function to create a new linked list node.
struct Qnode* newNode(int data, struct timeval arrival);

// A utility function to create an empty queue
struct Queue* createQueue(int max_threads, int max_requests, enum overload_handling schedalg);

// Move request to wait room.
int enQueue(struct Queue* q, int *data, struct timeval arrival);

// Take request from wait room and make thread to handle it. 
struct Qnode* deQueueAndHandle(struct Queue* q);

// Done handle request 
void DoneHandle(struct Queue* q);

int OverloadHandle(struct Queue* q, int *data);

#define QUEUE_H

#endif //QUEUE_H

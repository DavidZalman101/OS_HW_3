#ifndef QUEUE_H

// implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// Queue Node
struct Qnode {
    int data;
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
};

// A utility function to create a new linked list node.
struct Qnode* newNode(int data);

// A utility function to create an empty queue
struct Queue* createQueue(int max_threads, int max_requests);

// Move request to wait room.
void enQueue(struct Queue* q, int data);

// Take request from wait room and make thread to handle it. 
struct Qnode* deQueueAndHandle(struct Queue* q);

// Done handle request 
void DoneHandle(struct Queue* q);

#define QUEUE_H

#endif //QUEUE_H

#ifndef QUEUE_H

/// implementation of queue
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// global locks for using in Queues
pthread_mutex_t lock;
pthread_cond_t wait_data; // wait condition when queue is empty : consumer waits for producer to add data
pthread_cond_t wait_room; // wait condition when queue is full : producer waits consumer to remove data

// Queue Node
struct Qnode {
    int data;
};

// The Queue implement as array with consumer producer - User must use the lock.
struct Queue {
    struct Qnode** queue;
    int max_size;
    int producer;
    int consumer;
};

struct Queues {
    struct Queue *wait_queue;
    struct Queue *worker_queue;;
}

// A utility function to create a new linked list node.
struct Qnode* newNode(int data);

// A utility function to create an empty queue
struct Queue* createQueue(int max_size);

// The function to add a key k to q
void enQueue(struct Queue* q, int data);

// Function to remove a key from given queue q
void deQueue(struct Queue* q);

void deQueueSpecifif(struct Queue* q, int data);

// Function to check if Queue is empty
bool QueueEmpty(struct Queue* q);

// Function to check the Queue size
int QueueSize(struct Queue* q);

// return next data of the elements be consume
int QueueNextConcumerData(struct Queue* q);

// Init the queues
struct Queues* initQueues(int wait_queue_size,int workers_queue_size);

#define QUEUE_H

#endif //QUEUE_H

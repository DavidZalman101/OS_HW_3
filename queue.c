#include "queue.h"

struct Qnode* newNode(int data)
{
    struct Qnode* temp
            = (struct Qnode*)malloc(sizeof(struct Qnode));
    temp->data = data;
    return temp;
}

struct Queue* createQueue(int max_threads, int max_requests)
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->queue = (struct Qnode**)malloc(sizeof(struct Qnode*) * (size_t)max_requests);
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->wait_room, NULL);
    pthread_cond_init(&q->wait_data, NULL);
    q->max_threads = max_threads;
    q->max_requests = max_requests;
    q->active_threads = 0;
    q->producer = 0;
    q->consumer = 0;
    return q;
}

void destroyQueue(struct Queue* q){
    for(int i = 0; i < q->max_requests; i++) { // TODO: Assure that after dequeue we refer pointer to null.
        if(q->queue[i])
            free(q->queue[i]);
    }
    free(q);
}

void enQueue(struct Queue* q, int data)
{
    pthread_mutex_lock(&q->lock);
    // Handle if there are too many requests.
    if ((q->producer - q->consumer) + q->active_threads >= q->max_requests) {
        // TODO: Part 2.
        printf("Part 2 will handle soon");
    }
    
    // if go there number of requests < max_requests
    while(q->producer - q->consumer >= q->max_requests) {
        pthread_cond_wait(&q->wait_room, &q->lock);
    }

    struct Qnode* temp = newNode(data);
    // TODO
    q->queue[q->producer % (q->max_requests)] = temp;
    q->producer++;

    pthread_cond_broadcast(&q->wait_data);
    pthread_mutex_unlock(&q->lock);
}

struct Qnode* deQueueAndHandle(struct Queue* q)
{
    pthread_mutex_lock(&q->lock);
    
    struct Qnode* node;
    
    while((q->producer - q->consumer)==0)
        pthread_cond_wait(&q->wait_data, &q->lock);

    node = q->queue[q->consumer % (q->max_requests)];
    q->queue[q->consumer % (q->max_requests)] = NULL;
    q->consumer++;
    q->active_threads++;

    pthread_cond_signal(&q->wait_room);
    pthread_mutex_unlock(&q->lock);
    
    return node;
}

void DoneHandle(struct Queue* q) {
    pthread_mutex_lock(&q->lock);
    
    q->active_threads--;
    
    pthread_mutex_unlock(&q->lock);
}



/*
Thread_function { 
    while(1){
        deQueueAndHandle();
        HandleRequest();
        DoneHandle();
    }
}
*/

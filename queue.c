#include "queue.h"

struct Qnode* newNode(int data, struct timeval arrival)
{
    struct Qnode* temp
            = (struct Qnode*)malloc(sizeof(struct Qnode));
    temp->data = data;
    temp->arrival = arrival;
    return temp;
}

struct Queue* createQueue(int max_threads, int max_requests, enum overload_handling schedalg)
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->queue = (struct Qnode**)malloc(sizeof(struct Qnode*) * (size_t)max_requests);
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->wait_room, NULL);
    pthread_cond_init(&q->wait_data, NULL);
    q->schedalg = schedalg;
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

int enQueue(struct Queue* q, int *data, struct timeval arrival)
{
    pthread_mutex_lock(&q->lock);
    // Save orig data, in case of DH data pointer should 
    // be with fd that we want to close.
    // in case of DT data not change.
    int true_data = *data;
    int status = STATUS_SUCCESS;
    // Handle if there is overload.
    if ((q->producer - q->consumer) + q->active_threads >= q->max_requests) {
        status = OverloadHandle(q, data);
    }
    if (status != STATUS_SUCCESS) {
    // In case we handeling DT - Close fd in data pointer
        pthread_mutex_unlock(&q->lock);
        return STATUS_FAIL;
    }

    // If we are here than were handeling BLOCK or DH.
    // Block - we waited till there a place to add new req.
    // DH - we just deleted the old req so the place to add new one.    
    
    struct Qnode* temp = newNode(true_data,arrival);
    // TODO
    q->queue[q->producer % (q->max_requests)] = temp;
    q->producer++;

    pthread_cond_broadcast(&q->wait_data);
    pthread_mutex_unlock(&q->lock);
    // In Case we are in BLOCK
    if(q->schedalg == BLOCK)
        return STATUS_SUCCESS;
    // in case we are on DH data poiner have dt of oldest request , we want to close it. 
    return STATUS_FAIL;
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

    pthread_mutex_unlock(&q->lock);
    
    return node;
}

void DoneHandle(struct Queue* q) {
    pthread_mutex_lock(&q->lock);
    
    q->active_threads--;
    pthread_cond_signal(&q->wait_room);
    
    pthread_mutex_unlock(&q->lock);
}

int OverloadHandle(struct Queue* q, int *data){
    if(q->schedalg == BLOCK) {
        // In Case of block we should wait till will be place for new job
        while((q->producer - q->consumer) + q->active_threads >= q->max_requests)
            pthread_cond_wait(&q->wait_room, &q->lock);
    }
    // In this case we should close the fd and try so lets pipe STATUS_FAIL
    //  and end the enqueue, out function we will Close() and iterate new one.
    else if (q->schedalg == DT) {
        return STATUS_FAIL;
    }
    else if (q->schedalg == DH) {
        // Let's just enqueue the oldest one later in the function will just add other one.
        if(q->producer - q->consumer > 0) {
            struct Qnode *node = q->queue[q->consumer % (q->max_requests)];
            q->queue[q->consumer % (q->max_requests)] = NULL;
            q->consumer++;
            *data = node->data;
            free(node);
        }
    }
    return STATUS_SUCCESS;
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

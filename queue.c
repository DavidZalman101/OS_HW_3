#include "queue.h"
#include "segel.h"

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

void enQueue(struct Queue* q, int data, struct timeval arrival)
{
    pthread_mutex_lock(&q->lock);
    // in case of DT data not change.
    
    // Handle if there is overload.
    if ((q->producer - q->consumer) + q->active_threads >= q->max_requests) {
        if (OverloadHandle(q, data) != STATUS_SUCCESS) {
    // In case we handeling DT or DH/Random with empty wait queue - Close fd in data pointer.
            pthread_mutex_unlock(&q->lock);
            return;
        }
    }

    // If we are here than were handeling BLOCK or DH or Random.
    // Both case we want to just add new node 
    
    struct Qnode* temp = newNode(data,arrival);
    q->queue[q->producer % (q->max_requests)] = temp;
    q->producer++;

    pthread_cond_broadcast(&q->wait_data);
    pthread_mutex_unlock(&q->lock);
    // In Case we are in BLOCK
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

void DoneHandle(struct Queue* q, int data) {
    pthread_mutex_lock(&q->lock);
    
    Close(data);
    q->active_threads--;
    pthread_cond_broadcast(&q->wait_room);
    pthread_mutex_unlock(&q->lock);
}

int OverloadHandle(struct Queue* q, int data){
    if(q->schedalg == BLOCK) {
        // In Case of block we should wait till will be place for new job
        while((q->producer - q->consumer) + q->active_threads >= q->max_requests)
            pthread_cond_wait(&q->wait_room, &q->lock);
    }
    // In this case we should close the fd and try so lets pipe STATUS_FAIL
    //  and end the enqueue, out function we will Close() and iterate new one.
    else if (q->schedalg == DT) {
        Close(data);
        return STATUS_FAIL;
    }
    else if (q->schedalg == DH) {
        // Let's just enqueue the oldest one later in the function will just add other one.
        if(q->producer - q->consumer > 0) { // if queueu is not empty 
            struct Qnode *node = q->queue[q->consumer % (q->max_requests)];
            q->queue[q->consumer % (q->max_requests)] = NULL;
            q->consumer++;
            Close(node->data);
            free(node);
        }
        else {
            Close(data);
            return STATUS_FAIL;
        }
    }
    else if (q->schedalg == RANDOM) {
        // Let's just enqueue the oldest one later in the function will just add other one.
        if(q->producer - q->consumer > 0) { // if queueu is not empty
            int old_consumer = q->consumer;
            int init_reqs_num = q->producer - q->consumer;
            int num_req_to_drop =  (init_reqs_num+1)/2;
            q->consumer = q->producer;
            int* hist = (int*)malloc(sizeof(*hist)*init_reqs_num);

            for(int i=0 ; i<init_reqs_num ; i++) {
            hist[i]=0;
            }

            for(int i=0 ; i<num_req_to_drop ; i++){
                bool success = STATUS_FAIL;
                while (success != STATUS_SUCCESS) {
                    int to_drop = rand() % init_reqs_num;
                    if(hist[to_drop] == 0) {
                        hist[to_drop] = 1;
                        success = STATUS_SUCCESS;
                    }
                }
            }

                 for(int i=0 ; i<init_reqs_num ; i++) {
                    if(hist[i] == 0) {
                        q->queue[q->producer % (q->max_requests)] = q->queue[(old_consumer+i) % (q->max_requests)];
                        q->queue[(old_consumer+i) % (q->max_requests)] = NULL;
                        q->producer++;
                    }
                    else {
                        Close(q->queue[(old_consumer+i) % (q->max_requests)]->data);
                        free(q->queue[(old_consumer+i) % (q->max_requests)]);
                        q->queue[(old_consumer+i) % (q->max_requests)] = NULL;
                    }
                }
            free(hist);
        }
        else {
            Close(data);
            return STATUS_FAIL;
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

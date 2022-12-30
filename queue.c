#include "queue.h"

/*              Qnode Implementions             */
struct Qnode* newNode(int data)
{
    struct Qnode* temp
            = (struct Qnode*)malloc(sizeof(struct Qnode));
    temp->data = data;
    return temp;
}

/*              Queue Implementions             */
struct Queue* createQueue(int max_size)
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->queue = (struct Qnode**)malloc(sizeof(struct Queue*) *(size_t)max_size);
    q->max_size = max_size;
    q->producer = 0;
    q->consumer = 0;
    return q;
}


void destroyQueue(struct Queue* q){
    for(int i=0;i<q->max_size;i++){
        if(q->queue)
            free(q->queue);
    }
    free(q);
}

void enQueue(struct Queue* q, int data, pthread_mutex_t *lock)
{
    while(q->producer - q->consumer >= q->max_size){
        pthread_cond_wait(&q->wait_room, lock);
    }

    struct Qnode* temp = newNode(data);
    q->queue[q->producer & (q->max_size-1)] = temp;
    q->producer++;

    pthread_cond_broadcast(&q->wait_data);
}

void deQueue(struct Queue* q, pthread_mutex_t *lock)
{
    struct Qnode* node;

    while((q->producer - q->consumer)==0)
        pthread_cond_wait(&q->wait_data, lock);

    node = q->queue[q->consumer & (q->max_size-1)];
    q->queue[q->consumer & (q->max_size-1)] = NULL;
    q->consumer++;
    free(node);

    pthread_cond_signal(&q->wait_room);
}

// Function to check if Queue is empty
bool QueueEmpty(struct Queue* q) {
    return (q->consumer - q->producer == 0);
}

// Function to check the Queue size
int QueueSize(struct Queue* q) {
    return (q->consumer - q->producer);
}
// deQueue Specific Node by his data
void deQueueSpecifif(struct Queue* q, int data) {
    int temp_consumer = q->consumer;

    while(temp_consumer - q->producer <=0) {
        struct Qnode* temp;
        temp = q->queue[temp_consumer & (q->max_size-1)];
        if(temp->data == data){
            q->queue[temp_consumer & (q->max_size-1)] = q->queue[q->consumer & (q->max_size-1)];
            q->queue[q->consumer & (q->max_size-1)] = temp;
            deQueue(q);
            break;
        }
    }

}

int QueueNextConcumerData(struct Queue* q) {
    return q->queue[q->consumer & (q->max_size-1)]->data;
}

/*              Queues Implementions             */
// Init the queues
struct Queues* initQueues(int wait_queue_size,int workers_queue_size) {
    struct Queues* queues = (struct Queues*)malloc(sizeof(struct Queues));
    queues->wait_queue = createQueue(user_params.queue_size);
    queues->worker_queue = createQueue(user_params.threads);
    // init locks
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&wait_room, NULL);
    pthread_cond_init(&wait_data, NULL);
}
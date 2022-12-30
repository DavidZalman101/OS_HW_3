#include "segel.h"
#include "request.h"
#include "queue.h"

//
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// HW3: Parse the new arguments too


struct user_params {
    int portnum;
    int threads;
    int queue_size;
    char schedalg[7];
};

void getargs(struct user_params* user_params, int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    user_params->portnum = atoi(argv[1]);
    user_params->threads = atoi(argv[2]);
    user_params->queue_size = atoi(argv[3]);
    strcpy(user_params->schedalg,argv[4]);
}

struct Queue *queue = NULL;

int* thread_static;
int* thread_dynamic;
int* thread_total;

void* thread_func(void* thread_params) {
    while(1){
        struct Qnode* node = deQueueAndHandle(queue);
        Close(node->data);
        free(node);
        DoneHandle(queue);
    }
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, clientlen;
    struct user_params user_params= { 0 };
    struct sockaddr_in clientaddr;

    getargs(&user_params, argc, argv);

    // init queues
    queue = createQueue(user_params.threads, user_params.queue_size);

    pthread_t* threads = (pthread_t*)malloc((size_t)user_params.threads * sizeof(pthread_t));
    for(int i=0 ; i < user_params.threads ; i++ ) {
        pthread_create(&threads[i], NULL, thread_func, NULL);
    }
    
    listenfd = Open_listenfd(user_params.portnum);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        enQueue(queue, connfd);

        // Comment : This is their Comments.
        // HW3: In general, don't handle the request in the main thread.
        // Save the relevant info in a buffer and have one of the worker threads
        // do the work.
        //
        // requestHandle(connfd);

        // Close(connfd);
    }

}






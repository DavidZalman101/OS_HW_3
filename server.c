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
    enum overload_handling schedalg;
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
    if(strcmp("block",argv[4]) == 0) 
        user_params->schedalg = BLOCK;
    else if(strcmp("dt",argv[4]) == 0) 
        user_params->schedalg = DT; 
    else if(strcmp("dh",argv[4]) == 0) 
        user_params->schedalg = DH;
    else if(strcmp("random",argv[4]) == 0) 
        user_params->schedalg = RANDOM;
}

struct Queue *queue = NULL;


void* thread_func(void* thread_params) {
    while(1){
        struct Qnode* node = deQueueAndHandle(queue);
        struct timeval arrival_time = node->arrival;
        struct timeval handle_time;
        gettimeofday(&handle_time,NULL);
        requestHandle(node->data, arrival_time, handle_time, (struct thread_params*)thread_params);
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
    queue = createQueue(user_params.threads, user_params.queue_size, user_params.schedalg);

    struct thread_params* threads = (struct thread_params*)malloc((size_t)user_params.threads * sizeof(*threads));
    for(int i=0 ; i < user_params.threads ; i++ ) {
        threads[i].thread_idx = i;
        threads[i].total_req = 0;
        threads[i].static_req = 0;
        threads[i].dynamic_req = 0;
        pthread_create(&(threads[i].tid), NULL, thread_func, (void *)&threads[i]);
    }
    
    listenfd = Open_listenfd(user_params.portnum);
    while (1) {
        int status;
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        struct timeval arrival;
        gettimeofday(&arrival,NULL);
        status = enQueue(queue, &connfd, arrival);
        if(status != STATUS_SUCCESS) {
            Close(connfd);
        }
    }
}






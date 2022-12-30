#ifndef __REQUEST_H__

struct thread_params {
    pthread_t tid;
    int thread_idx;
    int total_req;
    int static_req;
    int dynamic_req;
};

void requestHandle(int fd,struct timeval arrival, struct timeval handle, struct thread_params* thread);

#endif

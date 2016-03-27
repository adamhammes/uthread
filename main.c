#include <stdio.h>
#include "uthread.h"

void th0() {
    int i;
    for (i = 0; i < 3; i++) {
        if (i == 0) {
            continue;
        }

        printf("This is thread 0.\n");
        char msg[256];
        sprintf(msg, "greeting from %d to %d", 0, i);
        uthread_send(i, msg, 256);

        printf("Thread %d has sent to Thread %d: %s\n", 0, i, msg);
        char *rcv_msg;
        uthread_recv(i, (void **) &rcv_msg);
        printf("Thread %d receives from Thread %d: %s\n", 0, i, rcv_msg);
    }

    printf("Thread 0 exit.\n");
    uthread_exit();
}

void th1() {
    int i;
    for (i = 0; i < 3; i++) {
        if (i == 1) {
            continue;
        }

        printf("This is thread 1.\n");
        char msg[256];
        sprintf(msg, "greeting from %d to %d", 1, i);
        uthread_send(i, msg, 256);

        printf("Thread %d has sent to Thread %d: %s\n", 1, i, msg);
        char *rcv_msg;
        uthread_recv(i, (void **) &rcv_msg);
        printf("Thread %d receives from Thread %d: %s\n", 1, i, rcv_msg);
    }
    printf("Thread 1 exit.\n");
    uthread_exit();
}

void th2() {
    int i;
    for (i = 0; i < 3; i++) {
        if (i == 2) {
            continue;
        }
        printf("\nThis is thread 2.\n");

        char msg[256];
        sprintf(msg, "greeting from %d to %d", 2, i);
        uthread_send(i, msg, 256);
        printf("Thread %d has sent to Thread %d: %s\n", 2, i, msg);

        char *rcv_msg;
        uthread_recv(i, (void **) &rcv_msg);
        printf("Thread %d receives from Thread %d: %s\n", 2, i, rcv_msg);
    }
    printf("Thread 2 exit.\n");
    uthread_exit();
}

int main() {
    uthread_init();
    uthread_create(th0, 1);
    uthread_create(th1, 1);
    uthread_create(th2, 1);
    uthread_exit();
}
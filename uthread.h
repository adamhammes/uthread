#ifndef UTHREAD_H_
#define UTHREAD_H_

void uthread_init();
int uthread_create(void (* func)( ), int priority);
void uthread_exit();
void uthread_yield();

int uthread_send(int tid, void *content, int size);
int uthread_recv(int tid, void **content);

#endif
#include <sched.h>
#include <stdlib.h>
#include <ucontext.h>
#include <string.h>
#include "uthread.h"

// define a queue of user thread records
typedef struct thread_info_s {
    ucontext_t *context;
    struct thread_info_s *next;
    int priority;
    int tid;
} thread_info;

thread_info *head, *current_thread;
thread_info *tid_assigned[100];

// get the next tid to be assigned
// returns -1 if all 100 spots are used up
int next_tid() {
    int i;
    for (i = 0; i < 100; i++) {
        if (tid_assigned[i] == NULL) {
            return i;
        }
    }

    return -1;
}

void enqueue(thread_info *thread) {
    thread_info *cur_thread = head;
    thread_info *last_thread = NULL;

    // make sure the thread is not already queued
    while (cur_thread != NULL) {
        if (cur_thread->tid == thread->tid) {
            return;
        }
        cur_thread = cur_thread->next;
    }

    cur_thread = head;
    while (cur_thread != NULL && thread->priority >= cur_thread->priority) {
        last_thread = cur_thread;
        cur_thread = cur_thread->next;
    }

    if (last_thread == NULL) {
        // need to insert before head
        thread->next = head;
        head = thread;
    } else {
        // inserting between elements or at end
        thread->next = last_thread->next;
        last_thread->next = thread;
    }
}


thread_info *dequeue() {
    thread_info *to_return = head;

    if (head != NULL) {
        head = head->next;
    }

    return to_return;
}

// return the tid of the new thread, or -1 if uthread is a capacity
int uthread_create(void (*func)(), int priority) {
    // check if we can run another thread
    int tid = next_tid();
    if (tid == -1) {
        return tid;
    }

    //construct a thread record
    thread_info *thread;
    thread = (thread_info *) malloc(sizeof(thread_info));
    thread->tid = tid;

    //initialize the context structure
    thread->context = (ucontext_t *) malloc(sizeof(ucontext_t));
    getcontext(thread->context);
    thread->context->uc_stack.ss_sp = malloc(16384);
    thread->context->uc_stack.ss_size = 16384;
    thread->next = NULL;
    thread->priority = priority;

    //make the context for a thread running func
    makecontext(thread->context, func, 0);

    // save the thread record and add it to the queue
    tid_assigned[tid] = thread;
    enqueue(thread);
    return tid;
}

void uthread_exit() {
    if (head == NULL) {
        //all threads are finished, so we can completely exit
        exit(0);
    }

    // free up the thread's tid
    if (current_thread != NULL) {
        tid_assigned[current_thread->tid] = NULL;
    }

    // get the next thread and run
    current_thread = dequeue();
    setcontext(current_thread->context);
}

void uthread_yield() {
    if (head == NULL) {
        // no other threads to yield to; resume execution
        return;
    }

    // save the current thread's information
    thread_info *old_thread = current_thread;
    enqueue(old_thread);

    // fetch and run the thread at the front of the ready queue
    current_thread = dequeue();
    swapcontext(old_thread->context, current_thread->context);
}

typedef struct message_s {
    void *content;
    int content_size;
    struct message_s *next;
} message_info;

message_info *messages [100][100];
int needs_message[100][100];

void save_message(int sender, int destination, void *content, int size) {
    // make a copy of the message content
    void *content_copy = malloc(size);
    memcpy(content_copy, content, size);

    // initialize the message struct
    message_info *new_message = (message_info *) malloc(sizeof (message_info));
    new_message->content = content_copy;
    new_message->content_size = size;
    new_message->next = NULL;

    message_info *cur_message = messages[destination][sender];

    if (cur_message == NULL) {
        // if there are no current messages for this recipient, start the
        // linked list with this message
        messages[destination][sender] = new_message;
        return;
    }

    // otherwise, iterate to the end of the list, then add our new message
    while (cur_message->next != NULL) {
        cur_message = cur_message->next;
    }
    cur_message->next = new_message;
}

int uthread_send(int tid, void *content, int size) {
    save_message(current_thread->tid, tid, content, size);
    thread_info *destination = tid_assigned[tid];

    // if there is no thread with the given tid, then we just return -1
    if (destination == NULL) {
        return -1;
    }

    if (needs_message[tid][current_thread->tid]) {
        // If the receiving thread is currently waiting on a message,
        // put it back into the ready queue.
        enqueue(destination);
    }

    return 0;
}

int uthread_recv(int tid, void **content) {
    needs_message[current_thread->tid][tid] = 1;
    if (messages[current_thread->tid][tid] == NULL) {
        // If the requested message is not there, swap context without
        // putting the current thread into the ready queue.
        // uthread_send() will be responsible for enqueueing the requesting
        // thread if the message becomes available.

        thread_info *old_thread = current_thread;
        current_thread = dequeue();
        swapcontext(old_thread->context, current_thread->context);
    }

    // the only way to get here is if the requested message is ready
    needs_message[current_thread->tid][tid] = 0;
    message_info *message = messages[current_thread->tid][tid];
    messages[current_thread->tid][tid] = message->next;

    *content = message->content;
    return message->content_size;
}

void uthread_init() {
    head = NULL;
    current_thread = NULL;

    int i, j;
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            messages[i][j] = NULL;
        }
    }

    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            needs_message[i][j] = 0;
        }
    }

    for (i = 0; i < 100; i++) {
        tid_assigned[i] = NULL;
    }
}

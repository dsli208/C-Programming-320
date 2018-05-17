/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef SERVER_H
#define SERVER_H

#include "mailbox.h"
#include "thread_counter.h"

/*
 * Thread counter that should be used to track the number of
 * service threads that have been created.
 */
extern THREAD_COUNTER *thread_counter;

/*
 * Thread function for the thread that handles client requests.
 *
 * The arg pointer point to the file descriptor of client connection.
 * This pointer must be freed after the file descriptor has been
 * retrieved.
 */
void *bvd_client_service(void *arg);

/*
 * Thread function for the thread that handles client requests.
 *
 * The arg pointer points to the file descriptor of client connection.
 * This pointer must be freed after the file descriptor has been
 * retrieved.
 */
struct fd_and_mb {
    int fd;
    MAILBOX *mb;
};
/*
 * Once the file descriptor and mailbox have been retrieved,
 * this structure must be freed.
 */
void *bvd_mailbox_service(void *arg);

#endif

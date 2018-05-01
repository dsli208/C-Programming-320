#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <semaphore.h>

#include "debug.h"
#include "server.h"
#include "directory.h"
#include "protocol.h"
#include "thread_counter.h"

/*
 * A thread counter keeps a count of the number of threads that are
 * currently running.  Each time a thread starts, it increments the
 * thread count.  Each time a thread is about to exit, it decrements
 * the thread count.  A thread counter also provides a function that
 * can be called by a thread that wishes to wait for the thread count
 * to drop to zero.  Such a function is useful, for example, in order
 * to achieve clean termination of a multi-threaded application:
 * when termination is desired, the "main" thread takes some action
 * to cause the other threads to terminate and then it waits for the
 * thread count to drop to zero before exiting the program.
 */


struct thread_counter {
    int num_threads;
};

THREAD_COUNTER *tcnt_init() {
    debug("Starting thread counter.");
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    sem_wait(&mutex);

    THREAD_COUNTER *tc = malloc(sizeof(thread_counter));
    tc -> num_threads = 0;

    if (tc != NULL) {
        debug("Thread counter successfully started");
    }
    sem_post(&mutex);

    return tc;
}

/*
 * Finalize a thread counter.
 */
void tcnt_fini(THREAD_COUNTER *tc) {
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    sem_wait(&mutex);

    if (tc != NULL) {
        free(tc);
        tc = NULL;
        debug("Thread counter finished and freed.");
    }

    sem_post(&mutex);

}

/*
 * Increment a thread counter.
 */
void tcnt_incr(THREAD_COUNTER *tc) {
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    sem_wait(&mutex);

    debug("Incrementing from %d to %d", tc -> num_threads, tc -> num_threads + 1);

    if (tc != NULL) {
        tc -> num_threads += 1;
    }

    debug("Threads is now %d", tc -> num_threads);

    sem_post(&mutex);

}

/*
 * Decrement a thread counter, alerting anybody waiting
 * if the thread count has dropped to zero.
 */
void tcnt_decr(THREAD_COUNTER *tc) {

    sem_t mutex;
    sem_init(&mutex, 0, 1);
    sem_wait(&mutex);

    if (tc != NULL) {
        if (tc -> num_threads - 1 <= 0) {
            // FILL IN THIS
        }
        else {
            debug("Decrementing from %d to %d", tc -> num_threads, tc -> num_threads - 1);
            tc -> num_threads -= 1;
            debug("Threads is now %d", tc -> num_threads);
        }
    }

    sem_post(&mutex);

}

/*
 * A thread calling this function will block in the call until
 * the thread count has reached zero, at which point the
 * function will return.
 */
void tcnt_wait_for_zero(THREAD_COUNTER *tc) {
    sem_t mutex;
    sem_init(&mutex, 0, 1);
    sem_wait(&mutex);
    sem_post(&mutex);
}

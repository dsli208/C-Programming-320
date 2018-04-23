/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef THREAD_COUNTER_H
#define THREAD_COUNTER_H

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
typedef struct thread_counter THREAD_COUNTER;

/*
 * Initialize a new thread counter.
 */
THREAD_COUNTER *tcnt_init();

/*
 * Finalize a thread counter.
 */
void tcnt_fini(THREAD_COUNTER *tc);

/*
 * Increment a thread counter.
 */
void tcnt_incr(THREAD_COUNTER *tc);

/*
 * Decrement a thread counter, alerting anybody waiting
 * if the thread count has dropped to zero.
 */
void tcnt_decr(THREAD_COUNTER *tc);

/*
 * A thread calling this function will block in the call until
 * the thread count has reached zero, at which point the
 * function will return.
 */
void tcnt_wait_for_zero(THREAD_COUNTER *tc);

#endif

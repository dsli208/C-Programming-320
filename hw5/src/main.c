#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include "csapp.h"
#include "debug.h"
#include "server.h"
#include "directory.h"
#include "thread_counter.h"

#define BUFFER_SIZE 1024

static void terminate();

THREAD_COUNTER *thread_counter;
volatile int p_flag;
volatile int h_flag;
int port_num = -2;
char *optarg;

int main(int argc, char* argv[]) {
    // First, install the sigaction() handler for SIGHUP
    struct sigaction new_action;
    new_action.sa_handler = terminate;

    sigaction(SIGHUP, &new_action, NULL);
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    char option;

    char *port;
    char *hostname;

    p_flag = 0;
    for (int i = 0; i < argc; i++) {
        option = getopt(argc, argv, "+p:h:q");
        if (option == 'p' && !p_flag) {
            port = optarg;
            p_flag = 1;
            port_num = atoi(port);
            continue;
        }
        else if (option == 'h' && !h_flag) {
            hostname = optarg;
            h_flag = 1;
            (void)hostname;
        }
        // What to do for 'q'?
    }

    // on which the server should listen.

    // Perform required initializations of the thread counter and directory.
    thread_counter = tcnt_init();
    dir_init();

    if (!p_flag) {
        fprintf(stderr, "You MUST specify a PORT NUMBER.\n");

        terminate();
    }

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function bvd_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    listenfd = open_listenfd(port);

    while(1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, (SA*)&clientaddr, &clientlen);
        pthread_create(&tid, NULL, bvd_client_service, connfdp);
        //tcnt_incr(thread_counter);
    }

    fprintf(stderr, "You have to finish implementing main() "
        "before the Bavarde server will function.\n");

    terminate();
}

/*
 * Function called to cleanly shut down the server.
 */
void terminate(int sig) {
    // Shut down the directory.
    // This will trigger the eventual termination of service threads.
    dir_shutdown();

    debug("Waiting for service threads to terminate...");
    tcnt_wait_for_zero(thread_counter);
    debug("All service threads terminated.");

    tcnt_fini(thread_counter);
    dir_fini();
    exit(EXIT_SUCCESS);
}

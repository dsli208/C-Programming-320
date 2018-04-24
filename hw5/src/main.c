#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "debug.h"
#include "server.h"
#include "directory.h"
#include "thread_counter.h"

static void terminate();

THREAD_COUNTER *thread_counter;
volatile int p_flag;
int port_num = -2;
char *optarg;

int main(int argc, char* argv[]) {
    // First, install the sigaction() handler for SIGHUP
    sigaction(SIGHUP, NULL, NULL);
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    char option;

    char *port;
    p_flag = 0;
    for (int i = 0; i < argc; i++) {
        if ((option = getopt(argc, argv, "+p:")) != -1) {
            port = optarg;
            p_flag = 1;
            port_num = atoi(port);
            break;
        }
    }
    // on which the server should listen.

    // Perform required initializations of the thread counter and directory.
    thread_counter = tcnt_init();
    dir_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function bvd_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // loop - maybe put in a separate file?
    while(1) {

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

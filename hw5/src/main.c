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
    sigaction(SIGHUP, NULL, NULL);
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

    // socket --> bind --> listen
    // OR open_listenfd
    int *connfdp = malloc(sizeof(int));
    int socket_descriptor;
    pthread_t tid;
    (void)tid;
    struct sockaddr_in server, client;
    (void)client;
    /*if ((socket_descriptor = open_listenfd(port)) < 0) {
        fprintf(stderr, "You have to finish specifying bind params "
        "before the Bavarde server will function.\n");

        terminate();
    }*/

    socket_descriptor = socket(AF_INET, SOCK_STREAM, 0); // FIX
    if (socket_descriptor < 0) {
        fprintf(stderr, "Could not create socket.\n");

        terminate();
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port_num);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_descriptor, (struct sockaddr*)&server, sizeof(server)) < 0) {
        fprintf(stderr, "You have to finish specifying bind params "
        "before the Bavarde server will function.\n");

        terminate();
    }
    if (listen(socket_descriptor, BUFFER_SIZE) < 0) {
        fprintf(stderr, "You have to finish specifying listen params "
        "before the Bavarde server will function.\n");

        terminate();
    }
    // loop - maybe put in a separate file?
    while(1) {
        // ACCEPT
        socklen_t client_len = sizeof(client);
        int client_fd;
        if ((client_fd = accept(socket_descriptor, (struct sockaddr*)&client, &client_len)) < 0) {
            fprintf(stderr, "Client FD error.\n");

            terminate();
        }

        pthread_create(&tid, NULL, bvd_client_service, connfdp);
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

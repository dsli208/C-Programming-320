#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>

#include "debug.h"
#include "server.h"
#include "directory.h"
#include "protocol.h"
#include "thread_counter.h"

// FINISH THIS, ELSE THIS WILL CAUSE PROBLEMS WHEN RUNNING THE SERVER

int directory_size;
// Directory data structure - doubly linked list
typedef struct directory_info_block {
    char *handle;
    int sockfd;
    MAILBOX *mailbox;
} directory_info_block;

typedef struct directory_node {
    struct directory_info_block *info;
    struct directory_node *next;
    struct directory_node *prev;
} directory_node;

// Head and tail of the linked list
directory_node *directory_head;
directory_node *directory_tail;

// Empty directory?
int empty_directory() {
    if (directory_head == NULL) {
        return 0;
    }
    return 1;
}

// Insert a new node
void insert_new_node(char *handle, int sockfd, MAILBOX *mailbox) {
    directory_info_block *info = malloc(sizeof(directory_info_block));
    info -> handle = malloc(sizeof(char*));

    if (directory_head -> info == NULL) { // First node in the list
        strcpy(info-> handle, handle);
        info -> sockfd = sockfd;
        info -> mailbox = mailbox;
        //directory_node *new_node = malloc(sizeof(directory_node));
        directory_head -> info = info;
    }
    else { // Malloc a new one
        // Setting the new node in the linked list
        directory_node *new_node = (directory_node*)(malloc(sizeof(directory_node)));
        directory_tail -> next = new_node;
        new_node -> prev = directory_tail;
        directory_tail = new_node;

        // Set the new node details
        strcpy(info -> handle, handle);
        info -> sockfd = sockfd;
        info -> mailbox = mailbox;

        // Set the new node itself
        directory_tail -> info = info;
    }
}

/*
 * Initialize the directory.
 */
void dir_init(void) {
    directory_tail = directory_head = malloc(sizeof(directory_node));
    directory_size = 0;
}

/*
 * Shut down the directory.
 * This marks the directory as "defunct" and shuts down all the client sockets,
 * which triggers the eventual termination of all the server threads.
 */
void dir_shutdown(void) {

}

/*
 * Finalize the directory.
 *
 * Precondition: the directory must previously have been shut down
 * by a call to dir_shutdown().
 */
void dir_fini(void) {

}


/*
 * Register a handle in the directory.
 *   handle - the handle to register
 *   sockfd - file descriptor of client socket
 *
 * Returns a new mailbox, if handle was not previously registered.
 * Returns NULL if handle was already registered or if the directory is defunct.
 */
MAILBOX *dir_register(char *handle, int sockfd) {
    // Insert a new node for the new handle in the directory
    MAILBOX *new_mailbox = mb_init(handle);

    insert_new_node(handle, sockfd, new_mailbox);

    directory_size++;

    // FINISHED???
    return new_mailbox;
}


/*
 * Unregister a handle in the directory.
 * The associated mailbox is removed from the directory and shut down.
 */
void dir_unregister(char *handle) {

}

/*
 * Query the directory for a specified handle.
 * If the handle is not registered, NULL is returned.
 * If the handle is registered, the corresponding mailbox is returned.
 * The reference count of the mailbox is increased to account for the
 * pointer that is being returned.  It is the caller's responsibility
 * to decrease the reference count when the pointer is ultimately discarded.
 */
MAILBOX *dir_lookup(char *handle) {
    // Iterate through the linked list until we reach the tail
    directory_node *cursor = directory_head;

    while (cursor != NULL) {
        // For each node, look at the handle
        directory_info_block *info = cursor -> info;

        // If we get a match, return the mailbox from that block
        if (strcmp(info -> handle, handle) == 0) {
            return info -> mailbox;
        }

        // Otherwise, move into the next node
        cursor = cursor -> next;
    }

    // If the handle matches, return the MAILBOX* in the struct
    // Else, return NULL
    return NULL;
}

/*
 * Obtain a list of all handles currently registered in the directory.
 * Returns a NULL-terminated array of strings.
 * It is the caller's responsibility to free the array and all the strings
 * that it contains.
 */
char **dir_all_handles(void) {
    // FIX THIS
    char **all_handles = (char**)calloc(directory_size, sizeof(char*));
    char **all_handles_p = all_handles;
    directory_node *cursor = directory_head;

    while (cursor != NULL) {
        // Copy each handle to char**
        directory_info_block *info = cursor -> info;
        *all_handles_p = malloc((strlen(info->handle) + 1) * sizeof(char));
        strcpy(*all_handles_p, info -> handle);
        // Null terminate the string
        char *null_modify = ((*all_handles_p) + strlen(info->handle));
        *null_modify = '\0';

        all_handles_p++;
        cursor = cursor -> next;
    }
    return all_handles;
}

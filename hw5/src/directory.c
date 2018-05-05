#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <semaphore.h>

#include "debug.h"
#include "server.h"
#include "directory.h"
#include "protocol.h"
#include "thread_counter.h"
#include "mailbox.h"

volatile int shutdown_flag;

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


sem_t mutex;
// Head and tail of the linked list
directory_node *directory_head;
directory_node *directory_tail;

// Empty directory?
int empty_directory() {
    if (!directory_size) {
        return 1;
    }
    return 0;
}

void remove_node(directory_node *node) {
    // Rearrange the links
    directory_node *prev = node -> prev;
    directory_node *next = node -> next;

    if (prev != NULL)
        prev -> next = node -> next;

    if (next != NULL)
        next -> prev = node -> prev;

    if (node == directory_head) {
        if (directory_head == directory_tail || directory_head -> next == NULL) {
            directory_head = directory_tail = NULL;
        }
        else {
            directory_head = directory_head -> next;
        }
    }
    else if (node == directory_tail) {
        directory_tail = directory_tail -> prev;
    }


    // Free everything in memory associated with the node
    directory_info_block *info = node -> info;
    free(info -> handle);
    //free(info -> mailbox);

    free(info);
    free(node);
}

// Insert a new node
void insert_new_node(char *handle, int sockfd, MAILBOX *mailbox) {
    //sem_wait(&mutex);
    directory_info_block *info = malloc(sizeof(directory_info_block));
    info -> handle = malloc(sizeof(char*));

    if (directory_head -> info == NULL) { // First node in the list
        strcpy(info-> handle, handle);
        info -> sockfd = sockfd;
        info -> mailbox = mailbox;
        //directory_node *new_node = malloc(sizeof(directory_node));
        directory_head -> info = info;

        directory_head -> next = NULL;
        directory_head -> prev = NULL;
        debug("The current head handle is %s", directory_head -> info -> handle);
        debug("The current tail handle is %s", directory_tail -> info -> handle);
    }
    else { // Malloc a new one
        // Setting the new node in the linked list
        directory_node *new_node = (directory_node*)(malloc(sizeof(directory_node)));
        debug("The current head handle is %s", directory_head -> info -> handle);
        debug("The current tail handle is %s", directory_tail -> info -> handle);
        directory_tail -> next = new_node;
        new_node -> prev = directory_tail;
        new_node -> next = NULL;
        directory_tail = new_node;

        // Set the new node details
        strcpy(info -> handle, handle);
        info -> sockfd = sockfd;
        info -> mailbox = mailbox;

        // Set the new node itself
        directory_tail -> info = info;

        debug("The current tail handle is %s", directory_tail -> info -> handle);
    }
    //sem_post(&mutex);
}

/*
 * Initialize the directory.
 */
void dir_init(void) {

    directory_tail = directory_head = malloc(sizeof(directory_node));
    directory_head -> info = NULL;
    directory_size = 0;
    shutdown_flag = 0;
    sem_init(&mutex, 0, 1);
}

/*
 * Shut down the directory.
 * This marks the directory as "defunct" and shuts down all the client sockets,
 * which triggers the eventual termination of all the server threads.
 */

void dir_shutdown(void) {
    sem_wait(&mutex);
    directory_node *cursor = directory_head;

    while (cursor != NULL) {
        // For each node, look at the handle
        directory_info_block *info = cursor -> info;

        // Shutdown the fd
        if (info != NULL) {
            debug("Shutting down socket %d", info -> sockfd);
            shutdown(info -> sockfd, SHUT_RDWR);
        }

        // Otherwise, move into the next node
        cursor = cursor -> next;
    }
    debug("Directory successfully shutdown");
    sem_post(&mutex);
}

/*
 * Finalize the directory.
 *
 * Precondition: the directory must previously have been shut down
 * by a call to dir_shutdown().
 */
void dir_fini(void) {
    if (shutdown_flag) {
        sem_wait(&mutex);
        directory_node *cursor = directory_head;

        while (cursor != NULL) {
            // Free the components
            directory_info_block *remove_info = cursor -> info;

            mb_unref(remove_info -> mailbox);

            directory_size--;

            // Not NULL --> remove the node
            remove_node(cursor);

            // Otherwise, move into the next node
            cursor = cursor -> next;
        }
        debug("Directory successfully shutdown");
        sem_post(&mutex);
    }
}


// Helper for dir_unregister
directory_node *dir_unregister_lookup(char *handle) {

    directory_node *cursor = directory_head;

    while (cursor != NULL) {
        // For each node, look at the handle
        directory_info_block *info = cursor -> info;

        // If we get a match, return the node
        if (strcmp(info -> handle, handle) == 0) {
            return cursor;
        }

        // Otherwise, move into the next node
        cursor = cursor -> next;
    }

    return NULL;
}
/*
 * Unregister a handle in the directory.
 * The associated mailbox is removed from the directory and shut down.
 */
void dir_unregister(char *handle) {
    sem_wait(&mutex);
    // Look for the handle
    directory_node *node_to_remove = dir_unregister_lookup(handle);
    //debug("DIR_UNREGISTER mailbox handle %s", mb_get_handle(node_to_remove -> info -> mailbox));
    if (node_to_remove == NULL) {
        sem_post(&mutex);
        return;
    }

    debug("Directory %s found", handle);
    // Not NULL --> unregister mailbox

    directory_info_block *remove_info = node_to_remove -> info;
    shutdown(remove_info -> sockfd, SHUT_RDWR);
    mb_unref(remove_info -> mailbox);
    mb_shutdown(remove_info -> mailbox);

    directory_size--;

    // Not NULL --> remove the node
    remove_node(node_to_remove);

    // Resolution of logout then login again bug
    if (directory_size == 0) {
        directory_head = malloc(sizeof(directory_node));
        directory_tail = directory_head;
    }

    debug("Directory successfully unregistered.");
    sem_post(&mutex);
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
    sem_wait(&mutex);
    directory_node *cursor = directory_head;

    while (cursor != NULL) {
        // For each node, look at the handle
        directory_info_block *info = cursor -> info;

        if (info == NULL) {
            break;
        }

        // If we get a match, return the mailbox from that block
        if (strcmp(info -> handle, handle) == 0) {
            debug("Mailbox %s found", info -> handle);
            mb_ref(info -> mailbox);
            sem_post(&mutex);
            return info -> mailbox;
        }

        // Otherwise, move into the next node
        cursor = cursor -> next;
    }
    sem_post(&mutex);

    // If the handle matches, return the MAILBOX* in the struct
    // Else, return NULL
    return NULL;
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
    if (!empty_directory() && dir_lookup(handle) != NULL) {
        debug("Handle already exists");
        return NULL;
    }
    sem_wait(&mutex);
    // Insert a new node for the new handle in the directory
    MAILBOX *new_mailbox = mb_init(handle);
    debug("Handle is %s", handle);
    debug("new mailbox handle is %s", mb_get_handle(new_mailbox));
    insert_new_node(handle, sockfd, new_mailbox);

    directory_size++;

    // FINISHED???
    mb_ref(new_mailbox);
    sem_post(&mutex);
    return new_mailbox;
}

/*
 * Obtain a list of all handles currently registered in the directory.
 * Returns a NULL-terminated array of strings.
 * It is the caller's responsibility to free the array and all the strings
 * that it contains.
 */
char **dir_all_handles(void) {
    sem_wait(&mutex);
    debug("Directory size is %d", directory_size);
    char **all_handles = (char**)calloc(directory_size + 1, sizeof(char*));
    char **all_handles_p = all_handles;
    directory_node *cursor = directory_head;
    debug("Header listing started.");
    int i = 0;
    while (cursor != NULL) {
        // Copy each handle to char**
        directory_info_block *info = cursor -> info;
        *all_handles_p = malloc((strlen(info->handle) + 1) * sizeof(char));

        debug("This handle is %s", info -> handle);

        strcpy(*all_handles_p, info -> handle);

        debug("strcpy complete");
        // Null terminate the string
        char *null_modify = ((*all_handles_p) + strlen(info->handle));
        *null_modify = '\0';

        all_handles_p++;
        i++;

        if (cursor -> next != NULL) {
            debug("Non-null next");
        }

        cursor = cursor -> next;
    }

    debug("List complete. i is %d", i);
    sem_post(&mutex);
    return all_handles;
}
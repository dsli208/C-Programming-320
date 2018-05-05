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


// V at shutdown, add_message, add_notice

typedef struct mailbox_entry_block {
    MAILBOX_ENTRY *entry;
    struct mailbox_entry_block *next;
    struct mailbox_entry_block *prev;
} ENTRY_BLOCK;

typedef struct mailbox {
    int defunct_flag;
    // Mutex
    sem_t mutex;
    // Reference count
    int reference_count;
    // Handle
    char *handle;
    // Entry queue
    ENTRY_BLOCK *head;
    ENTRY_BLOCK *tail;
    // Hook
    MAILBOX_DISCARD_HOOK *hook;
} MAILBOX;

/*
 * A mailbox provides the ability for its client to set a hook to be
 * called when an undelivered entry in the mailbox is discarded on
 * mailbox finalization.  For example, the hook might arrange for the
 * sender of an undelivered message to receive a bounce notification.
 * The discard hook is not responsible for actually deallocating the
 * mailbox entry; that is handled by the mailbox finalization procedure.
 * There is one special case that the discard hook must handle, and that
 * is the case that the "from" field of a message is NULL.  This will only
 * occur in a mailbox entry passed to discard hook, and it indicates that
 * the sender of the message was in fact the mailbox that is now being
 * finalized.  Since that mailbox can no longer be used, it does not make
 * sense to do anything further with it (and in fact trying to do so would
 * result in a deadlock because the mailbox is locked), so it has been
 * replaced by NULL.
 *
 * The following is the type of discard hook function.
 */
//typedef void (MAILBOX_DISCARD_HOOK)(MAILBOX_ENTRY *);
void discard_hook(MAILBOX_ENTRY *entry) {
    if (entry -> type == MESSAGE_ENTRY_TYPE) {
        MESSAGE message = entry -> content.message;
        MAILBOX *bounce_mb = message.from;
        mb_add_notice(bounce_mb, BOUNCE_NOTICE_TYPE, message.msgid, entry -> body, entry -> length);
        //mb_unref(bounce_mb); // Correct???
    }
}


// Mailbox helper functions
void add_mb_entry(MAILBOX *mb, MAILBOX_ENTRY *new_mailbox_entry) { // CHECK
    //sem_wait(&(mb -> mutex));
    debug("Adding new entry");
    debug("Mb entry called on mailbox whose handle is %s", mb -> handle);
    if (mb -> head == NULL && mb -> tail == NULL) { // Empty list, consider changing && to ||?
        // Construct first node
        mb -> head = malloc(sizeof(ENTRY_BLOCK));
        mb -> head -> entry = new_mailbox_entry;
        mb -> head -> next = NULL;
        mb -> head -> prev = NULL;
        mb -> tail = mb -> head;
        debug("Has the mailbox been altered? Its handle is %s", mb -> handle);
    }
    else {
        ENTRY_BLOCK *new_block = malloc(sizeof(ENTRY_BLOCK));
        new_block -> entry = new_mailbox_entry;
        new_block -> next = NULL;
        new_block -> prev = mb -> tail;

        (mb -> tail) -> next = new_block;
        mb -> tail = new_block;
        debug("Has the mailbox been altered? Its handle is %s", mb -> handle);
    }
    //sem_post(&(mb -> mutex));
    debug("Entry added");
}

/*
 * Create a new mailbox for a given handle.
 * The mailbox is returned with a reference count of 1.
 */
MAILBOX *mb_init(char *handle) {
    MAILBOX *new_mailbox = malloc(sizeof(MAILBOX));
    sem_init(&(new_mailbox -> mutex), 0, 0);
    new_mailbox -> defunct_flag = 0;
    new_mailbox -> reference_count = 1;
    new_mailbox -> handle = strdup(handle);
    debug("Mailbox handle set to %s", new_mailbox -> handle);
    new_mailbox -> head = NULL;
    new_mailbox -> tail = NULL;

    return new_mailbox;
}

/*
 * Set the discard hook for a mailbox.
 */
void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *hook) {
    debug("Setting discard hook");
    //sem_wait(&(mb -> mutex));
    mb -> hook = hook;
    //sem_post(&(mb -> mutex));
    debug("Discard hook set");
}

/*
 * Increase the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is copied,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.
 */
void mb_ref(MAILBOX *mb) {
    debug("Referencing");
    //sem_wait(&(mb -> mutex));
    mb -> reference_count += 1;
    //sem_post(&(mb -> mutex));
    debug("Referenced");
}

/*
 * Decrease the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is discarded,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.  When the reference count reaches zero,
 * the mailbox will be finalized.
 */
void mb_unref(MAILBOX *mb) {
    debug("Unreferencing");
    //sem_wait(&(mb -> mutex));
    mb -> reference_count -= 1;

    if (mb -> reference_count <= 0) {
        //sem_post(&(mb -> mutex));
        //mb_shutdown(mb);
        // mailbox finalized
    }
    debug("Unreferenced");
}

/*
 * Shut down this mailbox.
 * The mailbox is set to the "defunct" state.  A defunct mailbox should
 * not be used to send any more messages or notices, but it continues
 * to exist until the last outstanding reference to it has been
 * discarded.  At that point, the mailbox will be finalized, and any
 * entries that remain in it will be discarded.
 */
void mb_shutdown(MAILBOX *mb) {
    // Mark as defunct
    mb -> defunct_flag = 1;
    // Discard all entries

    // Free handle and hook (CHANGE IF THE HOOK HAS OTHER POINTERS IN IT)
    //free(mb -> handle);
    //free(mb -> hook);
    sem_post(&(mb -> mutex));
}

/*
 * Get the handle associated with a mailbox.
 */
char *mb_get_handle(MAILBOX *mb) {
    //sem_wait(&(mb -> mutex));
    char *handle = mb -> handle;
    //strcpy(handle, mb -> handle);
    //sem_post(&(mb -> mutex));
    return handle;
}

/*
 * Add a message to the end of the mailbox queue.
 *   msgid - the message ID
 *   from - the sender's mailbox
 *   body - the body of the message, which can be arbitrary data, or NULL
 *   length - number of bytes of data in the body
 *
 * The message body must have been allocated on the heap,
 * but the caller is relieved of the responsibility of ultimately
 * freeing this storage, as it will become the responsibility of
 * whomever removes this message from the mailbox.
 *
 * The reference to the sender's mailbox ("from") is conceptually
 * "transferred" from the caller to the new message, so no increase in
 * the reference count is performed.  However, after the call the
 * caller must discard this pointer which it no longer "owns".
 */
void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length) {
    debug("Adding message.  Handle is %s", mb -> handle);
    //sem_wait(&(mb -> mutex));
    MAILBOX_ENTRY *new_entry = malloc(sizeof(MAILBOX_ENTRY));
    new_entry -> type = MESSAGE_ENTRY_TYPE;

    // Allocate the message
    MESSAGE *message = malloc(sizeof(MESSAGE));

    // Set message values
    message -> msgid = msgid;
    message -> from = from;

    // Set the remaining entry values
    new_entry -> content.message = *message;
    new_entry -> body = body;
    new_entry -> length = length;

    // Add it to the entries queue
    add_mb_entry(mb, new_entry);

    sem_post(&(mb -> mutex));
    debug("Message added to mb with handle %s", mb -> handle);
}

/*
 * Add a notice to the end of the mailbox queue.
 *   ntype - the notice type
 *   msgid - the ID of the message to which the notice pertains
 *   body - the body of the notice, which can be arbitrary data, or NULL
 *   length - number of bytes of data in the body
 *
 * The notice body must have been allocated on the heap, but the
 * caller is relieved of the responsibility of ultimately freeing this
 * storage, as it will become the responsibility of whomever removes
 * this notice from the mailbox.
 */
void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid, void *body, int length) {
    debug("Adding notice. Header is %s", mb -> handle);
    //sem_wait(&(mb -> mutex));
    MAILBOX_ENTRY *new_entry = malloc(sizeof(MAILBOX_ENTRY));
    new_entry -> type = NOTICE_ENTRY_TYPE;

    // Allocate the notice
    NOTICE *notice = malloc(sizeof(NOTICE));

    // Set the notice values
    notice -> type = ntype;
    notice -> msgid = msgid;

    // Set the remaining entry values
    new_entry -> content.notice = *notice;
    new_entry -> body = body;
    new_entry -> length = length;

    // Add it to the entries queue
    add_mb_entry(mb, new_entry);

    sem_post(&(mb -> mutex));
    debug ("Notice added.  Header is %s", mb -> handle);
}

/*
 * Remove the first entry from the mailbox, blocking until there is
 * one.  The caller assumes the responsibility of freeing the entry
 * and its body, if present.  In addition, if it is a message entry,
 * the caller must decrease the reference count on the "from" mailbox
 * to account for the destruction of the pointer.
 *
 * This function will return NULL in case the mailbox is defunct.
 * The thread servicing the mailbox should use this as an indication
 * that service should be terminated.
 */
MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb) {
    debug("Retrieving next entry for mailbox with handle %s", mb -> handle);
    sem_wait(&(mb -> mutex));
    if ((mb -> defunct_flag != 0) && mb -> head == NULL) {
        //sem_post(&(mb -> mutex));
        debug("next entry retrieved --> NULL");
        return NULL;
    }
    else if (mb -> head == mb -> tail) {
        ENTRY_BLOCK *mb_entry_block = mb -> head;
        mb -> head = mb -> tail = NULL;
        //sem_post(&(mb -> mutex));
        debug("next entry retrieved and list is empty");
        MAILBOX_ENTRY *mb_entry = mb_entry_block -> entry;
        if (mb_entry -> type == MESSAGE_ENTRY_TYPE) {
            MESSAGE message = mb_entry -> content.message;
            mb_unref(message.from);
            debug("Has the mailbox been altered? Its handle is %s", mb -> handle);
        }
        return mb_entry;
    }
    else {
        ENTRY_BLOCK *mb_entry_block = mb -> head;
        mb -> head = (mb -> head) -> next;
        //sem_post(&(mb -> mutex));
        debug("next entry retrieved");
        MAILBOX_ENTRY *mb_entry = mb_entry_block -> entry;
        if (mb_entry -> type == MESSAGE_ENTRY_TYPE) {
            MESSAGE message = mb_entry -> content.message;
            mb_unref(message.from);
            debug("Has the mailbox been altered? Its handle is %s", mb -> handle);
        }
        return mb_entry;
    }
}

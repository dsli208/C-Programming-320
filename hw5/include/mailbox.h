/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef MAILBOX_H
#define MAILBOX_H

#include <stdio.h>
#include <stdlib.h>

/*
 * A mailbox is a queue that contains two types of entries:
 * "messages" and "notices".
 *
 * A message is a user-generated transmission from one client to another.
 * it contains a message ID that uniquely identifies the message,
 * the mailbox of the sender of the message, a body that may consist of
 * arbitary data, and a length field that specifies the number of bytes
 * of data in the body.
 *
 * A notice is a server-generated transmission that informs a client
 * about the disposition of a previously sent message.  A notice contains
 * a type field and the message ID of the message to which it pertains.
 *
 * The reason a message contains information about the sender is so that
 * a notice concerning the disposition of the message can be sent back to
 * the sender when the message is either delivered or bounced.
 * A message contains the mailbox of the sender, rather than just the
 * sender's handle, to avoid the need to make multiple copies of the handle,
 * each of which would require allocation, and, ultimately, freeing.
 * In order to be sure that the mailbox referred to by a message is
 * guaranteed not to have been deallocated as long as the message still
 * exists, when a message is constructed, the reference count of the
 * sender's mailbox is incremented to account for the pointer stored in
 * the message.  When the message is ultimately discarded, it is the
 * responsibility of whomever discards it to decrease the reference count
 * of the mailbox to account for the pointer to it that is being destroyed.
 */
typedef struct mailbox MAILBOX;

typedef struct message {
    int msgid;
    MAILBOX *from;
} MESSAGE;

/*
 * A notice can be an ACK notice, which positively acknowledges a
 * previous client request, a NACK notice, which negatively
 * acknowledges a previous client request, a bounce notice, which
 * informs a client that a previously sent message was not delivered,
 * or a delivery notice (return receipt), which informs a client that
 * a previously sent message has been delivered to its intended
 * recipient.
 */
typedef enum { ACK_NOTICE_TYPE,
	       NACK_NOTICE_TYPE,
	       BOUNCE_NOTICE_TYPE,
	       RRCPT_NOTICE_TYPE }
    NOTICE_TYPE;

typedef struct notice {
    NOTICE_TYPE type;
    int msgid;
} NOTICE;

/*
 * An entry in a mailbox can be either a message entry or a notice entry.
 */
typedef enum { MESSAGE_ENTRY_TYPE, NOTICE_ENTRY_TYPE } ENTRY_TYPE;

typedef struct mailbox_entry {
    ENTRY_TYPE type;
    union {
	MESSAGE message;
	NOTICE notice;
    } content;
    void *body;
    int length;
} MAILBOX_ENTRY;

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
typedef void (MAILBOX_DISCARD_HOOK)(MAILBOX_ENTRY *);

/*
 * Create a new mailbox for a given handle.
 * The mailbox is returned with a reference count of 1.
 */
MAILBOX *mb_init(char *handle);

/*
 * Set the discard hook for a mailbox.
 */
void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *);

/*
 * Increase the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is copied,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.
 */
void mb_ref(MAILBOX *mb);

/*
 * Decrease the reference count on a mailbox.
 * This must be called whenever a pointer to a mailbox is discarded,
 * so that the reference count always matches the number of pointers
 * that exist to the mailbox.  When the reference count reaches zero,
 * the mailbox will be finalized.
 */
void mb_unref(MAILBOX *mb);

/*
 * Shut down this mailbox.
 * The mailbox is set to the "defunct" state.  A defunct mailbox should
 * not be used to send any more messages or notices, but it continues
 * to exist until the last outstanding reference to it has been
 * discarded.  At that point, the mailbox will be finalized, and any
 * entries that remain in it will be discarded.
 */
void mb_shutdown(MAILBOX *mb);

/*
 * Get the handle associated with a mailbox.
 */
char *mb_get_handle(MAILBOX *mb);

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
void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length);

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
void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid, void *body, int length);

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
MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb);

#endif

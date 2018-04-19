/**
 * === DO NOT MODIFY THIS FILE ===
 * If you need some other prototypes or constants in a header, please put them
 * in another header file.
 *
 * When we grade, we will be replacing this file with our own copy.
 * You have been warned.
 * === DO NOT MODIFY THIS FILE ===
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * The "Bavarde" chat server protocol.
 *
 * This header file specifies the format of communication between the
 * Bavarde server and its clients.  To avoid confusion, we will use
 * the term "message" to refer to a message sent from one client to
 * another, and "packet" to refer to a single message sent at the
 * protocol level.  A full-duplex, stream-based (i.e. TCP) connection
 * is used between a client and the server.  Communication is effected
 * by the client and server sending "packets" to each other over this
 * connection.  Each packet consists of a fixed-length header, with
 * fields in network byte order, followed by a payload whose length is
 * specified in the header.  Each packet sent from a client to a
 * server engenders either a positive acknowledgement (ACK) or a
 * negative acknowledgement (NACK) notice from the server to the
 * client.  In addition, the server may send other packets
 * asynchronously to the client.  These packets may be sent at any
 * time and are not acknowledged by the client.
 *
 * The following are the packet types in the protocol:
 *
 * Client-to-server requests, acknowledged by server:
 *   LOGIN:  Log client into the system
 *   LOGOUT: Log client out of the system
 *   USERS: Get list of users on the system
 *   SEND: Send a message to a user
 *
 * Server-to-client notices, not acknowledged by client:
 *   ACK: Positive acknowledgement of previous server-to-client packet
 *   NACK: Negative acknowledgement of previous server-to_client packet
 *   DLVR: Delivery of a message previously sent by SEND
 *   RRCPT: Notice of successful delivery of a previously sent message
 *   BOUNCE: Notice of unsuccessful delivery of a previously sent message
 */

typedef enum {
    BVD_NO_PKT,  // Unused
    BVD_LOGIN_PKT, BVD_LOGOUT_PKT, BVD_USERS_PKT, BVD_SEND_PKT,
    BVD_ACK_PKT, BVD_NACK_PKT, BVD_DLVR_PKT, BVD_RRCPT_PKT, BVD_BOUNCE_PKT
} bvd_packet_type;

/*
 * Each message consists of a fixed size header, followed by a
 * variable-size payload.  Multi-byte fields in the header are in
 * network byte order.
 *
 * The following structure defines the header format:
 */
typedef struct {
    uint8_t type;		   // Type of the packet
    uint32_t payload_length;       // Length of payload
    uint32_t msgid;                // Unique ID of message to which packet pertains
    uint32_t timestamp_sec;        // Seconds field of time packet was sent
    uint32_t timestamp_nsec;       // Nanoseconds field of time packet was sent
} bvd_packet_header;

/*
 * The msgid field in the packet header should contain a
 * client-generated value that uniquely identifies a particular
 * message.  The client may use any convenient technique to produce
 * the value stored in this field.  Possibilities are: message
 * sequence numbers, timestamps, and hashes of message content.  These
 * unique identifiers are not used by the bavarde server; they are
 * simply passed through in order to permit a client to associate
 * notifications from the server (such as delivery and nondelivery
 * notifications) with the messages to which they pertain.
 *
 * The actual body of a message is sent in the variable-length payload
 * part of a packet, which immediately follows the fixed-length header
 * and which consists of exactly payload_length bytes of data.  The
 * first line of a message contains either the handle of the intended
 * receiver (in case of a message being sent), or the handle of the
 * sender (in case of a message being delivered).  This line is
 * terminated by the \r\n line termination sequence.  Following the
 * line termination sequence is the body of the message, which may be
 * in an arbitrary format (even binary).  The total number of bytes
 * contained in the first line of the message, plus the line
 * termination sequence, plus the message body must be exactly equal
 * to the payload_length specified in the packet header.
 *
 * Format of message sent by client:
 *   (handle of receiver)\r\n(message body)
 *
 * Format of message delivered to client:
 *   (handle of sender)\r\n(message body)
 *
 * In the case of a login request, the payload part of the packet
 * contains just the requested handle and the message body is omitted.
 */

/*
 * Send a packet with a specified header and payload.
 *   fd - file descriptor on which packet is to be sent
 *   hdr - the packet header, with multi-byte fields in host byte order
 *   payload - pointer to packet payload, or NULL, if none.
 *
 * Multi-byte fields in the header are converted to network byte order
 * before sending.  The header structure passed to this function may
 * be modified as a result.
 *
 * On success, 0 is returned.
 * On error, -1 is returned and errno is set.
 */
int proto_send_packet(int fd, bvd_packet_header *hdr, void *payload);

/*
 * Receive a packet, blocking until one is available.
 *  fd - file descriptor from which packet is to be received
 *  hdr - pointer to caller-supplied storage for packet header
 *  payload - variable into which to store payload pointer
 *
 * The returned header has its multi-byte fields in host byte order.
 *
 * If the returned payload pointer is non-NULL, then the caller
 * is responsible for freeing the storage.
 *
 * On success, 0 is returned.
 * On error, -1 is returned, payload and length are left unchanged,
 * and errno is set.
 */
int proto_recv_packet(int fd, bvd_packet_header *hdr, void **payload);

#endif

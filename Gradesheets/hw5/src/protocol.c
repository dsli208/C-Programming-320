#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>

#include "debug.h"
#include "server.h"
#include "directory.h"
#include "protocol.h"
#include "thread_counter.h"

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
int proto_send_packet(int fd, bvd_packet_header *hdr, void *payload) {
    //hdr = malloc(sizeof(bvd_packet_header));
    struct timespec t;
    (void)t;
    debug("Starting write");

    debug("Contents of hdr: %x %x %x %x %x", hdr -> type, hdr -> payload_length, hdr -> msgid, hdr -> timestamp_sec, hdr -> timestamp_nsec);
    uint32_t pay_len = hdr -> payload_length;
    //hdr -> type = (uint8_t)htonl(hdr -> type);
    hdr -> payload_length = htonl(hdr -> payload_length);
    hdr -> msgid = htonl(hdr -> msgid);
    hdr -> timestamp_sec = htonl(hdr -> timestamp_sec);//clock_gettime(CLOCK_MONOTONIC, &t);
    hdr -> timestamp_nsec = htonl(hdr -> timestamp_nsec);//clock_gettime(CLOCK_MONOTONIC, &t);
    debug("Contents of hdr: %x %x %x %x %x", hdr -> type, hdr -> payload_length, hdr -> msgid, hdr -> timestamp_sec, hdr -> timestamp_nsec);

    // Writing to header????
    int write_return = write(fd, hdr, sizeof(*hdr));
    debug("Write returns: %d", write_return);

    uint32_t hdr_size = sizeof(*hdr);

    while (write_return) {
        if (write_return < 0) {
            return -1;
        }
        else if (write_return == 0) {
            // break?
        }
        hdr_size -= write_return;
        write_return = write(fd, hdr, hdr_size);
    }

    if (hdr -> payload_length > 0) {
        //debug("payload before calloc: %s", (char *)payload);
        //payload = calloc(1, hdr -> payload_length);
        //debug("payload after calloc: %s", (char *)payload);
        //uint32_t pay_len = hdr -> payload_length;
        // The line below causes an infinite loop
        int nest_write_return = write(fd, payload, pay_len);
        while (nest_write_return) {
            debug("Nest write returns: %d", nest_write_return);
            if (nest_write_return < 0) {
                return -1;
            }
            pay_len -= nest_write_return;
            nest_write_return = write(fd, payload, pay_len);
            debug("Nest write return is now %d", nest_write_return);
        }
    }

    return 0;
}

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
int proto_recv_packet(int fd, bvd_packet_header *hdr, void **payload) {
    // IO ERROR - STRUCT DIFFERENCES - NTOHL or HTONL?
    // STILL NEED TO HANDLE SHORT COUNT CASES
    //hdr = malloc(sizeof(bvd_packet_header));

    struct timespec t;
    (void)t;
    size_t hdr_size = sizeof(*hdr);
    debug("Receiving packet");
    debug("size of header is %lu", hdr_size);
    int read_return = read(fd, hdr, hdr_size);
    debug("Read returns: %d", read_return);

    while (read_return) {
        if (read_return < 0)
            return -1;
        else if (read_return == 0) {
            //break; // Done reading
        }
        hdr_size -= read_return;
        read_return = read(fd, hdr, hdr_size);
    }

    debug("First read done.");

    debug("Contents of hdr: %x %x %x %x %x", hdr -> type, hdr -> payload_length, hdr -> msgid, hdr -> timestamp_sec, hdr -> timestamp_nsec);

    //hdr -> type = (uint8_t)ntohl(hdr -> type);
    hdr -> payload_length = ntohl(hdr -> payload_length);
    hdr -> msgid = ntohl(hdr -> msgid);
    hdr -> timestamp_sec = ntohl(hdr -> timestamp_sec);//clock_gettime(CLOCK_REALTIME, &t);
    hdr -> timestamp_nsec = ntohl(hdr -> timestamp_nsec);//clock_gettime(CLOCK_REALTIME, &t);

    debug("Contents of hdr: %x %x %x %x %x", hdr -> type, hdr -> payload_length, hdr -> msgid, hdr -> timestamp_sec, hdr -> timestamp_nsec);
    debug("header converted");

    if (hdr -> payload_length > 0) {
        void *p = calloc(1, hdr -> payload_length);
        *payload = p;
        int nest_read_return = read(fd, *payload, hdr -> payload_length);
        debug("Payload read returns: %d", nest_read_return);
        uint32_t p_length_remaining = hdr -> payload_length;
        while (nest_read_return) {
            debug("Payload read returns: %d", nest_read_return);
            if (nest_read_return < 0) {
                return -1;
            }
            p_length_remaining -= nest_read_return;
            nest_read_return = read(fd, *payload, p_length_remaining);
        }
    }
    debug("packet received");

    return 0;
}

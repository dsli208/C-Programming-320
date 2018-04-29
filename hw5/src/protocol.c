#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

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
    struct timespec t;
    //hdr -> type = (uint8_t)htonl(hdr -> type);
    hdr -> payload_length = htonl(hdr -> payload_length);
    hdr -> msgid = htonl(hdr -> msgid);
    hdr -> timestamp_sec = clock_gettime(CLOCK_MONOTONIC, &t);
    hdr -> timestamp_nsec = clock_gettime(CLOCK_MONOTONIC, &t);

    int write_return = write(fd, hdr, sizeof(*hdr));
    debug("Write returns: %d", write_return);
    if (write_return < 0) {
        return -1;
    }
    else if (write_return == 0) {
        // break?
    }
    if (hdr -> payload_length > 0) {
        int nest_write_return = write(fd, payload, hdr -> payload_length);
        debug("Nest write returns: %d", nest_write_return);
        if (nest_write_return < 0) {
            return -1;
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
    // STILL NEED TO HANDLE SHORT COUNT CASES

    struct timespec t;
    debug("Receiving packet");
    int read_return = read(fd, hdr, sizeof(*hdr));
    debug("Read returns: %d", read_return);
    if (read_return < 0)
        return -1;
    else if (read_return == 0) {
        //break; // Done reading
    }

    debug("First read done.");

    //hdr -> type = (uint8_t)ntohl(hdr -> type);
    hdr -> payload_length = ntohl(hdr -> payload_length);
    hdr -> msgid = ntohl(hdr -> msgid);
    hdr -> timestamp_sec = clock_gettime(CLOCK_MONOTONIC, &t);
    hdr -> timestamp_nsec = clock_gettime(CLOCK_MONOTONIC, &t);

    debug("header converted");

    if (hdr -> payload_length > 0) {
        int nest_read_return = read(fd, payload, hdr -> payload_length);
        debug("Payload read returns: %d", nest_read_return);
        if (nest_read_return < 0) {
            return -1;
        }
        else if (nest_read_return == 0) {
            //break;
        }
    }
    debug("packet received");

    return 0;
}

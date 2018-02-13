#include "url.h"

/*
 * Interface information for routines to make an HTTP connection to a server
 * on the Internet, send a simple HTTP GET request, and interpret the response
 * headers that come back.
 *
 * E. Stark, 11/18/97 for CSE 230
 */

/*
 * Data types used by this package
 */

typedef struct http HTTP;		/* An "HTTP connection" object */

/*
 * Functions for managing an HTTP connection
 *
 * Usage:
 *  (1) Use http_open() to create an open HTTP connection to
 *	the specified IP address and port.
 *
 *  (2) Use http_request() to issue an HTTP GET request for a particular
 *	URL to the server at the other end of an HTTP connection.
 *
 *  (3) [optional] Output additional request headers to the connection
 *	using the FILE pointer returned by http_file().
 *
 *  (4) Use http_response() to finish the request and collect the response
 *	code and response headers.
 *
 *  (5) Examine the response code and message using http_status().
 *	Query the response headers using http_headers_lookup().
 *
 *  (6) Collect the document making up the body of the response using
 *	http_getc()
 *
 *  (7) Close the HTTP connection using http_close();
 *
 * Functions that return int return zero if successful, nonzero if
 *	an error occurs.
 * Functions that return pointers return NULL if unsuccessful.
 *
 * Do not attempt to free() any pointers returned by http_status()
 *	or http_headers_lookup(), and do not use these pointers or the
 *	HTTP object itself again once http_close() has been called.
 */

HTTP *http_open(IPADDR *addr, int port);
int http_close(HTTP *http);
FILE *http_file(HTTP *http);
int http_request(HTTP *http, URL *up);
int http_response(HTTP *http);
int http_getc(HTTP *http);
char *http_status(HTTP *http, int *code);
char *http_headers_lookup(HTTP *http, char *key);


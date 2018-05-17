/*
 * Functions for managing URL's (Web addresses)
 *
 * Usage:
 *  (1) Parse a URL string into a URL object using url_parse();
 *
 *  (2) Extract IP address and port number for http_open()
 *	using url_address() and url_port().
 *	Note: the first time url_address() is used on a URL
 *	object, a DNS lookup will occur.
 *	
 *  (3) If desired, extract the "access method"
 *	("http", "ftp", "mailto", etc.) using url_method(),
 *	the hostname using url_hostname(), and the path on
 *	the remote server using url_path().
 *
 *  (4) When finished with the URL, free it with url_free().
 *
 * Functions that return pointers return NULL if unsuccessful.
 *
 * Do not attempt to free() any pointers returned by url_method()
 *	or url_hostname(), and do not use these pointers or the
 *	URL object itself again once url_free() has been called.
 */

#include <stdio.h>
#include "ipaddr.h"

typedef struct url URL;			/* A "parsed URL" object */

URL *url_parse(char *url);
void url_free(URL *up);
char *url_method(URL *up);
char *url_hostname(URL *up);
int url_port(URL *up);
char *url_path(URL *up);
IPADDR *url_address(URL *up);

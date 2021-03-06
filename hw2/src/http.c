/*
 * Routines for making a HTTP connection to a server on the Internet,
 * sending a simple HTTP GET request, and interpreting the response headers
 * that come back.
 *
 * E. Stark, 11/18/97 for CSE 230
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/socket.h>

#include "url.h"
#include "http.h"
#include "snarf.h"

typedef struct HDRNODE *HEADERS;
HEADERS http_parse_headers(HTTP *http);
void http_free_headers(HEADERS env);


/*
 * Routines to manage HTTP connections
 */

typedef enum { ST_REQ, ST_HDRS, ST_BODY, ST_DONE } HTTP_STATE;

struct http {
  FILE *file;     /* Stream to remote server */
  HTTP_STATE state;   /* State of the connection */
  int code;     /* Response code */
  char version[4];    /* HTTP version from the response */
  char *response;   /* Response string with message */
  HEADERS headers;    /* Reply headers */
};


/*
 * Open an HTTP connection for a specified IP address and port number
 */

HTTP *
http_open(IPADDR *addr, int port)
{
  HTTP *http;
  struct sockaddr_in sa;
  int sock;

  if(addr == NULL)
    return(NULL);
  if((http = malloc(sizeof(*http))) == NULL)
    return(NULL);
  else {
    http->code = 0;
    //http->response = malloc(sizeof(char*));
    /*if (output_file != NULL) {
      http->file = fopen(output_file, "w");
    }
    else {
      http->file = stdout;
    }*/
  }
  bzero(http, sizeof(*http));
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    free(http);
    return(NULL);
  }
  bzero(&sa, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  bcopy(addr, &sa.sin_addr.s_addr, sizeof(struct in_addr));
  if(connect(sock, (struct sockaddr *)(&sa), sizeof(sa)) < 0
     || (http->file = fdopen(sock, "w+")) == NULL) {
    free(http);
    close(sock);
    return(NULL);
  } // Problem - the above if statement triggers
  http->state = ST_REQ;
  return(http);
}

/*
 * Close an HTTP connection that was previously opened.
 */

int
http_close(HTTP *http)
{
  int err;

  err = fclose(http->file);
  free(http->response);
  free(http);
  return(err);
}

/*
 * Obtain the underlying FILE in an HTTP connection.
 * This can be used to issue additional headers after the request.
 */

FILE *
http_file(HTTP *http)
{
  return(http->file);
}

/*
 * Issue an HTTP GET request for a URL on a connection.
 * After calling this function, the caller may send additional
 * headers, if desired, then must call http_response() before
 * attempting to read any document returned on the connection.
 */

int http_request(HTTP *http, URL *up)
{
  void *prev;

  if(http->state != ST_REQ)
    return(1);
  /* Ignore SIGPIPE so we don't die while doing this */
  prev = signal(SIGPIPE, SIG_IGN);
  if(fprintf(http->file, "GET %s://%s:%d%s HTTP/1.0\r\nHost: %s\r\n",
       url_method(up), url_hostname(up), url_port(up),
       url_path(up), url_hostname(up)) == -1) {
    signal(SIGPIPE, prev);
    return(1);
  }
  http->state = ST_HDRS;
  signal(SIGPIPE, prev);
  return(0);
}

/*
 * Finish outputting an HTTP request and read the reply
 * headers from the response.  After calling this, http_getc()
 * may be used to collect any document returned as part of the
 * response.
 */

int
http_response(HTTP *http)
{
  void *prev;
  char *response = NULL;
  int len = 0;

  if(http == NULL) {
    return(1);
  }

  if(http->state != ST_HDRS)
    return(1);
  /* Ignore SIGPIPE so we don't die while doing this */
  prev = signal(SIGPIPE, SIG_IGN);
  if(fprintf(http->file, "\r\n") == -1
     || fflush(http->file) == EOF) {
    signal(SIGPIPE, prev);
    return(1);
  }
  rewind(http->file);
  signal(SIGPIPE, prev);
  //response = fgetln(http->file, &len);
  FILE *file = http -> file;
  len = sizeof(*file)/sizeof(char);
  size_t n = (size_t)len;

  getline(&response, &n, file);
  if(response == NULL
     || (http->response = malloc(len+1)) == NULL) {
    //free(http->response);
    return(1);
  }
  strncpy(http->response, response, len);
  do {
    http->response[len] = '\0';
    len--;
  }
  while(len >= 0 &&
  (http->response[len] == '\r'
   || http->response[len] == '\n' || http->response[len] == '\0')); // remove semi-colon?
  if(sscanf(http->response, "HTTP/%3s %d ", http->version, &http->code) != 2)
    return(1);
  http->headers = http_parse_headers(http); // PROGRAM SEGFAULTS HERE
  http->state = ST_BODY;
  return(0);
}

/*
 * Retrieve the HTTP status line and code returned as the
 * first line of the response from the server
 */

char *
http_status(HTTP *http, int *code)
{
  if(http->state != ST_BODY)
    return(NULL);
  if(code != NULL) {
    *code = http->code;
    //http->code = *code;
  }
  return(http->response);
}

/*
 * Read the next character of a document from an HTTP connection
 */

int
http_getc(HTTP *http)
{
  if(http->state != ST_BODY)
    return(EOF);
  return(fgetc(http->file));
}

/*
 * Routines for parsing the RFC822-style headers that come back
 * as part of the response to an HTTP request.
 */

// "LINKED LIST"

typedef struct HDRNODE {
    char *key;
    char *value;
    struct HDRNODE *next;
} HDRNODE;

/*
 * Function for parsing RFC 822 header lines directly from input stream.
 */

HEADERS
http_parse_headers(HTTP *http)
{
    if (http == NULL) {
      return(NULL);
    }
    FILE *f = http->file;
    if (f == NULL) {
      return NULL;
    }
    HEADERS env = NULL, last = NULL; // PROBLEM LINE --> is env the head of the linked list?
    HDRNODE *node;
    int initLen = sizeof(*f)/sizeof(char);
    int len = initLen;
    //int size = 0;
    size_t n = (size_t)len;
    //ssize_t sLen = 0;

    char *line = NULL;
    char *l = NULL;
    char *ll = NULL;
    char *cp = NULL;
    char* valuep = NULL;

    while(((getline(&ll, &n, f))) != EOF) { // INFINITE LOOP
      //len = initLen;
      //size += sLen;
      // Change len to n (size_t form)
       line = l = malloc(n+1);
       //ll = malloc(n + 1);
       l[n] = '\0';
       strncpy(l, ll, n);

       // Fix issue with len constantly being the size of the buffer
       while(l[n - 1] == '\0') {
        n--;
       }

       // Remove \r\n
       while(n > 0 && (l[n-1] == '\n' || l[n-1] == '\r')) {
             l[--n] = '\0';
       }
       if(n == 0) {
         free(line);
         line = NULL;
         //break;
         return env;
       }

       node = malloc(sizeof(HDRNODE));
       //node->key = malloc(sizeof(char*));
       //node->value = malloc(sizeof(char*));

       // initializing ENV if it is the FIRST ONE
       if (env == NULL) {
          env = node;
       }

       node->next = NULL;
       // Copy l to cp, removing any excess whitespace beforehand
       for(cp = l; *cp == ' '; cp++) // Remove ';'
           l = cp;

       //
       for( ; *cp != ':' && *cp != '\0'; cp++);
           if(*cp == '\0' || *(cp+1) != ' ') {
               free(line);
               line = NULL;
               line = NULL;
               //free(ll);
               //ll = NULL;
               free(node);
               node = NULL;
               continue;
           }
       *cp++ = '\0';
       //node->key = l;
       node->key = strdup(l);
       while(*cp == ' ')
           cp++;
       //node->value = cp;
       node->value = strdup(cp);
       for (valuep = node->value; *valuep != '\0'; valuep++) {
          if (*valuep == '\r' || *valuep == '\n') {
            *valuep = '\0';
          }
       }
       /*for(cp = node->key; *cp != '\0'; cp++)
           if(isupper(*cp))
              *cp = tolower(*cp);*/
       if (last != NULL) {
         last->next = node;
         //last = node;
     }
     last = node;
       free(line);
       line = NULL;
       //free(ll);
       //ll = NULL;
       n = (size_t)initLen;

       //free(sLen);

       /*if (node != NULL) {
        free(node);
        node = NULL;
      }*/
    }

    if (node != NULL) {
      free(node);
      node = NULL;
    }

    if (line != NULL) {
        free(line);
        line = NULL;
    }
    return(env);
}

HEADERS
http_get_headers(HTTP *http) {
  return http->headers;
}

/*
 * Free headers previously created by http_parse_headers()
 */

void
http_free_headers(HEADERS env)
{
    HEADERS next;

    while(env != NULL) {
     free(env->key);
     free(env->value);
     next = env->next;
     free(env);
     env = next;
    }
}

/*
 * Find the value corresponding to a given key in the headers
 */

char *
http_headers_lookup(HTTP *http, char *key)
{
    if (http == NULL ){
      return NULL;
    }
    HEADERS env = http->headers;
    while(env != NULL) {
       if(!strcasecmp(env->key, key))
         return(env->value);
       env = env->next;
    }
    return(NULL);
}

/**
Get code from HTTP struct
*/

int
http_get_code(HTTP *http) {
  return http->code;
}

void
http_search_keywords(HTTP *http, char *keywords[]) {
    if (keywords == NULL) {
      return;
    }
// Search for each of the args in the headers
    // Iterate through the "char array"
    //char **keywordStatus = keywords;

     for (int i = 0; keywords[i] != NULL && i < 100; i++) {
      char* keyword = keywords[i];

      HEADERS http_headers = http_get_headers(http);
      HEADERS cursor = http_headers;

      while (cursor != NULL) {
        if (!strcasecmp(cursor->key, keyword)) {
          fprintf(stderr, "%s: %s\n", cursor->key, cursor->value);
          break;
        }
        cursor = cursor->next;
      }
      /*char* result;
      if ((result = http_headers_lookup(http, keyword)) != NULL) {
        fprintf(stderr, "%s\n", result);
      }*/

      //keywordStatus++;

    }

}

/*void http_free_keywords(HTTP* http, char **keywords) {
  (void)http;
  free(keywords);
  keywords = NULL;
}*/

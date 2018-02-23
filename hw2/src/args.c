#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "snarf.h"
#include "http.h"

int opterr;
int optopt;
int optind;
char *optarg;

char *url_to_snarf;
char *output_file;
char **keywords;
char **keywordStatus;

void
parse_args(int argc, char *argv[])
{
  int i;
  char option;
  int contains_o = 0;
  int contains_q = 0;
  int keywordCount = 0;

  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+q:o")) != -1) {
      switch (option) {
        case 'q': {
          info("Query header: %s", optarg);
          contains_q = 1;
          if (keywords == NULL) {
            keywords = malloc(sizeof(char**));
            keywordStatus = keywords;
          }
          *keywordStatus = optarg;
          keywordStatus++;
          keywordCount++;
          break;
        }
        case 'o': {
          if (contains_o) {
            fprintf(stderr, KRED "-%co has already been entered\n" KNRM,
                    optopt);
            exit(-1);
          }
          info("Output file: %s", optarg);
	        output_file = optarg;
          contains_o = 1;
          break;
        }
        case '?': {
          if (optopt != 'h') {
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
            exit(-1);
          }
          else {
          USAGE(argv[0]);
          exit(0);
        }
          break;
        }
        default: {
          //exit(-1);
          break;
        }
      }
    } else if(argv[optind] != NULL) {
	info("URL to snarf: %s", argv[optind]);
	url_to_snarf = argv[optind];
	optind++;
    }
  }

  // Search for each of the args in the headers
  if (contains_q) {
    // Iterate through the "char array"
    keywordStatus = keywords;

    while (keywordStatus != NULL) {
      char* keyword = keywordStatus;

      HEADERS http_headers = http->headers;
      HEADERS cursor = http_headers;

      while (cursor != NULL) {
        if (!strcmp(cursor.key, keyword)) {
          printf(stderror, "%s", cursor.value);
          break;
        }
        cursor = cursor->next;
      }

      keywordStatus++;

    }
  }
}

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
//char **keywords;
char* keywords[100];
char *keywordStatus;

void
parse_args(int argc, char *argv[])
{
  if (argc == 0 || argv == NULL) {
    return;
  }
  outStream = NULL;
  int i;
  char option;
  contains_o = 0;
  contains_q = 0;
  int contains_url = 0;
  keywordCount = 0;
  output_file = NULL;

  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+q:o:")) != -1) {
      switch (option) {
        case 'q': {
          info("Query header: %s", optarg);
          contains_q = 1;
          /*if (keywords == NULL) {
            //keywords = malloc(sizeof(char**));
            keywordStatus = keywords;
          }*/
          keywordStatus = optarg;
          keywords[keywordCount] = keywordStatus;
          //keywordStatus++;
          keywordCount++;
          break;
        }
        case 'o': {
          if (contains_o) {
            fprintf(stderr, KRED "-%co has already been entered\n" KNRM,
                    optopt);
            USAGE(argv[0]);
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
            USAGE(argv[0]);
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
      if (contains_url) {
        exit(-1);
      }
  info("URL to snarf: %s", argv[optind]);
  url_to_snarf = argv[optind];
  optind++;
  contains_url = 1;
    }
  }

  if (!contains_o || output_file == NULL) {
    //outStream = fdopen(1, "w");
    outStream = stdout;
  }
  else {
    outStream = fopen(output_file, "w");
    if (outStream == NULL) {
      USAGE(argv[0]);
      exit(-1);
    }
  }

  /*if (contains_o) {
    fopen(output_file, "w");
  }*/
}

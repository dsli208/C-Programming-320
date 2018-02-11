#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "snarf.h"

int opterr;
int optopt;
int optind;
char *optarg;

char *url_to_snarf;
char *output_file;

void
parse_args(int argc, char *argv[])
{
  int i;
  char option;

  for (i = 0; optind < argc; i++) {
    debug("%d opterr: %d", i, opterr);
    debug("%d optind: %d", i, optind);
    debug("%d optopt: %d", i, optopt);
    debug("%d argv[optind]: %s", i, argv[optind]);
    if ((option = getopt(argc, argv, "+q:o")) != -1) {
      switch (option) {
        case 'q': {
          info("Query header: %s", optarg);
          break;
        }
        case 'o': {
          info("Output file: %s", optarg);
	  output_file = optarg;
          break;
        }
        case '?': {
          if (optopt != 'h')
            fprintf(stderr, KRED "-%c is not a supported argument\n" KNRM,
                    optopt);
          USAGE(argv[0]);
          exit(0);
          break;
        }
        default: {
          break;
        }
      }
    } else if(argv[optind] != NULL) {
	info("URL to snarf: %s", argv[optind]);
	url_to_snarf = argv[optind];
	optind++;
    }
  }
}

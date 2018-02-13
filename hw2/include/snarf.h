#define USAGE(prog_name)                                                       \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "\n%s [-h] [-q keyword] [-o file] URL\n"                           \
            "\n"                                                               \
	    "Retrieves document at URL using HTTP GET request\n"               \
            "\n"                                                               \
            "Option arguments:\n\n"                                            \
            "-h          Displays this usage menu.\n"                          \
            "-q keyword  Queries the HTTP response headers for 'keyword'\n"    \
            "            and outputs matching headers to stderr.\n"            \
            "            May be repeated to select multiple keywords.\n"       \
            "-o file     Retrieved document should be written to 'file',\n"    \
            "            instead of the default stdout.\n"                     \
            "\nPositional arguments:\n\n"                                      \
            "URL         Location of the document to retrieve.\n",             \
            (prog_name));                                                      \
  } while (0)

extern char *url_to_snarf;
extern char *output_file;

void parse_args(int argc, char *argv[]);

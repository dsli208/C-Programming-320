/*
 * Ecran: A program that (if properly completed) supports the multiplexing
 * of multiple virtual terminal sessions onto a single physical terminal.
 */

#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <curses.h>
#include <sys/signal.h>
#include <sys/select.h>

#include "ecran.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);

char *optarg;
char *output_file;

int main(int argc, char *argv[]) {

    initialize();
    char option;
    for (int i = 0; i < argc; i++) {
        if ((option = getopt(argc, argv, "+o:")) != -1) {
            output_file = optarg;
            outStream = fopen(output_file, "w");
            dup2(2, fileno(outStream));
        }
    }
    mainloop();
    // NOT REACHED
}

/*
 * Initialize the program and launch a single session to run the
 * default shell.
 */
static void initialize() {
    curses_init();
    char *path = getenv("SHELL");
    if(path == NULL)
	path = "/bin/bash";
    char *argv[2] = { " (ecran session)", NULL };
    session_init(path, argv);
}

/*
 * Cleanly terminate the program.  All existing sessions should be killed,
 * all pty file descriptors should be closed, all memory should be deallocated,
 * and the original screen contents should be restored before terminating
 * normally.  Note that the current implementation only handles restoring
 * the original screen contents and terminating normally; the rest is left
 * to be done.
 */
static void finalize(void) {
    // REST TO BE FILLED IN
    if (outStream != NULL) {
        fclose(outStream);
    }
    curses_fini();
    exit(EXIT_SUCCESS);
}

/*
 * Helper method to initialize the screen for use with curses processing.
 * You can find documentation of the "ncurses" package at:
 * https://invisible-island.net/ncurses/man/ncurses.3x.html
 */
static void curses_init(void) {
    initscr();
    raw();                       // Don't generate signals, and make typein
                                 // immediately available.
    noecho();
    main_screen = newwin(0, 0, 0, 0); // Don't echo -- let the pty handle it.
    //main_screen = stdscr;
    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);        // Clear the screen.
    status_line = newwin(0, 0, 0, 0);
    refresh();                   // Make changes visible.
}

/*
 * Helper method to finalize curses processing and restore the original
 * screen contents.
 */
void curses_fini(void) {
    endwin();
}

/*
 * Function to read and process a command from the terminal.
 * This function is called from mainloop(), which arranges for non-blocking
 * I/O to be disabled before calling and restored upon return.
 * So within this function terminal input can be collected one character
 * at a time by calling getch(), which will block until input is available.
 * Note that while blocked in getch(), no updates to virtual screens can
 * occur.
 */
void do_command() {
    // Quit command: terminates the program cleanly
    if(wgetch(main_screen) == 'q')
	finalize();
    else
	// OTHER COMMANDS TO BE IMPLEMENTED
	flash();
}

/*
 * Function called from mainloop(), whose purpose is do any other processing
 * that has to be taken care of.  An example of such processing would be
 * to deal with sessions that have terminated.
 */
void do_other_processing() {
    // TO BE FILLED IN
}

void set_status(char *status) {
    // TO BE FILLED IN
}
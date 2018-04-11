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
#include <sys/types.h>
#include <sys/wait.h>

#include "ecran.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);

char *optarg;
char *output_file;

volatile sig_atomic_t flag;
volatile sig_atomic_t o_flag;
volatile sig_atomic_t s_flag;

/*
 * SIGCHLD Handler
 */

void sigchld_handler(int sig) {
    // Do something?
}

void set_status(char *status) {
    wclear(status_line);
    char *c = status;
    while (*c != '\0') {
        waddch(status_line, *c);
        c++;
    }
    wrefresh(status_line);
}

int main(int argc, char *argv[]) {
    //putenv("TERM=ansi");
    o_flag = 0;
    signal(SIGCHLD, sigchld_handler);
    initialize();
    char option;
    for (int i = 0; i < argc; i++) {
        if ((option = getopt(argc, argv, "+o:")) != -1 && !o_flag) {
            o_flag = 1;
            output_file = optarg;
            outStream = fopen(output_file, "w");
            fputc('s', outStream);
            dup2(2, fileno(outStream));

            // Get the rest of the line to serve as your first command

        }
    }
    if (outStream == NULL) {
        outStream = stderr;
    }
    #ifdef DEBUG
        debug("%s", "Hello world");
    #endif
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
    if (session_init(path, argv) == NULL)
        finalize();
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
    for (int i = 0; i < MAX_SESSIONS; i++) {
        // POSSIBLE ERROR CASES?
        session_kill(sessions[i]);
    }
    if (outStream != NULL) {
        set_status("Closing file stream.");
        fflush(outStream);
        fclose(outStream);
    }
    //close(0);
    //close(1);
    //close(2);
    //close(3);

    curses_fini();

    exit(EXIT_SUCCESS);
}

/*
 * Helper function to split up the screen
 *
 */

void split_screen(void) {
    wresize(main_screen, LINES - 1, COLS/2);
    sec_screen = newwin(LINES - 1, COLS/2, COLS/2, 0);
    nodelay(sec_screen, TRUE);
    wclear(sec_screen);
    wrefresh(main_screen);
    wrefresh(sec_screen);
    set_status("Window successfully split.");

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
    main_screen = newwin(LINES - 1, COLS * 2, 0, 0); // Don't echo -- let the pty handle it.
    //main_screen = stdscr;
    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);        // Clear the screen.
    status_line = newwin(1, COLS * 2, LINES - 1, 0);
    nodelay(status_line, TRUE);
    wclear(status_line);
    refresh();              // Make changes visible. MAIN SCREEN ONLY
    set_status("Terminal multiplexor started.");
    wrefresh(status_line);
}

/*
 * Helper method to finalize curses processing and restore the original
 * screen contents.
 */
void curses_fini(void) {
    if (main_screen != NULL)
        delwin(main_screen);
    if (status_line != NULL)
        delwin(status_line);
    if (sec_screen != NULL)
        delwin(sec_screen);
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
    char c = wgetch(main_screen);
    if(c == 'q') {
	   finalize();
    }
    else if (c == 'n') {
        // CREATE NEW TERMINAL SESSION
        char *path = getenv("SHELL");
        if(path == NULL)
            path = "/bin/bash";
        char *argv[2] = { " (ecran session)", NULL };
        SESSION *new_session = session_init(path, argv);
        if (new_session == NULL) {
            set_status("Too many sessions or problem occurred in session creation.");
            return;
        }
        // NOW, SET THE SCREEN TO THE ACTUAL NEW SESSION
        session_setfg(new_session);
    }
    else if (c >= '0' && c <= '9') {
        // If session exists, switch there
        int sessionNum = c - 48;
        SESSION *newSession = sessions[sessionNum];
        if (newSession != NULL) {
            session_setfg(newSession);
        }
        else {
            flash();
            set_status("Session does not exist.");
        }
    }
    else if (c == 'k') {
        char c1 = wgetch(main_screen);
        if (c1 >= '0' && c1 <= '9') {
            int sessionNum = c1 - 48;
            SESSION *sessionToKill = sessions[sessionNum];
            if (sessionToKill != NULL)
                session_kill(sessionToKill);
            else
                flash();
        }
        else {
            flash();
        }
    }
    else if (c == 's') { // SPLIT SCREEN
        split_screen();
    }
    else {
        flash();
    }
	// OTHER COMMANDS TO BE IMPLEMENTED
}

/*
 * Function called from mainloop(), whose purpose is do any other processing
 * that has to be taken care of.  An example of such processing would be
 * to deal with sessions that have terminated.
 */
void do_other_processing() {
    // TO BE FILLED IN
    pid_t reap_pid = waitpid((pid_t)-1, 0, WNOHANG);
}
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
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

#include "ecran.h"
#include "debug.h"

static void initialize();
static void curses_init(void);
static void curses_fini(void);
static void finalize(void);

char *optarg;
char *output_file;

char status1[100] = { 0 };
char status2[100] = { 0 };
int status_line_int;

volatile sig_atomic_t flag;
volatile sig_atomic_t o_flag;
volatile sig_atomic_t s_flag;
volatile sig_atomic_t h_flag;

void set_status_intarg(char *s1, int i, char *s2) {
    wclear(status_line);
    waddstr(status_line, s1);
    waddch(status_line, ' ');
    waddch(status_line, (char)(i + 48));
    waddstr(status_line, s2);

    int n = strlen(s1) + 1 + strlen(s2);
    debug("%d %d\n", COLS, LINES);
    debug("%s%d\n", "Total string length: ", n);
    while (n < COLS - 40) {
        n++;
        waddch(status_line, ' ');
    }

    waddstr(status_line, "Active sessions: ");
    if (activeSessions < 10) {
        waddch(status_line, (char)(activeSessions + 48));
    }
    else {
        waddch(status_line, '1');
        waddch(status_line, '0');
    }

    time_t t = time(NULL);

    debug("%s", ctime(&t));
    waddstr(status_line, ctime(&t));

    // Copy s1 and s2 to their respective global vars
    if (strcmp(status1, s1) != 0)
        strcpy(status1, s1);
    if (strcmp(status2, s2) != 0)
        strcpy(status2, s2);
    if (i != status_line_int)
        status_line_int = i;

    wrefresh(status_line);
}

void set_status(char *status) {
    wclear(status_line);
    waddstr(status_line, status);
    int i = strlen(status);;
    /*char *c = status;
    while (*c != '\0') {
        waddch(status_line, *c);
        c++;
        i++;
    }*/
    while (i < COLS - 40) {
        i++;
        waddch(status_line, ' ');
    }
    waddstr(status_line, "Active sessions: ");
    if (activeSessions < 10) {
        waddch(status_line, (char)(activeSessions + 48));
    }
    else {
        waddch(status_line, '1');
        waddch(status_line, '0');
    }

    time_t t = time(NULL);

    debug("%s", ctime(&t));
    waddstr(status_line, ctime(&t));

    // Copy status to status1 and set status2 to NULL
    if (strcmp(status1, status) != 0)
        strcpy(status1, status);
    strcpy(status2, "");
    status_line_int = -5;

    wrefresh(status_line);
}

/*
 * SIGALRM Handler
 */

void sigalrm_handler(int sig) {
    // case for set_status
    set_status("");
}

/*
 * SIGCHLD Handler
 */

void sigchld_handler(int sig) {
    // Do something?
}

/*char *get_enhanced_statusline() {
    char s[18];
    sprintf(s, "Active sessions: %d", activeSessions);
    return s;
}*/

int main(int argc, char *argv[]) {

    o_flag = 0;

    signal(SIGCHLD, sigchld_handler);
    signal(SIGALRM, sigalrm_handler);

    //alarm(1);

    initialize();
    initSessions();


    char option;
    for (int i = 0; i < argc; i++) {
        debug("%d\n", i);
        if ((option = getopt(argc, argv, "+o:")) != -1 && !o_flag) {
            o_flag = 1;
            output_file = optarg;
            int fd = open(output_file, O_WRONLY | O_CREAT, 0777);

            dup2(fd, 2);
            debug("%s\n", "Hello world!");
            fprintf(stderr, "This is a test.");

            // NOW, check for the rest of the line
            if (i + 3 < argc - 1) {
                for (int j = i + 3; j < argc; j++) {
                    char *c = argv[j];
                    while (*c != '\0') {
                        session_putc(sessions[0], *c);
                        c++;
                    }
                    session_putc(sessions[0], ' ');
                }
                session_putc(sessions[0], '\n');
                //execvp(*(argv + 3), argv + 3);
            }
        }
    }
    if (!o_flag && argc > 1) {
                for (int j = 1; j < argc; j++) {
                    char *c = argv[j];
                    while (*c != '\0') {
                        session_putc(sessions[0], *c);
                        c++;
                    }
                    session_putc(sessions[0], ' ');
                }
                session_putc(sessions[0], '\n');
                //execvp(*(argv + 3), argv + 3);
    }

    //alarm(1);
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
        //fflush(outStream);
        //fclose(outStream);
        set_status("File stream closed.");
    }

    //free(status1); free(status2);
    curses_fini();

    exit(EXIT_SUCCESS);
}

/*
 * Helper function to split up the screen
 *
 */
void exit_help(void) {
    h_flag = 0;
    wclear(help_screen);
    delwin(help_screen);

    // GET CURRENT SESSION AND UPDATE ALL THE LINES IN ITS VSCREEN
    SESSION *curr_session = fg_session;
    update_vscreen(curr_session->vscreen);
    wrefresh(main_screen);
}

void show_help(void) {
    h_flag = 1;
    help_screen = newwin(LINES - 1, COLS, 0, 0);
    nodelay(help_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(help_screen);
    waddstr(help_screen, "ECRAN TERMINAL\n");
    waddstr(help_screen, "This works similar to a regular terminal.\nHowever, it might not be as functional as the real one.");
    waddstr(help_screen, "COMMANDS:\nCTRL + A + N: New session\nCTRL + A + (0-9): Switch sessions\nCTRL + D: Kill current session\nCTRL + A + K + (0-9): Kill session\nCTRL + A + Q: Quit ECRAN\n");
    waddstr(help_screen, "Press ESC and type as you normally would to exit this help screen.");
    wrefresh(help_screen);

    char c = wgetch(help_screen);;
    while(c != 27) {
        c = wgetch(help_screen);
    }
    exit_help();
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
    main_screen = newwin(LINES - 1, COLS, 0, 0); // Don't echo -- let the pty handle it.
    //main_screen = stdscr;
    nodelay(main_screen, TRUE);  // Set non-blocking I/O on input.
    wclear(main_screen);        // Clear the screen.
    status_line = newwin(1, COLS, LINES - 1, 0);
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
    if (main_screen != NULL) {
        wclear(main_screen);
        delwin(main_screen);
    }
    if (status_line != NULL) {
        wclear(status_line);
        delwin(status_line);
    }
    if (sec_screen != NULL) {
        wclear(sec_screen);
        delwin(sec_screen);
    }
    if (help_screen != NULL) {
        wclear(help_screen);
        delwin(help_screen);
    }
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
    /*if (h_flag && c == 27) {
        exit_help();
    }*/
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
            set_status_intarg("Successfully switched to session", sessionNum, "");
        }
        else {
            flash();
            set_status_intarg("Session", sessionNum, " does not exist.");
        }
    }
    else if (c == 'k' || c == EOF) {
        char c1 = wgetch(main_screen);
        if (c1 >= '0' && c1 <= '9') {
            int sessionNum = c1 - 48;
            SESSION *sessionToKill = sessions[sessionNum];
            if (sessionToKill != NULL) {
                session_kill(sessionToKill);
                set_status_intarg("Session", sessionNum, " successfully killed.");
            }
            else {
                flash();
                set_status_intarg("Session", sessionNum, " does not exist.");
            }
        }
        else {
            set_status("Invalid session number.");
            flash();
        }
    }
    else if (c == 's') { // SPLIT SCREEN
        split_screen();
    }
    else if (c == 'h') {
        // HELP SCREEN
        show_help();
    }
    else {
        flash();
    }
	// OTHER COMMANDS TO BE IMPLEMENTED
    //alarm(1);
}

/*
 * Function called from mainloop(), whose purpose is do any other processing
 * that has to be taken care of.  An example of such processing would be
 * to deal with sessions that have terminated.
 */
void do_other_processing() {
    // KILLING WAITING PROCESSES
    pid_t reap_pid = waitpid((pid_t)-1, 0, WNOHANG);
    //alarm(1);
    if (status_line_int < 0) {
        set_status(status1);
    }
    else {
        set_status_intarg(status1, status_line_int, status2);
    }
    wrefresh(main_screen);
}

/*
 * Helper function for termination
 */
void terminate() {
    finalize();
}
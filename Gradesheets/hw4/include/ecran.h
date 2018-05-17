#include <ncurses.h>
#include <curses.h>
#include "session.h"
#include "vscreen.h"
#include "debug.h"

/*
 * The control character used to escape program commands,
 * so that they are not simply transferred as input to the
 * foreground session.
 */
#define COMMAND_ESCAPE 0x1   // CTRL-A
#define COMMAND_SIGCHLD 0x4 // CTRL-D

int mainloop(void);
void do_command(void);
void do_other_processing(void);
void set_status(char *status);
void set_status_intarg(char *s1, int i, char *s2);
void terminate();

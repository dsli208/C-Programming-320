#ifndef VSCREEN_H
#define VSCREEN_H

/*
 * Data structure maintaining information about a virtual screen.
 */

#include <ncurses.h>
#include <curses.h>

typedef struct vscreen VSCREEN;

extern WINDOW *main_screen;
extern WINDOW *sec_screen;
extern WINDOW *status_line;

VSCREEN *vscreen_init(void);
void update_vscreen(VSCREEN *vscreen);
void vscreen_show(VSCREEN *vscreen);
void vscreen_sync(VSCREEN *vscreen);
void vscreen_putc(VSCREEN *vscreen, char c);
void vscreen_fini(VSCREEN *vscreen);

FILE *outStream;

#endif

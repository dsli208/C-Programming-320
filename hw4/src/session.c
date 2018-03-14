/*
 * Virtual screen sessions.
 *
 * A session has a pseudoterminal (pty), a virtual screen,
 * and a process that is the session leader.  Output from the
 * pty goes to the virtual screen, which can be one of several
 * virtual screens multiplexed onto the physical screen.
 * At any given time there is a particular session that is
 * designated the "foreground" session.  The contents of the
 * virtual screen associated with the foreground session is what
 * is shown on the physical screen.  Input from the physical
 * keyboard is directed to the pty for the foreground session.
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "session.h"

SESSION *sessions[MAX_SESSIONS];  // Table of existing sessions
SESSION *fg_session;              // Current foreground session

/*
 * Initialize a new session whose session leader runs a specified command.
 * If the command is NULL, then the session leader runs a shell.
 * The new session becomes the foreground session.
 */
SESSION *session_init(char *path, char *argv[]) {
    for(int i = 0; i < MAX_SESSIONS; i++) {
	if(sessions[i] == NULL) {
	    int mfd = posix_openpt(O_RDWR | O_NOCTTY);
	    if(mfd == -1)
		return NULL; // No more ptys
	    unlockpt(mfd);
	    char *sname = ptsname(mfd);
	    // Set nonblocking I/O on master side of pty
	    fcntl(mfd, F_SETFL, O_NONBLOCK);

	    SESSION *session = calloc(sizeof(SESSION), 1);
	    sessions[i] = session;
	    session->sid = i;
	    session->vscreen = vscreen_init();
	    session->ptyfd = mfd;

	    // Fork process to be leader of new session.
	    if((session->pid = fork()) == 0) {
		// Open slave side of pty, create new session,
		// and set pty as controlling terminal.
		int sfd = open(sname, O_RDWR);
		setsid();
		ioctl(sfd, TIOCSCTTY, 0);
		dup2(sfd, 2); close(sfd); close(mfd);

		// Set TERM environment variable to match vscreen terminal
		// emulation capabilities (which currently aren't that great).
		putenv("TERM=dumb");

		// Set up stdin/stdout and do exec.
		// TO BE FILLED IN
		fprintf(stderr, "EXEC FAILED (did you fill in this part?)\n");
		exit(1);
	    }
	    // Parent drops through
	    session_setfg(session);
	    return session;
	}
    }
    return NULL;  // Session table full.
}

/*
 * Set a specified session as the foreground session.
 * The current contents of the virtual screen of the new foreground session
 * are displayed in the physical screen, and subsequent input from the
 * physical terminal is directed at the new foreground session.
 */
void session_setfg(SESSION *session) {
    fg_session = session;
    // REST TO BE FILLED IN
}

/*
 * Read up to bufsize bytes of available output from the session pty.
 * Returns the number of bytes read, or EOF on error.
 */
int session_read(SESSION *session, char *buf, int bufsize) {
    return read(session->ptyfd, buf, bufsize);
}

/*
 * Write a single byte to the session pty, which will treat it as if
 * typed on the terminal.  The number of bytes written is returned,
 * or EOF in case of an error.
 */
int session_putc(SESSION *session, char c) {
    // TODO: Probably should use non-blocking I/O to avoid the potential
    // for hanging here, but this is ignored for now.
    return write(session->ptyfd, &c, 1);
}

/*
 * Forcibly terminate a session by sending SIGKILL to its process group.
 */
void session_kill(SESSION *session) {
    // TO BE FILLED IN
}

/*
 * Deallocate a session that has terminated.  This function does not
 * actually deal with marking a session as terminated; that would be done
 * by a signal handler.  Note that when a session does terminate,
 * the session leader must be reaped.  In addition, if the terminating
 * session was the foreground session, then the foreground session must
 * be set to some other session, or to NULL if there is none.
 */
void session_fini(SESSION *session) {
    // TO BE FILLED IN
}

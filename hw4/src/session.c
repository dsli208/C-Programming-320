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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <curses.h>
#include <ncurses.h>

#include "session.h"
#include "ecran.h"
#include "vscreen.h"

SESSION *sessions[MAX_SESSIONS];  // Table of existing sessions
SESSION *fg_session;              // Current foreground session

int activeSessions;

int find_current_session(SESSION *session) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i] == session) {
            return i;
        }
    }
    return -1;
}

void initSessions() {
    activeSessions = 1;
}

/*
 * Initialize a new session whose session leader runs a specified command.
 * If the command is NULL, then the session leader runs a shell.
 * The new session becomes the foreground session.
 */
SESSION *session_init(char *path, char *argv[]) {
    for(int i = 0; i < MAX_SESSIONS; i++) {
	if(sessions[i] == NULL) {
	    int mfd = posix_openpt(O_RDWR | O_NOCTTY);
	    if(mfd == -1) {
		  return NULL; // No more ptys
        }
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
		dup2(sfd, 2);close(mfd);

		// Set TERM environment variable to match vscreen terminal
		// emulation capabilities (which currently aren't that great).
		//putenv("TERM=ansi");
        putenv("TERM=dumb");

		// Set up stdin/stdout and do exec.
		// TO BE FILLED IN
        dup2(sfd, 0);
        dup2(sfd, 1); close(sfd);

        execvp(path, argv);
		//fprintf(stderr, "EXEC FAILED (did you fill in this part?)\n");
		//exit(1);
	    }
	    // Parent drops through
	    session_setfg(session);
        activeSessions++;
        set_status_intarg("New terminal session", i, " created successfully");
	    return session;
	}
    }
    set_status("Session table full.");
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
    VSCREEN *session_vscreen = fg_session->vscreen;
    update_vscreen(session_vscreen);
    vscreen_show(session_vscreen);

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
    set_status("Killing session");
    int index = find_current_session(session);
    if (session != NULL) {
        set_status("Killing process.");
        int pid = (session->pid) * -1;
        //pid_t pgid = getsid(pid);
        kill(pid, SIGKILL); // CODE BELOW IS NOT REACHED???
        // CLEANUP/REAPING

        // ?

        // Deallocate the terminated session
        set_status("Session killed.  Cleaning up the mess.");
        sessions[index] = NULL;
        session_fini(session);
        // ANYTHING ELSE?
        activeSessions--;
        set_status("Setting session to NULL. Done.");
        session = NULL;
    }
    else {
        set_status("Could not find session to kill.");
    }
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
    // TO BE FILLED IN _ INCOMPLETE
    close(session->ptyfd);

    // Free all the calloc'd parts - VSCREEN
    set_status("Deallocating vscreen");
    VSCREEN *vscreen_dealloc = session -> vscreen;
    vscreen_fini(vscreen_dealloc);
    // Free all the callo'd parts - CLOSING FILE DESCRIPTORS
    set_status("Closing file descriptors");

    // Set another session as the foreground one - IF THE FOREGROUND SESSION WAS THE ONE TERMINATED
    if (session != fg_session) {
        set_status("Freeing session");
        free(session);
        set_status("Session terminated successfully");
        return;
    }

    // Free all the calloc'd parts - SESSION
    set_status("Freeing session");
    free(session);

    set_status("Searching for replacement session.");
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i] != NULL) {
            set_status("Replacement session found.");
            session_setfg(sessions[i]);
                return;
        }
    }

    terminate();

    // OLD CODE FOR CREATING A SESSION
    /*char *path = getenv("SHELL");
    if(path == NULL)
        path = "/bin/bash";
    char *argv[2] = { " (ecran session)", NULL };
    SESSION *new_session = session_init(path, argv);
    session_setfg(new_session);
    set_status("Session terminated successfully");*/
}

/* See LICENSE file for copyright and license details. */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>

#include "arg.h"
#include "slstatus.h"
#include "util.h"

struct arg {
	const char *(*func)();
	const char *fmt;
	const char *args;
};

char buf[1024];
static int done;
static Display *dpy;

#include "config.h"

static void
terminate(const int signo)
{
	(void)signo;

	done = 1;
}

static void
difftimespec(struct timespec *res, struct timespec *a, struct timespec *b)
{
	res->tv_sec = a->tv_sec - b->tv_sec - (a->tv_nsec < b->tv_nsec);
	res->tv_nsec = a->tv_nsec - b->tv_nsec +
	               (a->tv_nsec < b->tv_nsec) * 1E9;
}

static void
usage(void)
{
	die("usage: %s [-s]", argv0);
}

#define print_array(argarray) \ 
	for (i = len = 0; i < LEN(argarray); i++) { \
		if (!(res = argarray[i].func(argarray[i].args))) { \
			res = unknown_str; \
		} \
		if ((ret = esnprintf(status + len, sizeof(status) - len, \
				    argarray[i].fmt, res)) < 0) { \
			break; \
		} \
		len += ret; \
	}

int
main(int argc, char *argv[])
{
	struct sigaction act;
	struct timespec epoch, start, current, diff, intspec, wait;
	size_t i, len;
	int sflag, ret;
	char status[MAXLEN];
	const char *res;

	sflag = 0;
	ARGBEGIN {
		case 's':
			sflag = 1;
			break;
		default:
			usage();
	} ARGEND

	if (argc) {
		usage();
	}

	memset(&act, 0, sizeof(act));
	act.sa_handler = terminate;
	sigaction(SIGINT,  &act, NULL);
	sigaction(SIGTERM, &act, NULL);

	if (!sflag && !(dpy = XOpenDisplay(NULL))) {
		die("XOpenDisplay: Failed to open display");
	}

	if (clock_gettime(CLOCK_MONOTONIC, &epoch) < 0) {
		die("clock_gettime:");
	}
	while (!done) {
		if (clock_gettime(CLOCK_MONOTONIC, &start) < 0) {
			die("clock_gettime:");
		}
		status[0] = '\0';

		difftimespec(&diff, &start, &epoch);
		if (!(diff.tv_sec * 1000 / primary_statusbar_duration_before_change)) {
			print_array(args);
		}
		else {
			print_array(args2);
			if (diff.tv_sec * 1000 / (primary_statusbar_duration_before_change + secondary_statusbar_duration)) {
				if (clock_gettime(CLOCK_MONOTONIC, &epoch) < 0) {
					die("clock_gettime:");
				}
			}
		}
		if (sflag) {
			puts(status);
			fflush(stdout);
			if (ferror(stdout))
				die("puts:");
		} else {
			if (XStoreName(dpy, DefaultRootWindow(dpy), status)
                            < 0) {
				die("XStoreName: Allocation failed");
			}
			XFlush(dpy);
		}

		if (!done) {
			if (clock_gettime(CLOCK_MONOTONIC, &current) < 0) {
				die("clock_gettime:");
			}
			difftimespec(&diff, &current, &start);

			intspec.tv_sec = interval / 1000;
			intspec.tv_nsec = (interval % 1000) * 1E6;
			difftimespec(&wait, &intspec, &diff);

			if (wait.tv_sec >= 0) {
				if (nanosleep(&wait, NULL) < 0 &&
				    errno != EINTR) {
					die("nanosleep:");
				}
			}
		}
	}

	if (!sflag) {
		XStoreName(dpy, DefaultRootWindow(dpy), NULL);
		if (XCloseDisplay(dpy) < 0) {
			die("XCloseDisplay: Failed to close display");
		}
	}

	return 0;
}

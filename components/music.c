#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <mpd/connection.h>
#include <mpd/client.h>
#include <mpd/tag.h>

#include <X11/Xlib.h>

#include "../util.h"
#define MPDHOST "localhost"
#define MPDPORT 6600

const char *
mpd_current_play() {

	struct mpd_connection *conn = NULL;
	struct mpd_song *song = NULL;
	struct mpd_status *status;
	const char *artist = NULL;
	const char *title = NULL;
	const char *name = NULL;
	const char *url = NULL;
	char *icon = "🎹";
	conn = mpd_connection_new(MPDHOST, MPDPORT, 1000);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS){
		warn("Mpd connection error: %s\n", mpd_connection_get_error_message(conn));
		snprintf(buf, LEN(buf), "%s N/A", icon);
		goto postend;
	}

	status = mpd_run_status(conn);
	if (status == NULL){
		warn("Cannot get mpd status: %s\n", mpd_status_get_error(status));
		snprintf(buf, LEN(buf), "%s N/A", icon);
		goto end;
	}
	enum mpd_state state = mpd_status_get_state(status);
	if (state == MPD_STATE_STOP || state == MPD_STATE_UNKNOWN){
		snprintf(buf, LEN(buf), "%s N/A", icon);
		goto end;
	}
	else if (state == MPD_STATE_PAUSE){
		icon = "⏸";
	}
	else if (state == MPD_STATE_PLAY){
		icon = "▶";
	}

	song = mpd_run_current_song(conn);
	if (song == NULL){
		warn("Error fetching song\n");
		snprintf(buf, LEN(buf), "%s N/A", icon);
		goto preend;
	}
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	name = mpd_song_get_tag(song, MPD_TAG_NAME, 0);
	if (title) {
		if (artist) 
			snprintf(buf, LEN(buf), "%s %s - %s", icon, artist, title);
		else 
			snprintf(buf, LEN(buf), "%s %s", icon, title);
		goto preend;
	}
	else if (name) {
		snprintf(buf, LEN(buf), "%s %s", icon, name);
		goto preend;
	}
	else if (artist) {
		snprintf(buf, LEN(buf), "%s %s", icon, artist);
		goto preend;
	}
	else {
		url = mpd_song_get_uri(song);
		if (url) {
			snprintf(buf, LEN(buf), "%s %s", icon, url);
			goto preend;
		}
	}
preend:
	mpd_song_free(song);
end:
	mpd_status_free(status);
postend:
	mpd_connection_free(conn);

	return buf;
}

/* sxiv: main.c
 * Copyright (c) 2012 Bert Muennich <be.muennich at googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#define _POSIX_C_SOURCE 200112L
#define _MAPPINGS_CONFIG

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include "commands.h"
#include "image.h"
#include "options.h"
#include "thumbs.h"
#include "types.h"
#include "util.h"
#include "window.h"
#include "config.h"
#include "palette.h"

enum {
	INFO_STR_LEN = 256,
	FILENAME_CNT = 1024
};

typedef struct {
	struct timeval when;
	bool active;
	timeout_f handler;
} timeout_t;

/* timeout handler functions: */
void redraw(void);
void reset_cursor(void);
void animate(void);
void clear_resize(void);

appmode_t mode;
img_t img;
tns_t tns;
win_t win;

fileinfo_t *files;
int filecnt, fileidx;
size_t filesize;

int prefix;

bool resized = false;

char win_bar_l[INFO_STR_LEN];
char win_bar_r[INFO_STR_LEN];
char win_title[INFO_STR_LEN];

timeout_t timeouts[] = {
	{ { 0, 0 }, false, redraw },
	{ { 0, 0 }, false, reset_cursor },
	{ { 0, 0 }, false, animate },
	{ { 0, 0 }, false, clear_resize },
};

void cleanup(void) {
	static bool in = false;

	if (!in) {
		in = true;
		img_close(&img, false);
		tns_free(&tns);
		win_close(&win);
	}
}

void check_add_file(char *filename) {
	const char *bn;

	if (filename == NULL || *filename == '\0')
		return;

	if (access(filename, R_OK) < 0) {
		warn("could not open file: %s", filename);
		return;
	}

	if (fileidx == filecnt) {
		filecnt *= 2;
		files = (fileinfo_t*) s_realloc(files, filecnt * sizeof(fileinfo_t));
	}
	if (*filename != '/') {
		files[fileidx].path = absolute_path(filename);
		if (files[fileidx].path == NULL) {
			warn("could not get absolute path of file: %s\n", filename);
			return;
		}
	}
	files[fileidx].loaded = false;
	files[fileidx].name = s_strdup(filename);
	if (*filename == '/')
		files[fileidx].path = files[fileidx].name;
	if ((bn = strrchr(files[fileidx].name , '/')) != NULL && bn[1] != '\0')
		files[fileidx].base = ++bn;
	else
		files[fileidx].base = files[fileidx].name;
	fileidx++;
}

void remove_file(int n, bool manual) {
	if (n < 0 || n >= filecnt)
		return;

	if (filecnt == 1) {
		if (!manual)
			fprintf(stderr, "sxiv: no more files to display, aborting\n");
		cleanup();
		exit(manual ? EXIT_SUCCESS : EXIT_FAILURE);
	}

	if (files[n].path != files[n].name)
		free((void*) files[n].path);
	free((void*) files[n].name);

	if (n + 1 < filecnt)
		memmove(files + n, files + n + 1, (filecnt - n - 1) * sizeof(fileinfo_t));
	if (n + 1 < tns.cnt) {
		memmove(tns.thumbs + n, tns.thumbs + n + 1, (tns.cnt - n - 1) *
		        sizeof(thumb_t));
		memset(tns.thumbs + tns.cnt - 1, 0, sizeof(thumb_t));
	}

	filecnt--;
	if (n < tns.cnt)
		tns.cnt--;
}

void set_timeout(timeout_f handler, int time, bool overwrite) {
	int i;

	for (i = 0; i < ARRLEN(timeouts); i++) {
		if (timeouts[i].handler == handler) {
			if (!timeouts[i].active || overwrite) {
				gettimeofday(&timeouts[i].when, 0);
				TV_ADD_MSEC(&timeouts[i].when, time);
				timeouts[i].active = true;
			}
			return;
		}
	}
}

void reset_timeout(timeout_f handler) {
	int i;

	for (i = 0; i < ARRLEN(timeouts); i++) {
		if (timeouts[i].handler == handler) {
			timeouts[i].active = false;
			return;
		}
	}
}

bool check_timeouts(struct timeval *t) {
	int i = 0, tdiff, tmin = -1;
	struct timeval now;

	gettimeofday(&now, 0);
	while (i < ARRLEN(timeouts)) {
		if (timeouts[i].active) {
			tdiff = TV_DIFF(&timeouts[i].when, &now);
			if (tdiff <= 0) {
				timeouts[i].active = false;
				if (timeouts[i].handler != NULL)
					timeouts[i].handler();
				i = tmin = -1;
			} else if (tmin < 0 || tdiff < tmin) {
				tmin = tdiff;
			}
		}
		i++;
	}
	if (tmin > 0 && t != NULL)
		TV_SET_MSEC(t, tmin);
	return tmin > 0;
}

void update_info(void) {
	int i, fw, pw, fi, ln, rn;
	char frame_info[16];
	const char *size_unit;
	float size = filesize;

	pw = 0;
	for (i = filecnt; i > 0; i /= 10)
		pw++;

	if (mode == MODE_THUMB) {
		if (tns.cnt != filecnt) {
			snprintf(win_bar_l, sizeof win_bar_l, "Loading... %0*d/%d",
			         pw, tns.cnt, filecnt);
		} else {
			fi = snprintf(win_bar_l, sizeof win_bar_l, "%0*d/%d%s",
			              pw, tns.sel + 1, filecnt, BAR_SEPARATOR);
			ln = snprintf(win_bar_l + fi, sizeof win_bar_l - fi, "%s",
			              files[tns.sel].name) + fi;
			if (win_textwidth(win_bar_l, ln, true) > win.w)
				snprintf(win_bar_l + fi, sizeof win_bar_l - fi, "%s",
				         files[tns.sel].base);
		}
		win_set_title(&win, "sxiv");
		win_set_bar_info(&win, win_bar_l, NULL);
	} else {
		size_readable(&size, &size_unit);
		if (img.multi.cnt > 0) {
			fw = 0;
			for (i = img.multi.cnt; i > 0; i /= 10)
				fw++;
			snprintf(frame_info, sizeof frame_info, "%s%0*d/%d",
			         BAR_SEPARATOR, fw, img.multi.sel+1, img.multi.cnt);
		} else {
			frame_info[0] = '\0';
		}
		fi = snprintf(win_bar_l, sizeof win_bar_l, "%0*d/%d%s",
		              pw, fileidx + 1, filecnt, BAR_SEPARATOR);
		ln = snprintf(win_bar_l + fi, sizeof win_bar_l - fi, "%s",
		              files[fileidx].name) + fi;
		rn = snprintf(win_bar_r, sizeof win_bar_r, "%.2f%s%s%dx%d%s%3d%%%s",
		              size, size_unit, BAR_SEPARATOR, img.w, img.h, BAR_SEPARATOR,
		              (int) (img.zoom * 100.0), frame_info);

		if (win_textwidth(win_bar_l, ln, true) +
		    win_textwidth(win_bar_r, rn, true) > win.w)
		{
			snprintf(win_bar_l + fi, sizeof win_bar_l - fi, "%s",
			         files[fileidx].base);
		}
		win_set_bar_info(&win, win_bar_l, win_bar_r);

		snprintf(win_title, sizeof win_title, "sxiv - %s", files[fileidx].name);
		win_set_title(&win, win_title);
	}
}

void redraw(void) {
	if (mode == MODE_IMAGE)
		img_render(&img);
	else
		tns_render(&tns);
	update_info();
	win_draw(&win);
	reset_timeout(redraw);
	reset_cursor();
}

void reset_cursor(void) {
	int i;
	cursor_t cursor = CURSOR_NONE;

	if (mode == MODE_IMAGE) {
		for (i = 0; i < ARRLEN(timeouts); i++) {
			if (timeouts[i].handler == reset_cursor) {
				if (timeouts[i].active)
					cursor = CURSOR_ARROW;
				break;
			}
		}
	} else {
		if (tns.cnt != filecnt)
			cursor = CURSOR_WATCH;
		else
			cursor = CURSOR_ARROW;
	}
	win_set_cursor(&win, cursor);
}

void animate(void) {
	if (img_frame_animate(&img, false)) {
		redraw();
		set_timeout(animate, img.multi.frames[img.multi.sel].delay, true);
	}
}

void clear_resize(void) {
	resized = false;
}


int main(int argc, char **argv) {
	palette_t * p;
	p = load_palettes("tag.palette.yml");
	display_palette(p);
	return 0;
}

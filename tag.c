/* sxiv: tag.c
 * Copyright (c) 2012 Devon Smith <decasm at gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <yaml.h>
#include <Imlib2.h>
#include "window.h"
#include "util.h"
#include "palette.h"
#include "image.h"
#include "tag.h"

#define TAG_C_LOG 0

int text_height;
int button_height;
int tag_bar_left_offset = 10;
int tag_bar_y;
int pad_width = 10;
int pad_height = 10;
int border_width = 1;
int border_height = 1;

rgba_t tag_color;

void set_label_highlight() {
	/* long hex = strtol(TAG_LABEL_SELECTED, NULL, 16);*/
	hex_to_rgba(TAG_LABEL_SELECTED, &tag_color);
}

void set_label_normal() {
	hex_to_rgba(TAG_LABEL_NORMAL, &tag_color);
}

void set_mapping_highlight() {
	hex_to_rgba(TAG_MAPPING_SELECTED, &tag_color);
}

void set_mapping_normal() {
	hex_to_rgba(TAG_MAPPING_NORMAL, &tag_color);
}

void set_font() {
	Imlib_Font font;

	/* set the font cache to 512Kb - again to avoid re-loading */
	imlib_set_font_cache_size(512 * 1024);
	/* add the ./ttfonts dir to our font path - you'll want a notepad.ttf */
	/* in that dir for the text to display */
	imlib_add_path_to_font_path("/usr/share/fonts/TTF");
	/* draw text - centered with the current mouse x, y */
	font = imlib_load_font("times/10");
	if (font) {
		/* set the current font */
		imlib_context_set_font(font);
		imlib_context_set_anti_alias(1);
	}
	else {
		fprintf(stderr, "Font not found: %s\n", "times/8");
	}
}

void draw_button_left_border(int x, int y, int height) {

	Imlib_Image buffer;
	buffer = imlib_create_image(border_width, height);
	if ( buffer == NULL ) {
		fprintf(stderr, "Null buffer second %d, %d\n", x, height);
		return;
	}
	imlib_context_set_image(buffer);

	imlib_context_set_color(0,0,0,255);
	imlib_image_draw_line (0, 0, 0, height, 0);

	imlib_render_image_on_drawable(x, y);
	imlib_free_image();
}

void draw_button_bottom_border(int x, int y, int width) {

	Imlib_Image buffer;
	buffer = imlib_create_image(width, border_height);
	if ( buffer == NULL ) {
		fprintf(stderr, "Null buffer third %d, %d\n", x, width);
		return;
	}
	imlib_context_set_image(buffer);

	imlib_context_set_color(0,0,0,255);
	imlib_image_draw_line (0, 0, width, 0, 0);

	imlib_render_image_on_drawable(x, y);
	imlib_free_image();
}



void render_text(char * text, int button_x, int button_y, int * button_w, int * button_h) {
	Imlib_Image buffer;
	int text_w, text_h;

	/* query the size it will be */
	imlib_get_text_size(text, &text_w, &text_h); 
	*button_w = text_w + pad_width;
	*button_h = text_height + pad_height;

	/* create our buffer image for rendering this update */
	buffer = imlib_create_image(*button_w, *button_h);
	if ( buffer == NULL ) {
		fprintf(stderr, "Null buffer First\n");
		return;
	}
	imlib_context_set_image(buffer);

	imlib_context_set_color(tag_color.red,tag_color.green,tag_color.blue,tag_color.alpha);
	imlib_image_fill_rectangle(0,0, *button_w, *button_h);

	imlib_context_set_color(255, 255, 255, 255);
	imlib_text_draw(pad_width/2, pad_height/2, text); 

	imlib_render_image_on_drawable(button_x, button_y);

	imlib_free_image();
}

void render_button(char key, char * text, int button_x, int button_y, int *button_w, int *button_h, bool bottom) {
	char button_text[1024];
	sprintf(button_text, "%c) %s", key, text);
	render_text(button_text, button_x, button_y, button_w, button_h);

	if ( bottom ) 
		draw_button_bottom_border(button_x, button_y+ *button_h, *button_w);

	draw_button_left_border(button_x, button_y, *button_h);
}

void tag_render_palette_mapping(tag_t *tag) {
	win_t * win;
	palette_t * pal;
	keytagmap_t * ktm;
	int button_x = tag_bar_left_offset, button_y = 0;
	int button_w = 0, button_h = button_height;

	win = tag->win;
	imlib_context_set_drawable(win->pm);

	pal = tag->palette;
	while ( pal->index != tag->current_palette) {
		pal = pal->next;
		if (pal == NULL ) return;
	}
	ktm = pal->tags;

	set_label_normal();
	button_y = tag_bar_y + button_height + 1;
	while ( ktm ) {
		render_button(ktm->key, ktm->tag, button_x, button_y, &button_w, &button_h, false);
		button_x += button_w;
		ktm = ktm->next;
	}
}

void tag_render_palette(tag_t * tag) {
	palette_t * pal;
	win_t * win;
	int button_x = 10, button_y = 0;
	int button_w = 0, button_h = button_height;
	
	win = tag->win;
	imlib_context_set_drawable(win->pm);
	pal = tag->palette;

	button_y = tag_bar_y;

	while ( pal ) {
		if ( tag->current_palette != -1 && tag->current_palette == pal->index) {
			tag_render_palette_mapping(tag);
			set_label_highlight();
		}
		else {
			set_label_normal();
		}
		render_button(pal->index + 0x30, pal->label, button_x, button_y, &button_w, &button_h, true);
		button_x += button_w;

		pal = pal->next;
	}
}

void tag_init(tag_t *tag, win_t *win) {
	palette_t * p_counter;
	int text_w;
	char * text = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	if ( tag->palette == NULL )
		tag->palette = load_palettes("tag.palette.yml");

	tag->win = win;
	tag->tagging_on = true;
	tag->palette_count = 1;
	tag->current_palette = -1;

	p_counter = tag->palette;
	while (p_counter->next != NULL) {
		tag->palette_count++;
		p_counter = p_counter->next;
	}

	set_font();
	set_label_normal();

	/* determine height of text given font size */
	imlib_get_text_size(text, &text_w, &text_height);
	button_height = text_height + pad_height;
}

void tag_free(tag_t *tag) {
	tag->win = NULL;
	tag->tagging_on = false;
	tag->palette_count = 0;
	tag->current_palette = -1;
	free(tag->palette); /* tagging can be turned off, palettes changed on disk, then reloaded */
	tag->palette = NULL;
}

void tag_render(tag_t *tag) {
	win_t *win;
	if (tag == NULL || tag->palette == NULL || tag->win == NULL )
		return;
	win = tag->win;
	tag_bar_y = win->h - (button_height*2) - 2;
	/* imlib_context_set_drawable(win->pm); */
	tag_render_palette(tag);
}


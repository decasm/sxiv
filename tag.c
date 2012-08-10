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
#include "window.h"
#include "util.h"
#include "palette.h"
#include "image.h"
#include "tag.h"

#define TAG_C_LOG 1

void render_text(char * text, int * button_w, int * button_h) {
	Imlib_Image buffer;
	int text_w, text_h;

	/* query the size it will be */
	imlib_get_text_size(text, &text_w, &text_h); 
	*button_w = text_w + 20;
	*button_h = text_h + 20;

	/* create our buffer image for rendering this update */
	buffer = imlib_create_image(*button_w, *button_h);

		/* now we want to work with the buffer */
	imlib_context_set_image(buffer);
	imlib_context_set_color(0,0,0,255);
	imlib_image_fill_rectangle(0,0, *button_w, *button_h);

	/* set the color (black) */
	imlib_context_set_color(255, 255, 255, 255);
	imlib_image_draw_rectangle(2, 2, *button_w - 4, *button_h - 4);
	/* print text to display in the buffer */
	/* draw it */
	imlib_text_draw(10, 10, text); 
}

void render_button(char key, char * text, int * button_w, int * button_h) {
	char button_text[1024];
	sprintf(button_text, "%c) %s", key, text);
	render_text(button_text, button_w, button_h);
}

void tag_render_palette(tag_t *tag, int palette_index) {
	Imlib_Font font;
	win_t * win;
	palette_t * pal;
	keytagmap_t * ktm;
	int label_x = 10, label_y = 0;
	int button_w = 0, button_h = 0;
	
	win = tag->win;
	pal = tag->palette;
	while ( pal->index != palette_index) {
		pal = pal->next;
	}
	ktm = pal->tags;

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

	imlib_context_set_drawable(win->pm);
	label_y = win->h - 50;
	while ( ktm ) {
		fprintf(stderr, "Palette key tag : %c -> %s\n", ktm->key,ktm->tag);
		render_button(ktm->key, ktm->tag, &button_w, &button_h);
		imlib_render_image_part_on_drawable_at_size(0, 0, button_w, button_h, label_x, label_y, button_w, button_h);
		label_x = label_x + button_w + 10;
		ktm = ktm->next;
	}
}

void tag_render_palette_labels(tag_t * tag) {
	/* char text[1024]; */
	/* a font */
	Imlib_Font font;
	palette_t * pal;
	win_t * win;
	int label_x = 10, label_y = 0;
	int button_w = 0, button_h = 0;
	
	pal = tag->palette;
	win = tag->win;

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

	label_y = win->h - 100;

	while ( pal ) {
		/*imlib_context_set_drawable(win->pm);*/
		fprintf(stderr, "Palette label : %s\n", pal->label);
		/* render_text(pal->label, &button_w, &button_h); */
		render_button(pal->index + 0x30, pal->label, &button_w, &button_h);
		imlib_render_image_part_on_drawable_at_size(0, 0, button_w, button_h, label_x, label_y, button_w, button_h);
		label_x = label_x + button_w + 10;

		if ( tag->current_palette != -1 && tag->current_palette == pal->index) {
			tag_render_palette(tag, pal->index);
		}
		pal = pal->next;
	}
}


void test_draw_text(win_t *win, char * text) {
	/* char text[1024]; */
	/* a font */
	Imlib_Font font;
	/* width and height values */
	int button_w = 0, button_h = 0;

	/*imlib_context_set_drawable(win->pm);*/

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

		/* sprintf(text, "Some Test Text"); */
		render_button('q', text, &button_w, &button_h);
	}
	else {
		fprintf(stderr, "Font not found\n");
	}
	imlib_render_image_part_on_drawable_at_size(0, 0, button_w, button_h, 10, win->h - 50, button_w, button_h);
}

void tag_init(tag_t *tag, palette_t *palette, win_t *win) {
	palette_t * p_counter;

	tag->win = win;
	tag->palette = palette;
	tag->palette_count = 1;
	tag->current_palette = -1;

	p_counter = palette;
	while (p_counter->next != NULL) {
		tag->palette_count++;
		p_counter = p_counter->next;
	}
	TAG_C_LOG && fprintf(stderr, "tag.c: tag count(%d)\n", tag->palette_count);


}

void tag_free(palette_t *palette) {

}

void tag_render(tag_t *tag) {
	win_t *win;
	if (tag == NULL || tag->palette == NULL || tag->win == NULL)
		return;
	win = tag->win;
	win_clear(win);
	imlib_context_set_drawable(win->pm);
	tag_render_palette_labels(tag);
}


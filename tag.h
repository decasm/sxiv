/* sxiv: tag.h
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

#ifndef TAG_H
#define TAG_H

#define TAG_LABEL_NORMAL 0x5A5A5AFF
#define TAG_LABEL_SELECTED 0x000000FF
#define TAG_MAPPING_NORMAL 0x5A5A5AFF
#define TAG_MAPPING_SELECTED 0x000000FF

typedef struct {
	palette_t *palette;
	win_t *win;

	int palette_count;
	int current_palette;
} tag_t;


void tag_init(tag_t *, palette_t *, win_t *);
void tag_render(tag_t *);
void tag_render_palette(tag_t *);
void tag_render_palette_mapping(tag_t *tag);

#endif /* TAG_H */

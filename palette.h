/* sxiv: palette.h
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

#ifndef PALETTE_H
#define PALETTE_H

typedef struct palette_s palette_t;
typedef struct keytagmap_s keytagmap_t;
typedef struct mapping_s mapping_t;
typedef struct mapping2_s mapping2_t;

palette_t * load_palettes(char *);
void display_palette(palette_t *);

struct keytagmap_s {
	char key;
	char * tag;
	keytagmap_t * next;
};

struct palette_s {
	int index;
	char * label;
	palette_t * next;
	keytagmap_t * tags;
};

struct mapping2_s {
	char * key;
	char * value;
	mapping2_t * next;
};

struct mapping_s {
	char * key;
	char * value;
	mapping_t * next;
	mapping_t * down;
};

#endif


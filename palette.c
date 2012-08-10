/* sxiv: palette.c
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


/* http://www.vim.org/scripts/script.php?script_id=1218  */
/* vim nerd commenter */

#include <stdio.h>
#include <string.h>
#include <yaml.h>
#include "palette.h"
#include "util.h"

#define SXITLOG 0
#define SUBMAP "*SUBMAP*"

void handle_scalar_node(yaml_document_t *, yaml_node_t *);
void handle_mapping_node(yaml_document_t *, yaml_node_t *);
void handle_sequence_node(yaml_document_t *, yaml_node_t *);
void handle_node(yaml_document_t *, yaml_node_t *);

char * event_type_to_string(yaml_event_type_t type) {
	switch(type) {
		case YAML_NO_EVENT:
			return "No Event";
			break;
		case YAML_STREAM_START_EVENT:
			return "Stream Start Event";
			break;
		case YAML_STREAM_END_EVENT:
			return "Stream End Event";
			break;
		case YAML_DOCUMENT_START_EVENT:
			return "Document Start Event";
			break;
		case YAML_DOCUMENT_END_EVENT:
			return "Document End Event";
			break;
		case YAML_ALIAS_EVENT:
			return "Alias Event";
			break;
		case YAML_SCALAR_EVENT:
			return "Scalar Event";
			break;
		case YAML_SEQUENCE_START_EVENT:
			return "Sequence Start Event";
			break;
		case YAML_SEQUENCE_END_EVENT:
			return "Sequence End Event";
			break;
		case YAML_MAPPING_START_EVENT:
			return "Mapping Start Event";
			break;
		case YAML_MAPPING_END_EVENT:
			return "Mapping End Event";
			break;
	}
	return "Unknown Event";
}
 
palette_t * new_palette() {
	palette_t * p = malloc(sizeof(palette_t));
	p->index = -1;
	p->label = NULL;
	p->next = NULL;
	p->tags = NULL;
	return p;
}

keytagmap_t * new_keytagmap() {
	keytagmap_t * k = malloc(sizeof(keytagmap_t));
	k->key = 0;
	k->tag = NULL;
	k->next = NULL;
	return k;
}

void display_palette(palette_t * palette) {
	keytagmap_t * ktm;
	while ( palette )  {
		fprintf(stderr, "Palette: label:(%s) index(%d)\n", palette->label, palette->index);
		ktm = palette->tags;
		while ( ktm ) {
			fprintf(stderr, "  %c: %s\n", ktm->key, ktm->tag);
			ktm = ktm->next;
		}
		palette = palette->next;
	}
}



mapping_t * new_mapping() {
	mapping_t * m = malloc(sizeof(mapping_t));
	m->key = NULL;
	m->value = NULL;
	m->next = NULL;
	m->down = NULL;
	return m;
}

void display_map(mapping_t * m) {
	if ( m->key && m->value ) {
		fprintf(stderr, "  %s = %s\n", m->key, m->value);
	}
	else if ( m->key ) {
		fprintf(stderr, "  %s = \n", m->key);
	}
	if ( m->down ) {
		display_map(m->down);
	}
	if ( m->next ) {
		display_map(m->next);
	}
}

/* Experimental parsing. Uses potentially customizable handler routines.
 * More work than is needed at the moment.
 */
int parse_palette_event_map(yaml_parser_t * parser) {
	yaml_event_t event;
	char * scalar;
	mapping_t * root = NULL, * mapping = NULL, * parent = NULL;

	root = new_mapping();

	do {
		yaml_parser_parse(parser, &event);
		switch(event.type) { 
			case YAML_NO_EVENT:
			case YAML_ALIAS_EVENT:
			case YAML_STREAM_START_EVENT:
			case YAML_STREAM_END_EVENT:
			case YAML_DOCUMENT_START_EVENT:
			case YAML_DOCUMENT_END_EVENT:
			case YAML_SEQUENCE_START_EVENT:
			case YAML_SEQUENCE_END_EVENT:
				/* ignore all these events */
				break;
			case YAML_MAPPING_START_EVENT:
				if ( ! mapping ) {
					root->down = mapping = new_mapping();
				}
				else if ( mapping->key && ! mapping->value ) {
					mapping->value = s_strdup(SUBMAP);
					parent = mapping;
					mapping->down = new_mapping();
					mapping = mapping->down;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				mapping = parent;
				break;
			case YAML_SCALAR_EVENT:
				scalar = (char *)event.data.scalar.value;
				if ( mapping ) {
					if ( ! mapping->key ) {
						mapping->key = s_strdup(scalar);
					}
					else if ( ! mapping->value ) { 
						mapping->value = s_strdup(scalar);
					}
					else {
						/* Key and Value set on current mapping */
						mapping->next = new_mapping();
						mapping = mapping->next;
						mapping->key = s_strdup(scalar);
					}
				}
				break;
		}
		if(event.type != YAML_STREAM_END_EVENT) {
			yaml_event_delete(&event);
		}
	} while(event.type != YAML_STREAM_END_EVENT);

	display_map(root);

	return 0;
}

typedef void (*yaml_map_handler)(mapping2_t *);

void handle_map_start(mapping2_t * map) {
	fprintf(stderr, "OK OK\n");
}

void handle_map_end(mapping2_t * map) {
	fprintf(stderr, "OK OK\n");
}

/* Experimental parsing. Uses potentially customizable handler routines.
 * More work than is needed at the moment.
 */
int parse_palette_funcs(yaml_parser_t * parser) {
	yaml_event_t event;
	char * scalar;
	mapping2_t * map = NULL;

	yaml_map_handler map_start = handle_map_start;
	yaml_map_handler map_end = handle_map_end;

	do {
		yaml_parser_parse(parser, &event);
		switch(event.type) { 
			case YAML_NO_EVENT:
			case YAML_ALIAS_EVENT:
			case YAML_STREAM_START_EVENT:
			case YAML_STREAM_END_EVENT:
			case YAML_DOCUMENT_START_EVENT:
			case YAML_DOCUMENT_END_EVENT:
			case YAML_SEQUENCE_START_EVENT:
			case YAML_SEQUENCE_END_EVENT:
				/* ignore all these events */
				break;
			case YAML_MAPPING_START_EVENT:
				(*map_start)(map);
				break;
			case YAML_MAPPING_END_EVENT:
				(*map_end)(map);
				break;
			case YAML_SCALAR_EVENT:
				scalar = (char *)event.data.scalar.value;
				break;
		}
		if(event.type != YAML_STREAM_END_EVENT) {
			yaml_event_delete(&event);
		}
	} while(event.type != YAML_STREAM_END_EVENT);

	return 0;
}

palette_t * parse_palette(yaml_parser_t * parser) {
	yaml_event_t event;
	char * scalar;
	palette_t * palette = NULL, * root_palette = NULL;
	keytagmap_t * ktm =NULL;
	char * longest_tag = NULL;
	int longest_tag_length = 0;

	do {
		yaml_parser_parse(parser, &event);
		switch(event.type) { 
			case YAML_NO_EVENT:
			case YAML_ALIAS_EVENT:
			case YAML_STREAM_START_EVENT:
			case YAML_STREAM_END_EVENT:
			case YAML_DOCUMENT_START_EVENT:
			case YAML_DOCUMENT_END_EVENT:
			case YAML_SEQUENCE_START_EVENT:
			case YAML_SEQUENCE_END_EVENT:
				/* ignore all these events */
				break;
			case YAML_MAPPING_START_EVENT:
				if ( palette && ! palette->tags ) {
					SXITLOG && fprintf(stderr, " start map 1\n");
					palette->tags = ktm = new_keytagmap();
				}
				else {
					SXITLOG && fprintf(stderr, " start map 2\n");
				}
				break;
			case YAML_MAPPING_END_EVENT:
				SXITLOG && fprintf(stderr, " end map 1 p idx(%d) label(%s)\n", palette->index, palette->label);
				ktm = NULL;
				break;
			case YAML_SCALAR_EVENT:
				scalar = (char *)event.data.scalar.value;
				SXITLOG && fprintf(stderr, " scalar evt 1: %s\n", scalar);
				if ( ! ktm ) {
					SXITLOG && fprintf(stderr, " scalar evt 2a: %s\n", scalar);
					if ( ! palette ) {
						palette = root_palette = new_palette();
					}
					else {
						palette->next = new_palette();
						palette = palette->next;
					}

					if ( strlen(scalar) == 1 && palette->index == -1 ) {
						SXITLOG && fprintf(stderr, " palette index: %s\n", scalar);
       					sscanf(scalar, "%d", &(palette->index));
					}
				}
				else {
					SXITLOG && fprintf(stderr, " scalar evt 2b: %s\n", scalar);
					if ( strcmp(scalar, "label") == 0 ) {
						SXITLOG && fprintf(stderr, " label key: %s\n", scalar);
						palette->label = s_strdup(scalar);
					}
					else if ( palette && palette->label && strcmp(palette->label, "label") == 0 ) {
						SXITLOG && fprintf(stderr, " label val: %s\n", scalar);
						palette->label = s_strdup(scalar);
					}
					else if ( ! ktm->key ) {
						SXITLOG && fprintf(stderr, " !k: %s\n", scalar);
						sscanf(scalar, "%c", &(ktm->key));
					}
					else if ( ! ktm->tag ) {
						SXITLOG && fprintf(stderr, " k&!v: %s\n", scalar);
						ktm->tag = s_strdup(scalar);
						if (strlen(ktm->tag) > longest_tag_length) {
							longest_tag = ktm->tag;
							longest_tag_length = strlen(longest_tag);
						}
					}
					else { /* The next key for the current ktm */
						SXITLOG && fprintf(stderr, " !(k|v): %s\n", scalar);
						ktm->next = new_keytagmap();
						ktm = ktm->next;
						sscanf(scalar, "%c", &(ktm->key));
					}
				}
				break;
		}
		if(event.type != YAML_STREAM_END_EVENT) {
			yaml_event_delete(&event);
		}
	} while(event.type != YAML_STREAM_END_EVENT);

	/* display_palette(root_palette); */
	fprintf(stderr, "Longest tag: (%s) length(%d)\n", longest_tag, longest_tag_length);
	return root_palette;
}

palette_t * load_palettes(char * filename) {
	yaml_parser_t parser;
	FILE *input;
	palette_t * palette;

	input = fopen(filename, "rb");

	/* Create the Parser object. */
	yaml_parser_initialize(&parser);

	/* Set a file input. */
	yaml_parser_set_input_file(&parser, input);	

	/*if ( ! parse_palette_funcs(&parser) )*/
	/*if ( ! parse_palette(&parser) )(*/

	palette = parse_palette(&parser);
	if ( NULL == palette )
		goto error;

	/* Destroy the Parser object. */
	yaml_parser_delete(&parser);
	return palette;

	/* On error. */
	error:
		/* Destroy the Parser object. */
		yaml_parser_delete(&parser);
		return NULL;
}




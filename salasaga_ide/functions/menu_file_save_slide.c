/*
 * $Id$
 *
 * Salasaga: Function that takes a slide structure pointer, and adds it's data to an in-memory XML document
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/parser.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_file_save_layer.h"


void menu_file_save_slide(gpointer element, gpointer user_data)
{
	// Local variables
	slide			*slide_pointer;				// Points to the present slide
	GList			*layer_pointer;				// Points to the presently processing layer

	xmlNodePtr		slide_root;					// Points to the root of the slide data
	xmlNodePtr		slide_node;					// Pointer to the new slide node

	GString			*tmp_gstring;				// Temporary GString


	// Initialise various things
	slide_pointer = element;
	slide_root = user_data;
	layer_pointer = slide_pointer->layers;
	tmp_gstring = g_string_new(NULL);

    // Create the slide container
	slide_node = xmlNewChild(slide_root, NULL, (const xmlChar *) "slide", NULL);
	if (NULL == slide_node)
	{
		display_warning("Error ED23: Error creating the slide node\n");
		return;
	}

	// Add the slide name to the slide container attributes
	if (NULL != slide_pointer->name)
	{
		xmlNewProp(slide_node, (const xmlChar *) "name", (const xmlChar *) slide_pointer->name->str);
	}

	// Add the slide duration to the slide container attributes
	g_string_printf(tmp_gstring, "%u", slide_pointer->duration);
	xmlNewProp(slide_node, (const xmlChar *) "duration", (const xmlChar *) tmp_gstring->str);

	// Add the layer data to the slide container
	layer_pointer = g_list_first(layer_pointer);
	g_list_foreach(layer_pointer, menu_file_save_layer, slide_node);

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);

	return;
}

/*
 * $Id$
 *
 * Flame Project: Function that takes a slide structure pointer, and adds it's data to an in-memory XML document
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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


// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "../gui-functions.h"


void menu_file_save_slide(gpointer element, gpointer user_data)
{
	// Local variables
	slide			*slide_pointer;				// Points to the present slide
	GList			*layer_pointer;				// Points to the presently processing layer

	xmlNodePtr		slide_root;					// Points to the root of the slide data
	xmlNodePtr		slide_node;					// Pointer to the new slide node


	// Initialise various things
	slide_pointer = element;
	slide_root = user_data;
	layer_pointer = slide_pointer->layers;

    // Create the slide container
	slide_node = xmlNewChild(slide_root, NULL, (const xmlChar *) "slide", NULL);
	if (NULL == slide_node)
	{
		display_warning("ED23: Error creating the slide node\n");
		return;
	}

	// Add the slide name to the slide container attributes
	if (NULL != slide_pointer->name)
	{
		xmlNewProp(slide_node, (const xmlChar *) "name", (const xmlChar *) slide_pointer->name->str);
	}

	// Add the layer data to the slide container
	layer_pointer = g_list_first(layer_pointer);
	g_list_foreach(layer_pointer, menu_file_save_layer, slide_node);

	return;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:31  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

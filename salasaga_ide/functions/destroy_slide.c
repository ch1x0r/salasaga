/*
 * $Id$
 *
 * Flame Project: Function to free the memory for a given slide
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
#include "display_warning.h"


// fixme5: Had to comment out some of the "free's", as they were causing segfaults.  Thus this function needs to be rewritten properly at some point
void destroy_slide(gpointer element, gpointer user_data)
{
	// Local variables
	gint				layer_counter;			// Standard counter
	layer				*layer_data;			// Used for freeing the elements of a deleted slide
	gint				num_layers;				// Number of layers in a slide
	GList				*removed_layer;			// Used for freeing the elements of a deleted slide
	slide				*slide_data;			// Used for freeing the elements of a deleted slide


	// Initialise some things
	slide_data = element;

	// Free the memory allocated to the deleted slide
	if (NULL != slide_data->name)
	{
		g_string_free(slide_data->name, TRUE);
	}
	gtk_object_destroy(GTK_OBJECT(slide_data->tooltip));
	if (NULL != slide_data->layer_store) g_object_unref(slide_data->layer_store);
	if (NULL != slide_data->timeline_widget) g_object_unref(slide_data->timeline_widget);

	// Free the memory allocated to the deleted slides' layers
	slide_data->layers = g_list_first(slide_data->layers);
	num_layers = g_list_length(slide_data->layers);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Point to the first remaining layer in the list
		slide_data->layers = g_list_first(slide_data->layers);
		layer_data = (slide_data->layers)->data;

		// Free its elements
		g_string_free(layer_data->name, TRUE);
		// This is causing a Segfault
		// g_free(layer_data->row_iter);
		switch (layer_data->object_type)
		{
			case TYPE_GDK_PIXBUF:
				// This is causing a Segfault
				// g_string_free(((layer_image *) layer_data->object_data)->image_path, TRUE);
				if (NULL != ((layer_image *) layer_data->object_data)->image_data) g_object_unref(((layer_image *) layer_data->object_data)->image_data);
				break;

			case TYPE_MOUSE_CURSOR:
				// Nothing here needs freeing
				break;

			case TYPE_EMPTY:
				// Nothing here needs freeing
				break;

			case TYPE_TEXT:
				if (NULL != ((layer_text *) layer_data->object_data)->text_buffer) g_object_unref(((layer_text *) layer_data->object_data)->text_buffer);
				break;

			case TYPE_HIGHLIGHT:
				// Nothing here needs freeing
				break;

			default:
				display_warning("Error ED57: Unknown layer type when destroying a slide.\n");
		}
		g_free(layer_data->object_data);

		// Remove the layer from the list
		removed_layer = slide_data->layers;
		slide_data->layers = g_list_remove_link(slide_data->layers, slide_data->layers);
		g_list_free(removed_layer);
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/01/19 06:34:59  vapour
 * Tweaked an error message for clarity.
 *
 * Revision 1.5  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.4  2007/10/04 19:51:09  vapour
 * Began adjusting the includes in each function to only call the ones it needs, in order to dramatically speed up recompilation time for developers.
 *
 * Revision 1.3  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:53  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

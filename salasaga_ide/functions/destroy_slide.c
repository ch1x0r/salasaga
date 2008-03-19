/*
 * $Id$
 *
 * Salasaga: Function to free the memory for a given slide
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
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

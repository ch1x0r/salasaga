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
#include "layer_free.h"


void destroy_slide(gpointer element, gpointer user_data)
{
	// Local variables
	gint				layer_counter;			// Standard counter
	layer				*layer_data;			// Used for freeing the elements of a deleted slide
	gint				num_layers;				// Number of layers in a slide
	slide				*slide_data;			// Used for freeing the elements of a deleted slide


	// Initialise some things
	slide_data = element;

	// Free the memory allocated to the deleted slide
	if (NULL != slide_data->thumbnail)
		g_object_unref(slide_data->thumbnail);
	if (NULL != slide_data->name)
		g_string_free(slide_data->name, TRUE);
	if (NULL != slide_data->tooltip)
		g_object_unref(G_OBJECT(slide_data->tooltip));
	if (NULL != slide_data->layer_store)
		g_object_unref(slide_data->layer_store);
	if (NULL != slide_data->timeline_widget)
		g_object_unref(slide_data->timeline_widget);

	// Free the memory allocated to the deleted slides' layers
	slide_data->layers = g_list_first(slide_data->layers);
	num_layers = g_list_length(slide_data->layers);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Loop through the layers, freeing the memory allocated to them
		slide_data->layers = g_list_first(slide_data->layers);
		layer_data = g_list_nth_data(slide_data->layers, layer_counter);
		layer_free(layer_data);
	}

	// Free the memory allocated to the layer list itself
	g_list_free(slide_data->layers);

	// Free the memory allocated to the slide structure itself
	g_free(slide_data);
}

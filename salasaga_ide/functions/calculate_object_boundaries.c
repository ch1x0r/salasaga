/*
 * $Id$
 *
 * Salasaga: Function to calculate collision detection boundaries
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


void calculate_object_boundaries(void)
{
	// Local variables
	boundary_box		*boundary;				// Boundary information
	guint				count_int;				// Counter
	GList				*layer_pointer;
	guint				num_layers;				// The number of layers in the slide
	layer_highlight		*this_highlight;		// Pointer to the highlight layer data we're working on
	layer_image			*this_image;			// Pointer to the image layer data we're working on
	layer_mouse			*this_mouse;			// Pointer to the mouse layer data we're working on
	layer_text			*this_text;				// Pointer to the text layer data we're working on
	layer				*this_layer;			// Pointer to the layer we're working on

	GdkRectangle		tmp_rectangle;			//


	// Only continue in this function if we have a slide structure available
	if (NULL == current_slide)
	{
		return;
	}

	// Work out how many layers we need to iterate through
	layer_pointer = g_list_first(((slide *) current_slide->data)->layers);
	num_layers = g_list_length(layer_pointer);

	// (Re-)Initialise the boundary list
	if (NULL != boundary_list)
	{
		boundary_list = g_list_first(boundary_list);
		g_list_foreach(boundary_list, (GFunc) g_free, NULL);  // I *think* this will free the memory allocated for the GdkRegions.
		g_list_free(boundary_list);
		boundary_list = NULL;
	}

	for (count_int = 0; count_int <= num_layers - 1; count_int++)
	{
		// Select the desired layer
		layer_pointer = g_list_first(layer_pointer);
		layer_pointer = g_list_nth(layer_pointer, count_int);
		this_layer = (layer *) layer_pointer->data;

		// Determine the layer type then calculate its boundaries accordingly
		switch (this_layer->object_type)
		{
			case TYPE_EMPTY:

				// No boundaries to calculate
				tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
				break;

			case TYPE_GDK_PIXBUF:

				// If we're processing the background layer, we skip it
				if (TRUE == this_layer->background)
				{
					tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
					break;
				}

				// Simplify the pointers
				this_image = (layer_image *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (this_layer->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (this_image->width * zoom) / 100;
				tmp_rectangle.y = (this_layer->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (this_image->height * zoom) / 97;
				break;

			case TYPE_HIGHLIGHT:

				// Simplify the pointers
				this_highlight = (layer_highlight *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (this_layer->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (this_highlight->width * zoom) / 100;
				tmp_rectangle.y = (this_layer->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (this_highlight->height * zoom) / 97;
				break;

			case TYPE_MOUSE_CURSOR:

				// Simplify the pointers
				this_mouse = (layer_mouse *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (this_layer->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (this_mouse->width * zoom) / 100;
				tmp_rectangle.y = (this_layer->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (this_mouse->height * zoom) / 97;
				break;

			case TYPE_TEXT:

				// Simplify the pointers
				this_text = (layer_text *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (this_layer->x_offset_start * zoom) / 102;
				tmp_rectangle.width = (this_text->rendered_width * zoom) / 101;
				tmp_rectangle.y = (this_layer->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (this_text->rendered_height * zoom) / 97;
				break;

			default:
				display_warning("Error ED27: Unknown layer type\n");
		}

		// * Store the calculated boundary *

		// Skip the background layer
		if (0 != tmp_rectangle.width)
		{
			// Allocate memory for a new GdkRegion, then add it to the boundary list
			boundary = g_new(boundary_box, 1);
			boundary->region_ptr = gdk_region_rectangle(&tmp_rectangle);
			boundary->layer_ptr = layer_pointer;
			boundary_list = g_list_first(boundary_list);
			boundary_list = g_list_append(boundary_list, boundary);
		}
	}
}

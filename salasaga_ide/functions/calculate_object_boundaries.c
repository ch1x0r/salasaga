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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

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
	boundary_box		*boundary;					// Boundary information
	guint				count_int;					// Counter used to loop through the layers
	GList				*layer_pointer;
	guint				num_layers;					// The number of layers in the slide
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	layer_highlight		*this_highlight;			// Pointer to the highlight layer data we're working on
	layer_image			*this_image;				// Pointer to the image layer data we're working on
	layer_mouse			*this_mouse;				// Pointer to the mouse layer data we're working on
	layer_text			*this_text;					// Pointer to the text layer data we're working on
	layer				*this_layer;				// Pointer to the layer we're working on
	GdkRectangle		tmp_rectangle;				//


	// Only continue in this function if we have a slide structure available
	if (NULL == current_slide)
	{
		return;
	}

	// Calculate the height and width scaling values for the main drawing area at its present size
	scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

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

	for (count_int = 0; count_int < num_layers; count_int++)
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
				tmp_rectangle.x = roundf(this_layer->x_offset_start / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_image->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(this_layer->y_offset_start / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_image->height / scaled_height_ratio);
				break;

			case TYPE_HIGHLIGHT:

				// Simplify the pointers
				this_highlight = (layer_highlight *) this_layer->object_data;

				// Work out the working area covered by this object
				tmp_rectangle.x = roundf(this_layer->x_offset_start / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_highlight->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(this_layer->y_offset_start / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_highlight->height / scaled_height_ratio);
				break;

			case TYPE_MOUSE_CURSOR:

				// Simplify the pointers
				this_mouse = (layer_mouse *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = roundf(this_layer->x_offset_start / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_mouse->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(this_layer->y_offset_start / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_mouse->height / scaled_height_ratio);
				break;

			case TYPE_TEXT:

				// Simplify the pointers
				this_text = (layer_text *) this_layer->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = roundf(this_layer->x_offset_start / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_text->rendered_width / scaled_width_ratio);
				tmp_rectangle.y = roundf(this_layer->y_offset_start / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_text->rendered_height / scaled_height_ratio);
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

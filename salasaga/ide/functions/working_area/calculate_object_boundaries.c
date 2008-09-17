/*
 * $Id$
 *
 * Salasaga: Function to calculate collision detection boundaries
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 * 
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
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
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../layer/get_layer_position.h"
#include "../widgets/time_line.h"


void calculate_object_boundaries(void)
{
	// Local variables
	boundary_box		*boundary;					// Boundary information
	guint				count_int;					// Counter used to loop through the layers
	GList				*layer_pointer;
	GtkAllocation		layer_positions;			// Offset and dimensions for a given layer object
	GString				*message;					// Used to construct message strings
	guint				num_layers;					// The number of layers in the slide
	gint				pixmap_height;				// Height of the front stoe
	gint				pixmap_width;				// Width of the front store
	gboolean			return_code_gbool;			// Receives gboolean return codes
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	layer_highlight		*this_highlight;			// Pointer to the highlight layer data we're working on
	layer_image			*this_image;				// Pointer to the image layer data we're working on
	layer				*this_layer_data;			// Pointer to the layer we're working on
	layer_mouse			*this_mouse;				// Pointer to the mouse layer data we're working on
	layer_text			*this_text;					// Pointer to the text layer data we're working on
	slide				*this_slide_data;			// Pointer to the data for the slide we're working on
	gfloat				time_alpha;
	gfloat 				time_position;				// The point in time we need the handle box for
	GdkRectangle		tmp_rectangle;				//


	// Only continue in this function if we have a slide structure available
	if (NULL == current_slide)
	{
		return;
	}

	// Initialise some things
	this_slide_data = current_slide->data;

	// Calculate the height and width scaling values for the main drawing area at its present size
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) project_height / (gfloat) pixmap_height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) pixmap_width;

	// Work out how many layers we need to iterate through
	layer_pointer = g_list_first(this_slide_data->layers);
	num_layers = this_slide_data->num_layers;

	// Get the current time line cursor position
	time_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

	// (Re-)Initialise the boundary list
	if (NULL != boundary_list)
	{
		boundary_list = g_list_first(boundary_list);
		g_list_foreach(boundary_list, (GFunc) g_free, NULL);
		g_list_free(boundary_list);
		boundary_list = NULL;
	}

	for (count_int = 0; count_int < num_layers; count_int++)
	{
		// Select the desired layer
		layer_pointer = g_list_first(layer_pointer);
		layer_pointer = g_list_nth(layer_pointer, count_int);
		this_layer_data = layer_pointer->data;

		// Retrieve the layer position and alpha for the given point in time
		return_code_gbool = get_layer_position(&layer_positions, this_layer_data, time_position, &time_alpha);
		if (FALSE == return_code_gbool)
			return;

		// Determine the layer type then calculate its boundaries accordingly
		switch (this_layer_data->object_type)
		{
			case TYPE_EMPTY:

				// No boundaries to calculate
				tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
				break;

			case TYPE_GDK_PIXBUF:

				// If we're processing the background layer, we skip it
				if (TRUE == this_layer_data->background)
				{
					tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
					break;
				}

				// Simplify the pointers
				this_image = (layer_image *) this_layer_data->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = roundf(layer_positions.x / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_image->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(layer_positions.y / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_image->height / scaled_height_ratio);
				break;

			case TYPE_HIGHLIGHT:

				// Simplify the pointers
				this_highlight = (layer_highlight *) this_layer_data->object_data;

				// Work out the working area covered by this object
				tmp_rectangle.x = roundf(layer_positions.x / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_highlight->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(layer_positions.y / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_highlight->height / scaled_height_ratio);
				break;

			case TYPE_MOUSE_CURSOR:

				// Simplify the pointers
				this_mouse = (layer_mouse *) this_layer_data->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = roundf(layer_positions.x / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_mouse->width / scaled_width_ratio);
				tmp_rectangle.y = roundf(layer_positions.y / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_mouse->height / scaled_height_ratio);
				break;

			case TYPE_TEXT:

				// Simplify the pointers
				this_text = (layer_text *) this_layer_data->object_data;

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = roundf(layer_positions.x / scaled_width_ratio);
				tmp_rectangle.width = roundf(this_text->rendered_width / scaled_width_ratio);
				tmp_rectangle.y = roundf(layer_positions.y / scaled_height_ratio);
				tmp_rectangle.height = roundf(this_text->rendered_height / scaled_height_ratio);
				break;

			default:
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED27: %s", _("Error"), _("Unknown layer type."));
				display_warning(message->str);
				g_string_free(message, TRUE);
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

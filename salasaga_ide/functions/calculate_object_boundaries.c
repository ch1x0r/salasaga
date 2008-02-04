/*
 * $Id$
 *
 * Flame Project: Function to calculate collision detection boundaries
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

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


void calculate_object_boundaries(void)
{
	// Local variables
	GList				*layer_pointer;
	guint				num_layers;				// The number of layers in the slide
	guint				count_int;				// Counter

	boundary_box		*boundary;				// Boundary information

	guint				tmp_int;				//
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

		// Determine the layer type then calculate its boundaries accordingly
		switch (((layer *) layer_pointer->data)->object_type)
		{
			case TYPE_EMPTY:
				// No boundaries to calculate
				tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
				break;

			case TYPE_GDK_PIXBUF:
				// If we're processing the background layer, we skip it
				tmp_int = g_ascii_strncasecmp(((layer *) layer_pointer->data)->name->str, "Background", 10);
				if (0 == tmp_int)
				{
					tmp_rectangle.width = 0;  // Use this as a flag to indicate we're skipping this layer
					break;
				}

				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_image *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_HIGHLIGHT:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_highlight *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_MOUSE_CURSOR:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 100;
				tmp_rectangle.width = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->width * zoom) / 100;
				tmp_rectangle.y = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_mouse *) ((layer *) layer_pointer->data)->object_data)->height * zoom) / 97;
				break;

			case TYPE_TEXT:
				// Translate the area covered by the layer object, with the zoom factor
				tmp_rectangle.x = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->x_offset_start * zoom) / 102;
				tmp_rectangle.width = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_width * zoom) / 101;
				tmp_rectangle.y = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->y_offset_start * zoom) / 98;
				tmp_rectangle.height = (((layer_text *) ((layer *) layer_pointer->data)->object_data)->rendered_height * zoom) / 97;
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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.7  2008/02/04 14:55:33  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.6  2008/01/19 06:33:25  vapour
 * Tweaked an error message for clarity.
 *
 * Revision 1.5  2008/01/15 16:19:04  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.4  2007/10/06 11:40:34  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:05  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

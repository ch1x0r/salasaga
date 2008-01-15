/*
 * $Id$
 *
 * Flame Project: Function to regenerate the timeline duration images for an individual slide
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
#include "create_timeline_slider.h"


void regenerate_timeline_duration_images(slide *target_slide)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	gfloat				finish_pixel;				// Ending slider pixel to fill in
	guint				layer_counter;				// Counter used when processing layers
	layer				*layer_data;				// Pointer to the layer data we're working on
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	guint				num_layers;				// Number of layers in a slide (used for a loop)
	gfloat				pixel_width;				// Width of pixels to fill
	guint				start_frame;				// Used when working out a layer's start frame
	gfloat				start_pixel;				// Starting slider pixel to fill in


	// Create the duration slider images for the timeline area
	num_layers = g_list_length(target_slide->layers);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Work out the start and ending frames for this layer
		layer_data = g_list_nth_data(target_slide->layers, layer_counter);
		start_frame = layer_data->start_frame;
		finish_frame = layer_data->finish_frame;

		// Calculate the duration of the layer for drawing inside the slider
		start_pixel = 180 * ((gfloat) start_frame / (gfloat) target_slide->duration);
		finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) target_slide->duration);
		pixel_width = finish_pixel - start_pixel;

		// Create duration image
		layer_pixbuf = NULL;
		layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);

		// Update the timeline with the duration image
		gtk_list_store_set(target_slide->layer_store, layer_data->row_iter,
					TIMELINE_DURATION, layer_pixbuf,
					-1);
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.5  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.4  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:35  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

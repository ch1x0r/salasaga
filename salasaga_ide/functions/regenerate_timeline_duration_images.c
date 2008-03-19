/*
 * $Id$
 *
 * Salasaga: Function to regenerate the timeline duration images for an individual slide
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
#include "create_timeline_slider.h"


void regenerate_timeline_duration_images(slide *target_slide)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	gfloat				finish_pixel;				// Ending slider pixel to fill in
	guint				layer_counter;				// Counter used when processing layers
	layer				*layer_data;				// Pointer to the layer data we're working on
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	GList				*layer_ptr;					// Pointer to the layer Glist entry
	guint				num_layers;					// Number of layers in a slide (used for a loop)
	gfloat				pixel_width;				// Width of pixels to fill
	guint				start_frame;				// Used when working out a layer's start frame
	gfloat				start_pixel;				// Starting slider pixel to fill in


	// Create the duration slider images for the timeline area
	layer_ptr = g_list_first(target_slide->layers);
	num_layers = g_list_length(layer_ptr);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Work out the start and ending frames for this layer
		layer_data = g_list_nth_data(layer_ptr, layer_counter);
		start_frame = layer_data->start_frame;
		finish_frame = layer_data->finish_frame;

		// Calculate the duration of the layer for drawing inside the slider
		start_pixel = 180 * ((gfloat) start_frame / (gfloat) target_slide->duration);
		finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) target_slide->duration);
		pixel_width = finish_pixel - start_pixel;

		// Create duration image
		layer_pixbuf = NULL;
		layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);

		// fixme4: We should probably free the memory of the old timeline duration image here

		// Update the timeline with the duration image
		gtk_list_store_set(target_slide->layer_store, layer_data->row_iter,
					TIMELINE_DURATION, layer_pixbuf,
					-1);
	}
}

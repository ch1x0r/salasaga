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


// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Standard includes
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
#include "create_timeline_slider.h"


void regenerate_timeline_duration_images(slide *target_slide)
{
	// Local variables
	guint				duration_height = 20;		// Total height of the duration slider image in pixels
	guint				duration_width = 180;		// Total width of the duration slider image in pixels
	gfloat				finish_frame;				// Used when working out a layer's finish frame
	guint				finish_pixel;				// Ending slider pixel to fill in
	guint				layer_counter;				// Counter used when processing layers
	layer				*layer_data;				// Pointer to the layer data we're working on
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	GList				*layer_ptr;					// Pointer to the layer Glist entry
	guint				num_layers;					// Number of layers in a slide (used for a loop)
	gfloat				pixels_per_frame;			// Number of pixels is frame takes
	gfloat				start_frame;				// Frame number where the transition in begins
	guint				start_pixel;				// Starting slider pixel to fill in
	gfloat				trans_in_finish;			// Frame number where the transition in finishes
	guint				trans_in_finish_pixel;		// Pixel where the transition out finishes
	gfloat				trans_out_start;			// Frame number where the transition out starts
	guint				trans_out_start_pixel;		// Pixel where the transition out starts


	// Slide 
	pixels_per_frame = duration_width / target_slide->duration;

	// Create the duration slider images for the timeline area
	layer_ptr = g_list_first(target_slide->layers);
	num_layers = g_list_length(layer_ptr);
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		// Work out the start and ending frames for this layer
		layer_data = g_list_nth_data(layer_ptr, layer_counter);
		start_frame = layer_data->start_time * frames_per_second;

		// If there's a transition in, work things out
		if (TRANS_LAYER_NONE != layer_data->transition_in_type)
		{
			trans_in_finish = start_frame + (layer_data->transition_in_duration * frames_per_second);
		} else
		{
			trans_in_finish = start_frame;
		}

		// Work out the main layer display time 
		trans_out_start = trans_in_finish + (layer_data->duration * frames_per_second);

		// Work out the frame where the layer finishes displaying
		if (TRANS_LAYER_NONE != layer_data->transition_out_type)
		{
			finish_frame = trans_out_start + (layer_data->transition_out_duration * frames_per_second);
		} else
		{
			finish_frame = trans_out_start;
		}

		// Calculate the duration of the layer for drawing inside the slider
		start_pixel = roundf(start_frame * pixels_per_frame);
		trans_in_finish_pixel = roundf(trans_in_finish * pixels_per_frame);
		trans_out_start_pixel = roundf(trans_out_start  * pixels_per_frame);
		finish_pixel = roundf(finish_frame * pixels_per_frame);

		// Create duration image
		layer_pixbuf = NULL;
		layer_pixbuf = create_timeline_slider(GDK_PIXBUF(layer_pixbuf), duration_width, duration_height, start_pixel, trans_in_finish_pixel, trans_out_start_pixel, finish_pixel);

		// fixme4: We should probably free the memory of the old timeline duration image here

		// Update the timeline with the duration image
		gtk_list_store_set(target_slide->layer_store, layer_data->row_iter,
					TIMELINE_DURATION, layer_pixbuf,
					-1);
	}
}

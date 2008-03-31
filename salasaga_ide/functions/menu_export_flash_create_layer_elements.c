/*
 * $Id$
 *
 * Salasaga: Prepares the swf frame elements for a given layer, so it can be converted to swf 
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"

gboolean menu_export_flash_create_layer_elements(swf_frame_element *array_start, guint num_frames, layer *this_layer_data, guint layer_depth)
{
	// Local variables
	guint				element_number;
	gfloat				element_x_position_finish = 0;
	gfloat				element_x_position_increment = 0;
	gfloat				element_x_position_start = 0;
	gfloat				element_y_position_finish = 0;
	gfloat				element_y_position_increment = 0;
	gfloat				element_y_position_start = 0;
	guint				finish_frame;
	guint				frame_number;
	guint				frame_counter;				// Holds the number of frames
	guint				layer_counter;				// Holds the number of layers
	guint				num_displayed_frames;
	guint				num_layers = 0;				// The number of layers in the slide
	guint				num_slides;					// The number of slides in the movie
	guint				opacity_count;				// Used when calculating object opacity
	gfloat				opacity_step;				// Used when calculating object opacity
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	guint				slide_counter;				// Holds the number of slides
	guint				start_frame;
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	guint				x_position;					// Used in calculating layer object position
	guint				y_position;					// Used in calculating layer object position


	// Calculate the scaled start and finish positions for each element
	element_x_position_start = scaled_width_ratio * this_layer_data->x_offset_start;
	element_x_position_finish = scaled_width_ratio * this_layer_data->x_offset_finish;
	element_y_position_start = scaled_height_ratio * this_layer_data->y_offset_start;
	element_y_position_finish = scaled_height_ratio * this_layer_data->y_offset_finish;

	// If there is a fade in transition, fill in the relevant elements
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
	{
		// Work out the starting and ending frames for the fade
		start_frame = this_layer_data->start_time * frames_per_second;
		finish_frame = (this_layer_data->transition_in_duration + this_layer_data->start_time) * frames_per_second;

		// Indicate on which frame the element should be displayed, at what display depth, and its starting co-ordinates
		frame_number = (layer_counter * (slide_duration + 1)) + start_frame;
		array_start[frame_number].add = TRUE;
		array_start[frame_number].depth = layer_depth;
		array_start[frame_number].x_position = element_x_position_start;
		array_start[frame_number].y_position = element_y_position_start;
		array_start[frame_number].action_this = TRUE;
		array_start[frame_number].object_name = g_string_new(NULL);
		g_string_printf(array_start[frame_number].object_name, "Object%d", layer_depth);

		// Work out how much opacity to increment each frame by
		opacity_step = 100 / (this_layer_data->transition_in_duration * frames_per_second);
		opacity_count = 0;

		// Loop through each frame of the fade in, setting the opacity values
		for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
		{
			// Point to the desired element 
			element_number = (layer_counter * (slide_duration + 1)) + frame_counter;
			this_frame_ptr = &array_start[element_number];

			// Mark this element as needing action taken
			this_frame_ptr->action_this = TRUE;
			this_frame_ptr->opacity_change = TRUE;

			// Ensure the appropriate layer data can be found by later code
			this_frame_ptr->layer_info = this_layer_data;

			// Point to this object's display list item object name
			this_frame_ptr->object_name = array_start[frame_number].object_name;

			// Store the opacity value for this layer on this frame
			this_frame_ptr->opacity = opacity_count;
			opacity_count += floorf(opacity_step);
		}
	} else
	{
		// Indicate on which frame the element should be displayed, at what display depth, and its starting co-ordinates
		frame_number = (layer_counter * (slide_duration + 1)) + (this_layer_data->start_time * frames_per_second);
		array_start[frame_number].add = TRUE;
		array_start[frame_number].depth = layer_depth;
		array_start[frame_number].x_position = element_x_position_start;
		array_start[frame_number].y_position = element_y_position_start;
		array_start[frame_number].action_this = TRUE;
		array_start[frame_number].object_name = g_string_new(NULL);
		g_string_printf(array_start[frame_number].object_name, "Object%d", layer_depth);
	}

	// If there is a fade out transition, fill in the relevant elements
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
	{
		// Work out the starting and ending frames for the fade
		start_frame = this_layer_data->start_time * frames_per_second;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			start_frame += this_layer_data->transition_in_duration * frames_per_second;
		start_frame += this_layer_data->duration * frames_per_second;
		finish_frame = (this_layer_data->transition_out_duration * frames_per_second) + start_frame;

		// Work out how much opacity to decrement each frame by
		opacity_step = 100 / (this_layer_data->transition_out_duration * frames_per_second);
		opacity_count = 100;

		// Loop through each frame of the fade out, setting the opacity values
		for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
		{
			// Point to the desired element 
			element_number = (layer_counter * (slide_duration + 1)) + frame_counter;
			this_frame_ptr = &array_start[element_number];

			// Mark this element as needing action taken
			this_frame_ptr->action_this = TRUE;
			this_frame_ptr->opacity_change = TRUE;

			// Ensure the appropriate layer data can be found by later code
			this_frame_ptr->layer_info = this_layer_data;

			// Point to this object's display list item object name
			this_frame_ptr->object_name = array_start[frame_number].object_name;

			// Store the opacity value for this layer on this frame
			this_frame_ptr->opacity = opacity_count;
			opacity_count -= floorf(opacity_step);
		}
	}

	// Work out the starting and ending frames for the fully visible layer display
	start_frame = this_layer_data->start_time * frames_per_second;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		start_frame += (this_layer_data->transition_in_duration * frames_per_second);
	finish_frame = start_frame + (this_layer_data->duration * frames_per_second);
	num_displayed_frames = (finish_frame - start_frame) + 1;

	x_position = element_x_position_start;
	y_position = element_y_position_start;

	// If the layer moves, work out the movement related values
	if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
	{
		// Work out how much to increment the frame movement by in each direction
		element_x_position_increment = (element_x_position_finish - element_x_position_start) / (num_displayed_frames - 1);
		element_y_position_increment = (element_y_position_finish - element_y_position_start) / (num_displayed_frames - 1);
	}

	// Loop through each frame of the fully visible layer, filling in the relevant elements
	for (frame_number = start_frame; frame_number <= finish_frame; frame_number++)
	{
		// Point to the desired element 
		element_number = (layer_counter * (slide_duration + 1)) + frame_number;
		this_frame_ptr = &array_start[element_number];

		// Ensure the appropriate layer data can be found by later code
		this_frame_ptr->layer_info = this_layer_data;

		// Store the x and y positions for this layer for this frame
		this_frame_ptr->x_position = x_position;
		this_frame_ptr->y_position = y_position;

		// If the layer moves, fill in the relevant elements
		if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
		{
			// Mark this element as needing action taken
			this_frame_ptr->action_this = TRUE;
			this_frame_ptr->is_moving = TRUE;

			// Update the element position with each loop
			x_position += element_x_position_increment;
			y_position += element_y_position_increment;
		}

		// Store the full opacity setting for each frame
		this_frame_ptr->opacity = 100;
	}

	// Determine on which frame the element should be removed from display
	frame_number = finish_frame;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			frame_number += this_layer_data->transition_out_duration * frames_per_second;

	// If this is the background layer for the very last slide, we don't remove it
	if ((layer_counter == (num_layers - 1)) && (slide_counter == (num_slides - 1)) && (TRUE == this_layer_data->background))
	{
		array_start[frame_number].remove = FALSE; 
	} else
	{
		array_start[frame_number].remove = TRUE;
	}

	return TRUE;
}

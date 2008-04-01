/*
 * $Id$
 *
 * Salasaga: Function to convert a slide structure into a Flash output file
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

// Math include
#include <math.h>

// GTK include
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_export_flash_choose_resolution_index.h"
#include "menu_export_flash_control_bar.h"
#include "menu_export_flash_create_layer_elements.h"
#include "menu_export_flash_create_shape.h"
#include "menu_export_flash_process_element.h"


gint menu_export_flash_inner(gchar *output_filename)
{
	// Local variables
	GString				*as_gstring;				// Used for constructing action script statements
	gboolean			dictionary_shape_ok;		// Temporary value indicating if a dictionary shape was created ok or not
	guint				display_depth;				// Depth at which to display a layer
	guint				element_index;				// Points to the start of a layers elements in the swf frame array
	SWFAction			end_action;					// The actionscript for the end behaviour
	guint				frame_counter;				// Holds the number of frames
	guint				frame_number;
	SWFAction			initial_action;				// Swf action object used to run some action script
	GString				*initial_action_gstring;	// Swf action script can be constructed with this
	SWFAction			inc_slide_counter_action;	// Swf action object used to run some action script
	guint				layer_counter;				// Holds the number of layers
	gint				num_bytes_written;			// Receives the number of bytes written to disk for the swf output
	guint				num_layers = 0;				// The number of layers in the slide
	guint				num_slides;					// The number of slides in the movie
	guint				out_res_index;				// Index into the array of output resolution entries 
	gboolean			return_code_bool;			// Receives boolean return codes
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	guint				slide_counter;				// Holds the number of slides
	guint				slide_depth;				// Used in calculating the depth of layers in a slide
	guint				slide_duration;				// Holds the total number of frames in this slide
	GString				*slide_name_tmp;			// Temporary slide names are constructed with this
	SWFMovie			swf_movie;					// Swf movie object
	swf_frame_element	*swf_timing_array = NULL;	// Used to coordinate the actions in each frame
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	layer				*this_layer_data;			// Points to the data in the present layer
	slide				*this_slide_data;			// Points to the data in the present slide
	guint				total_frames;				// The total number of frames in the animation
	guint				total_num_layers;			// The total number of layers in the animation
	gfloat				total_seconds;				// The duration of the entire animation in seconds	


	// Initialise variables
	as_gstring = g_string_new(NULL);
	slide_name_tmp = g_string_new(NULL);
	slide_depth = 2;
	total_frames = 0;
	total_seconds = 0;
	initial_action_gstring = g_string_new(NULL);

	// Determine which of the control bar resolutions to use
	out_res_index = menu_export_flash_choose_resolution_index();

	// If an unknown output resolution is given, indicate an error and return
	if (-1 == out_res_index)
	{
		display_warning("Error ED201: Unknown output resolution selected for swf export.");
		return FALSE;
	}

	// Create an empty swf movie object
	swf_movie = newSWFMovieWithVersion(7);
	Ming_setSWFCompression(9);

	// Set the output size of the swf movie 
	SWFMovie_setDimension(swf_movie, output_width, output_height);

	// Set the frame rate for the movie
	SWFMovie_setRate(swf_movie, frames_per_second);

	// Set the background color for the animation
	SWFMovie_setBackground(swf_movie, 0x00, 0x00, 0x00);  // RGB value - black

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) output_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) output_width / (gfloat) project_width;

	// Displaying debugging info if requested
	if (debug_level)
	{
		printf("Scaled height ratio: %.2f\n", scaled_height_ratio);
		printf("Scaled width ratio: %.2f\n", scaled_width_ratio);
	}

	// Count the number of slides in the movie
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("Number of slides: %u\n", num_slides);
	}

	// Count the total number of layers in the movie
	total_num_layers = 2;
	for (slide_counter = 0; slide_counter <  num_slides; slide_counter++)
	{
		// Point to the selected slide
		slides = g_list_first(slides);
		this_slide_data = g_list_nth_data(slides, slide_counter);

		// Add the number of layers in this slide to the total count
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		num_layers = g_list_length(this_slide_data->layers);
		total_num_layers += num_layers;
	}

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("Total number of layers: %u\n", total_num_layers);
	}

	// If requested, add the swf control bar to the movie
	if (TRUE == show_control_bar)
	{
		return_code_bool = menu_export_flash_control_bar(swf_movie, out_res_index, total_num_layers + 5);
		if (TRUE != return_code_bool)
		{
			// Something went wrong when adding the control bar to the movie
			destroySWFMovie(swf_movie);
			return FALSE;
		}
	} else
	{
		// Ensure the swf output starts out in the correct play state
		if (START_BEHAVIOUR_PLAY == start_behaviour)
		{
			g_string_printf(initial_action_gstring, " var playing = true;");
		} else
		{
			g_string_printf(initial_action_gstring, " var playing = false;");
		}

		// Add the action script to keep the animation paused if that's whats requested
		initial_action_gstring = g_string_append(initial_action_gstring, " if (false == _root.playing) { _root.stop(); };");

		// Add the initialisation action to the movie
		initial_action = newSWFAction(initial_action_gstring->str);
		SWFMovie_add(swf_movie, (SWFBlock) initial_action);
	}

// For now, this uses a two pass per slide approach
//	1st pass gathers timing info, and creates an array of timing information for each slide
//		The array has one element for each frame, times each layer of the slide (i.e. num elements = num frames x num layers)
//		Also creates the dictionary of shapes, and adds them to an output buffer
//	2nd pass writes out the actions to happen for the slide, relying on the dictionary images created in the first pass
// (sounds like a reasonable approach (theory) for a first go, lets see it works in reality though)

	// For each slide, work out how many layers there are and how many frames the entire slide lasts for
	for (slide_counter = 0; slide_counter <  num_slides; slide_counter++)
	{
		// Initialise things for this slide
		slides = g_list_first(slides);
		this_slide_data = g_list_nth_data(slides, slide_counter);
		slide_duration = this_slide_data->duration * frames_per_second;

		// Add the duration in seconds to the total duration count for the animation
		total_seconds += this_slide_data->duration;

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += slide_duration;

		// Add the slide name to the output swf
		if (NULL == this_slide_data->name)
		{
			// The slide doesn't have a name, so we create a temporary one
			g_string_printf(slide_name_tmp, "Slide%u", slide_counter);
			SWFMovie_labelFrame(swf_movie, slide_name_tmp->str);
		} else
		{
			SWFMovie_labelFrame(swf_movie, this_slide_data->name->str);
		}

		// Work out how many layers there are in this slide
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		num_layers = g_list_length(this_slide_data->layers);

		// * At this stage we should know both the number of frames (slide_duration) and number of layers (num_layers) in the slide *

		// Output some debugging info if requested
		if (debug_level)
		{
			printf("Number of layers in slide %u is %u\n", slide_counter, num_layers);
			printf("Maximum frame number in slide %u is %u\n", slide_counter, slide_duration);
		}

		// Create an array that's layers x "number of frames in the slide"
		frame_number = num_layers * slide_duration;
		swf_timing_array = g_try_new0(swf_frame_element, frame_number);
		if (NULL == swf_timing_array)
		{
			display_warning("Error ED339: We couldn't allocate enough memory to generate the swf.  Export aborted.");
			return FALSE;
		}

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		slide_depth += num_layers;
		display_depth = slide_depth;
		this_slide_data = g_list_nth_data(slides, slide_counter);
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// If this layer is not visible, then we skip it
			if (FALSE == this_layer_data->visible)
				continue;

			// Create the dictionary shape for the layer
			dictionary_shape_ok = menu_export_flash_create_shape(this_layer_data);

			// If the creation of the dictionary shape worked, we add this layer to the list for processing
			if (TRUE == dictionary_shape_ok)
			{
				// Determine where in the array this layer's elements start
				element_index = layer_counter * slide_duration;

				// Process the layer information, filling out the relevant elements
				menu_export_flash_create_layer_elements(&swf_timing_array[element_index], slide_duration, this_layer_data, display_depth);
			}

			// Decrement the depth at which this element will be displayed
			display_depth--;
		}

		// Debugging output, displaying what we have in the pre-processing element array thus far
		if (3 == debug_level)
		{
			for (frame_counter = 0; frame_counter < slide_duration; frame_counter++)  // This loops _frame_ number of times
			{
				for (layer_counter = 0; layer_counter < num_layers; layer_counter++)  // This loops _num_layers_ of times
				{
					frame_number = (frame_counter * num_layers) + layer_counter;
					this_frame_ptr = &swf_timing_array[frame_number];

					printf("Frame number %u:\n", frame_number);

					if (TRUE == this_frame_ptr->action_this)
						printf("Action frame:\tON\t");
					else
						printf("Action frame:\tOFF\t");

					if (NULL != this_frame_ptr->layer_info)
						printf("Layer info:\tSET\t");
					else
						printf("Layer info:\tNULL\t");

					if (TRUE == this_frame_ptr->add)
						printf("Add frame:\tON\t");
					else
						printf("Add frame:\tOFF\t");

					if (TRUE == this_frame_ptr->remove)
						printf("Remove frame:\tON\t");
					else
						printf("Remove frame:\tOFF\t");

					if (TRUE == this_frame_ptr->is_moving)
					{
						printf("Move frame:\tON\tX position:\t%.2f\tY position\t%.2f\t", this_frame_ptr->x_position, this_frame_ptr->y_position);
					}
					else
						printf("Move frame:\tOFF\t");

					printf("\n\n");
				}
			}
		}

		// * After all of the layers have been pre-processed we have an array with the per frame info of what should be *
		// * where in the output swf, plus we also have the swf dictionary created and ready to use                     *
		for (frame_counter = 0; frame_counter < slide_duration; frame_counter++)  // This loops _frames_ number of times
		{
			for (layer_counter = 0; layer_counter < num_layers; layer_counter++)  // This loops _num_layers_ of times
			{
				// For each frame, access all of the layers then move to the next frame
				frame_number = (layer_counter * slide_duration) + frame_counter;
				this_frame_ptr = &swf_timing_array[frame_number];

				// Display debugging info if requested
				if (debug_level)
				{
					printf("Processing slide %d, swf array element # %u\n", slide_counter, frame_number);
				}

				// Process this frame element
				// fixme3: Our present approach using swf_elements doesn't work very well with removes. :(
				menu_export_flash_process_element(swf_movie, this_frame_ptr, FALSE);
			}

			// Advance to the next frame
			SWFMovie_nextFrame(swf_movie);
		}

		// * Advance the slide counting variable inside the swf *
		if (debug_level)
		{
			// If we're debugging, then generate debugging swf's too
			inc_slide_counter_action = newSWFAction(
					"if ((_root.this_slide <= (_root.num_slides - 1)) && (false == _root.reversing))"
					" {"
						" _root.this_slide += 1;"
						" trace(\"Slide counter incremented, now equals: \" + _root.this_slide + \".\");"
						" trace(\"'reversing' variable was false, and remains so.\");"
						" trace(\"'playing' variable is unchanged, at: \" + _root.playing + \".\");"
					" }"
					" if (true == _root.reversing)"
					" {"
						" _root.reversing = false;"
						" trace(\"Slide counter unchanged, now at: \" + _root.this_slide + \".\");"
						" trace(\"'reversing' variable was false, has been set to true.\");"
						" trace(\"'playing' variable is unchanged, at: \" + _root.playing + \".\");"
					" };"
					" if (_root.this_slide == _root.num_slides)"
					" {"
						" _root.playing = false;"
						" trace(\"Last frame of movie reached, 'playing' variable has been set to: \" + _root.playing + \".\");"
					" }"
					);
		} else
		{
			inc_slide_counter_action = newSWFAction(
					"if ((_root.this_slide <= (_root.num_slides - 1)) && (false == _root.reversing))"
					" {"
						" _root.this_slide += 1;"
					" };"
					" if (true == _root.reversing)"
					" {"
						" _root.reversing = false;"
					" };"
					" if (_root.this_slide == _root.num_slides)"
					" {"
						" _root.playing = false;"
					" }"
					);
		}
		SWFMovie_add(swf_movie, (SWFBlock) inc_slide_counter_action);

		// Free the memory allocated to the swf timing array, now that this slide is over
		g_free(swf_timing_array);
	}

	// If the end behaviour is to loop, then do that
	switch (end_behaviour)
	{
		case END_BEHAVIOUR_LOOP_PLAY:
			end_action = newSWFAction("_root.playing = true; cb_main.cb_play._visible = false; _root.gotoAndPlay(2);");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;

		case END_BEHAVIOUR_LOOP_STOP:
			end_action = newSWFAction("_root.playing = false; cb_main.cb_play._visible = true; _root.gotoAndStop(2);");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;

		case END_BEHAVIOUR_STOP:
		default:
			end_action = newSWFAction("_root.playing = false; cb_main.cb_play._visible = true; _root.stop();");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;
	}

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("The animation is %u frames long\n", total_frames);
	}

	// Save the swf movie file to disk
	num_bytes_written = SWFMovie_save(swf_movie, output_filename);
	if (-1 == num_bytes_written)  // -1 is returned if an error occurred during the save
	{
		// Something went wrong when saving the swf
		display_warning("Error ED100: Something went wrong when saving the swf file to disk");

		// Free the memory allocated in this function
		destroySWFMovie(swf_movie);
		return FALSE;
	}

	// Free the memory allocated in this function
	destroySWFMovie(swf_movie);

	// Indicate that the swf was created successfully
	return TRUE;
}

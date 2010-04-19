/*
 * $Id$
 *
 * Salasaga: Function to convert a slide structure into a Flash output file
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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


// Turn on C99 compatibility
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// Locale includes
#include <locale.h>
#include <langinfo.h>

// GTK include
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../global_functions.h"
#include "../../dialog/display_warning.h"
#include "../../preference/project_preferences.h"
#include "export_swf_choose_resolution_index.h"
#include "export_swf_control_bar.h"
#include "export_swf_create_layer_elements.h"
#include "export_swf_create_shape.h"
#include "export_swf_process_element.h"
#include "../../flex/flex_mxml_operations.h"

#define NEWSWF

#ifdef NEWSWF
/**
 * Generate flash from gtk layers
 * @param gchar* name of generated swf file
 */
gint export_swf_inner(gchar *output_filename) {
	GString	*message = g_string_new(NULL);		// contains error messages

	// first step, get information about output flash parameters, create mxml DOM object

	// Determine which of the control bar resolutions to use
	guint out_res_index = export_swf_choose_resolution_index();

	// If an unknown output resolution is given, indicate an error and return
	if (out_res_index == -1) {
		g_string_printf(message, "%s ED201: %s", _("Error"), _("Unknown output resolution selected for swf export."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}
	// dom object
	flex_mxml_dom_t dom = flex_mxml_create_document();

	flex_mxml_shape_add_button(dom, 0,0, "test_button");

	// get temporary filename for output mxml file
	gchar mxml_file_name[L_tmpnam + sizeof(".mxml") + 1];		// where to put temporary mxml file

	tmpnam(mxml_file_name);

	strncat (mxml_file_name, ".mxml", sizeof(".mxml"));

	// save DOM to temporary file
	flex_mxml_file_save(dom, mxml_file_name);

	// compile mxml file into flash
	if(flex_mxml_compile_to_swf(mxml_file_name, output_filename,0) !=0) {
		g_string_printf(message, "Failed to compile swf file from mxml file %s", mxml_file_name);
		display_warning(message->str);
		g_string_free(message, TRUE);
		return 0;
	}
	// remove temp

	return 1;
}

#else

gint export_swf_inner(gchar *output_filename)
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
	gchar				*locale_return;				// Receives a return code from the locale setting function
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
	GdkRectangle		tmp_rect = {0, 0, get_main_window()->allocation.width, get_main_window()->allocation.height};  // Temporary rectangle covering the area of the whole Salasaga window
	GString				*message;					// Used to construct message strings
	guint				total_frames;				// The total number of frames in the animation
	guint				total_num_layers;			// The total number of layers in the animation
	gfloat				total_seconds;				// The duration of the entire animation in seconds


	// Initialise variables
	slide_depth = 2;
	total_frames = 0;
	total_seconds = 0;
	set_mouse_click_double_added(FALSE);
	set_mouse_click_single_added(FALSE);
	set_mouse_click_triple_added(FALSE);
	as_gstring = g_string_new(NULL);
	initial_action_gstring = g_string_new(NULL);
	message = g_string_new(NULL);
	slide_name_tmp = g_string_new(NULL);

	// Force the numeric collation to use full stops (for ming text output)
	locale_return = setlocale(LC_NUMERIC, "C");
	if (NULL == locale_return)
	{
		g_string_printf(message, "%s ED417: %s", _("Error"), _("Unable to force locale to C.  Export in non-english locales may not work."));
		display_warning(message->str);
	}

	// Determine which of the control bar resolutions to use
	out_res_index = export_swf_choose_resolution_index();

	// If an unknown output resolution is given, indicate an error and return
	if (-1 == out_res_index)
	{
		g_string_printf(message, "%s ED201: %s", _("Error"), _("Unknown output resolution selected for swf export."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return FALSE;
	}

	// Create an empty swf movie object
	swf_movie = newSWFMovieWithVersion(7);
	Ming_setSWFCompression(9);

	// Set the output size of the swf movie
	SWFMovie_setDimension(swf_movie, get_output_width(), get_output_height());

	// Set the frame rate for the movie
	SWFMovie_setRate(swf_movie, get_frames_per_second());

	// Set the background colour for the animation
	SWFMovie_setBackground(swf_movie, 0x00, 0x00, 0x00);  // RGB value - black

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) get_output_height() / (gfloat) get_project_height();
	scaled_width_ratio = (gfloat) get_output_width() / (gfloat) get_project_width();

	// Displaying debugging info if requested
	if (get_debug_level())
	{
		printf("%s: %.2f\n", _("Scaled height ratio"), scaled_height_ratio);
		printf("%s: %.2f\n", _("Scaled width ratio"), scaled_width_ratio);
	}

	// Count the number of slides in the movie
	set_slides(g_list_first(get_slides()));
	num_slides = g_list_length(get_slides());

	// Output some debugging info if requested
	if (get_debug_level())
	{
		printf("%s: %u\n", _("Number of slides"), num_slides);
	}

	// Count the total number of layers in the movie
	total_num_layers = 2;
	for (slide_counter = 0; slide_counter <  num_slides; slide_counter++)
	{
		// Point to the selected slide
		set_slides(g_list_first(get_slides()));
		this_slide_data = g_list_nth_data(get_slides(), slide_counter);

		// Add the number of layers in this slide to the total count
		total_num_layers += this_slide_data->num_layers;
	}
	total_num_layers += num_slides; // Extra safety margin

	// Output some debugging info if requested
	if (get_debug_level())
	{
		printf("%s: %u\n", _("Total number of layers"), total_num_layers);
	}

	// If requested, add the swf control bar to the movie
	if (TRUE == get_show_control_bar())
	{
		return_code_bool = export_swf_control_bar(swf_movie, out_res_index, total_num_layers + 8);
		if (TRUE != return_code_bool)
		{
			// Something went wrong when adding the control bar to the movie
			destroySWFMovie(swf_movie);
			return FALSE;
		}
	} else
	{
		// Ensure the swf output starts out in the correct play state
		if (START_BEHAVIOUR_PLAY == get_start_behaviour())
		{
			g_string_printf(initial_action_gstring, " var playing = true;");
		} else
		{
			g_string_printf(initial_action_gstring, " var playing = false;");
		}

		// Add the action script to keep the animation paused if that's whats requested
		initial_action_gstring = g_string_append(initial_action_gstring, " if (false == _root.playing) { _root.stop(); };");

		// Add the initialisation action to the movie
		initial_action = compileSWFActionCode(initial_action_gstring->str);
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
		// Repaint the entire window (where the dialog box was, plus the status bar)
		gtk_widget_draw(get_main_window(), &tmp_rect);
		gdk_flush();

		// Initialise things for this slide
		set_slides(g_list_first(get_slides()));
		this_slide_data = g_list_nth_data(get_slides(), slide_counter);
		slide_duration = this_slide_data->duration * get_frames_per_second();

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
		num_layers = this_slide_data->num_layers;

		// * At this stage we should know both the number of frames (slide_duration) and number of layers (num_layers) in the slide *

		// Output some debugging info if requested
		if (get_debug_level())
		{
			printf(_("Number of layers in slide %u is %u\n"), slide_counter, num_layers);
			printf(_("Maximum frame number in slide %u is %u\n"), slide_counter, slide_duration);
		}

		// Create an array that's layers x "number of frames in the slide"
		frame_number = num_layers * slide_duration;
		swf_timing_array = g_try_new0(swf_frame_element, frame_number);
		if (NULL == swf_timing_array)
		{
			g_string_printf(message, "%s ED339: %s", _("Error"), _("We couldn't allocate enough memory to generate the swf.  Export aborted."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		slide_depth += num_layers + 1;
		display_depth = slide_depth;
		this_slide_data = g_list_nth_data(get_slides(), slide_counter);
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// If this layer is not visible, then we skip it
			if (FALSE == this_layer_data->visible)
				continue;

			// Create the dictionary shape for the layer
			dictionary_shape_ok = export_swf_create_shape(swf_movie, this_layer_data);

			// If the creation of the dictionary shape worked, we add this layer to the list for processing
			if (TRUE == dictionary_shape_ok)
			{
				// Determine where in the array this layer's elements start
				element_index = layer_counter * slide_duration;

				// Process the layer information, filling out the relevant elements
				export_swf_create_layer_elements(&swf_timing_array[element_index], slide_duration, this_layer_data, display_depth);
			}

			// Decrement the depth at which this element will be displayed
			display_depth--;
		}

		// Debugging output, displaying what we have in the pre-processing element array thus far
		if (3 == get_debug_level())
		{
			for (frame_counter = 0; frame_counter < slide_duration; frame_counter++)  // This loops _frame_ number of times
			{
				for (layer_counter = 0; layer_counter < num_layers; layer_counter++)  // This loops _num_layers_ of times
				{
					frame_number = (frame_counter * num_layers) + layer_counter;
					this_frame_ptr = &swf_timing_array[frame_number];

					printf(_("Frame number %u:\n"), frame_number);

					if (TRUE == this_frame_ptr->action_this)
						printf(_("Action frame:\tON\t"));
					else
						printf(_("Action frame:\tOFF\t"));

					if (NULL != this_frame_ptr->layer_info)
						printf(_("Layer info:\tSET\t"));
					else
						printf(_("Layer info:\tNULL\t"));

					if (TRUE == this_frame_ptr->add)
						printf(_("Add frame:\tON\t"));
					else
						printf(_("Add frame:\tOFF\t"));

					if (TRUE == this_frame_ptr->remove)
						printf(_("Remove frame:\tON\t"));
					else
						printf(_("Remove frame:\tOFF\t"));

					if (TRUE == this_frame_ptr->is_moving)
					{
						printf(_("Move frame:\tON\tX position:\t%.2f\tY position\t%.2f\t"), this_frame_ptr->x_position, this_frame_ptr->y_position);
					}
					else
						printf(_("Move frame:\tOFF\t"));

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
				if (get_debug_level())
				{
					// Commented out for now, as it's WAY too verbose until extremely needed, even when debugging
					// printf("Processing slide %d, swf array element # %u\n", slide_counter, frame_number);
				}

				// Process this frame element
				// fixme3: Our present approach using swf_elements doesn't work very well with removes. :(
				export_swf_process_element(swf_movie, this_frame_ptr, FALSE);
			}

			// Show movement on the progress bar
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(get_status_bar()));

			// Advance to the next frame
			SWFMovie_nextFrame(swf_movie);
		}

		// * Advance the slide counting variable inside the swf *
		if (get_debug_level())
		{
			// If we're debugging, then generate debugging swf's too
			g_string_printf(message,
					// Format string is grouped as per the string comments from one line below
					"%s" "%s%s%s" "%s%s%s" "%s%s" "%s" "%s" "%s%s%s" "%s%s%s" "%s%s" "%s" "%s" "%s%s" "%s",

					// The grouped strings and comments
					"if ((_root.this_slide <= (_root.num_slides - 1)) && (false == _root.reversing))"
					" {"
						" _root.this_slide += 1;",												// %s

						" trace(\"",															// %s
						_("Slide counter incremented, now equals:"),							// %s
						" \" + _root.this_slide + \".\");",										// %s

						" trace(\"",															// %s
						_("'reversing' variable was false, and remains so."),					// %s
						"\");",																	// %s

						" trace(\"",															// %s
						_("'playing' variable is unchanged, at:"),								// %s
						" \" + _root.playing + \".\");"
					" }",																		// %s

					" if (true == _root.reversing)"
					" {"
						" _root.reversing = false;",											// %s

						" trace(\"",															// %s
						_("Slide counter unchanged, now at:"),									// %s
						" \" + _root.this_slide + \".\");",										// %s

						" trace(\"",															// %s
						_("'reversing' variable was false, has been set to true."),				// %s
						"\");",																	// %s

						" trace(\"",															// %s
						_("'playing' variable is unchanged, at:"),								// %s
						" \" + _root.playing + \".\");"
					" };", 																		// %s

					" if (_root.this_slide == _root.num_slides)"
					" {"
						" _root.playing = false;",												// %s

						" trace(\"",															// %s
						_("Last frame of movie reached, 'playing' variable has been set to:"),	// %s
						" \" + _root.playing + \".\");"

					" }"																		// %s
					);

			inc_slide_counter_action = compileSWFActionCode(message->str);
			//g_string_free(message, TRUE);
		} else
		{
			inc_slide_counter_action = compileSWFActionCode(
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
	switch (get_end_behaviour())
	{
		case END_BEHAVIOUR_LOOP_PLAY:
			end_action = compileSWFActionCode("_root.playing = true; cb_main.cb_play._visible = false; _root.gotoAndPlay(2);");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;

		case END_BEHAVIOUR_LOOP_STOP:
			end_action = compileSWFActionCode("_root.playing = false; cb_main.cb_play._visible = true; _root.gotoAndStop(2);");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;

		case END_BEHAVIOUR_STOP:
		default:
			end_action = compileSWFActionCode("_root.playing = false; cb_main.cb_play._visible = true; _root.stop();");
			SWFMovie_add(swf_movie, (SWFBlock) end_action);
			break;
	}

	// Output some debugging info if requested
	if (get_debug_level())
	{
		printf(_("The animation is %u frames long\n"), total_frames);
	}

	// Save the swf movie file to disk
	num_bytes_written = SWFMovie_save(swf_movie, output_filename);
	if (-1 == num_bytes_written)  // -1 is returned if an error occurred during the save
	{
		// Something went wrong when saving the swf
		g_string_printf(message, "%s ED100: %s", _("Error"), _("Something went wrong when saving the swf file to disk."));
		display_warning(message->str);

		// Free the memory allocated in this function
		g_string_free(message, TRUE);
		destroySWFMovie(swf_movie);
		return FALSE;
	}

	// Free the memory allocated in this function
	destroySWFMovie(swf_movie);

	// Reset the numeric locale back to the user's one
	if (NULL != locale_return)
	{
		setlocale(LC_NUMERIC, "");
	}

	// Indicate that the swf was created successfully
	return TRUE;
}

#endif

/*
 * $Id$
 *
 * Flame Project: Function to convert a slide structure into a Flash output file
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "flash_create_header.h"
#include "flash_create_tag_bitmap.h"
#include "flash_create_tag_highlight.h"
#include "flash_create_tag_mouse.h"
#include "flash_create_tag_show_frame.h"
#include "flash_create_tag_text.h"
#include "flash_layer_display_list_add.h"
#include "flash_layer_display_list_remove.h"
#include "flash_movie_end.h"


GByteArray *menu_export_flash_inner()
{
	// Local variables
	guint16				char_counter;			// Counter of the number of characters in the animation
	guint				layer_counter;			// Holds the number of layers
	guint				max_frames = 0;			// The highest frame number in the slide
	guint				num_layers = 0;			// The number of layers in the slide
	guint				num_slides;				// The number of slides in the movie
	guint				slide_counter;			// Holds the number of slides
	GByteArray 			*swf_buffer;			// Holds the swf buffer as we create it
	swf_frame_element	*swf_timing_array = NULL;  // Used to coordinate the actions in each frame
	layer				*this_layer_data;		// Points to the data in the present layer
	slide				*this_slide_data;		// Points to the data in the present slide
	guint				total_frames;			// The total number of frames in the animation

	guint				element_counter;
	guint				element_max;

	gint				element_x_position_increment = 0;
	gint				element_x_position_start = 0;
	gint				element_y_position_increment = 0;
	gint				element_y_position_start = 0;

	GByteArray			*tmp_byte_array;		// Temporary byte array
	guint				tmp_integer;			// Temporary integer


	// Initialise variables
	char_counter = 1;  // Character 0 is reserved in Flash, so we avoid it
	swf_buffer = g_byte_array_new();
	total_frames = 0;


// For now, I'm using a two pass per slide approach
//	1st pass gathers timing info, and creates an array of timing information for each slide
//		The array has one element for each frame times each layer of the slide (i.e. num elements = num frames x num layers)
//		Also creates the dictionary of shapes, and adds them to an output buffer
//	2nd pass writes out the actions to happen for the slide, relying on the dictionary images created in the first pass
// (sounds like a reasonable approach (theory) for a first go, lets see it works in reality though)


	// For each slide, work out how many layers there are and how many frames the entire slide lasts for
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("Number of slides: %u\n", num_slides);
	}

	for (slide_counter = 0; slide_counter <  num_slides; slide_counter++)
	{
		// Initialise things for this slide
		this_slide_data = g_list_nth_data(slides, slide_counter);
		max_frames = 0;

		// Work out how many layers there are in this slide
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		num_layers = g_list_length(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			// Work out the maximum frame number for the slide
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);
			if (this_layer_data->finish_frame > max_frames)
			{
				max_frames = this_layer_data->finish_frame;
			}
		}

		// * At this stage we should know both the maximum frame number and number of layers in the slide *

		// Output some debugging info if requested
		if (debug_level)
		{
			printf("Number of layers in slide %u is %u\n", slide_counter, num_layers);
			printf("Maximum frame number in slide %u is %u\n", slide_counter, max_frames);
		}

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += max_frames;

		// Create an array that's layers x max number of frames
		swf_timing_array = g_new0(swf_frame_element, num_layers * max_frames);

		// Point to the first layer again
		this_slide_data = g_list_nth_data(slides, slide_counter);

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// Mark that the element needs to be shown on this frame
			swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].add = TRUE;

			// Mark that the element needs to be removed on this frame
			swf_timing_array[(layer_counter * max_frames) + this_layer_data->finish_frame].remove = TRUE;

			// Calculate the starting positions, increments and other info for each frame of the layer
			switch (this_layer_data->object_type)
			{
				case TYPE_GDK_PIXBUF:
					// * We're processing an image layer *

					// Create the dictionary shape for this layer
					tmp_byte_array = flash_create_tag_bitmap((layer_image *) this_layer_data->object_data, char_counter);
					if (NULL == tmp_byte_array)
					{
						// Something went wrong when creating the dictionary shape for this layer
						display_warning("Error ED90: Something went wrong when creating the dictionary shape for this image layer");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary

					} else
					{
						// Creation of shape went ok, so continue
						swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_GDK_PIXBUF;
						swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
						element_x_position_start = ((layer_image *) this_layer_data->object_data)->x_offset_start;
						element_y_position_start = ((layer_image *) this_layer_data->object_data)->y_offset_start;
						element_x_position_increment = (((layer_image *) this_layer_data->object_data)->x_offset_finish - ((layer_image *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
						element_y_position_increment = (((layer_image *) this_layer_data->object_data)->y_offset_finish - ((layer_image *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);

						// Add the dictionary stream to the output buffer
						swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);
					}

					break;

				case TYPE_HIGHLIGHT:
					tmp_byte_array = NULL;  // The code for this layer type hasn't been written
					break;
					// We're processing a highlight layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_HIGHLIGHT;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_highlight *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_highlight *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_highlight *) this_layer_data->object_data)->x_offset_finish - ((layer_highlight *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_highlight *) this_layer_data->object_data)->y_offset_finish - ((layer_highlight *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);

					// Create the dictionary shape for this layer
					tmp_byte_array = flash_create_tag_highlight((layer_highlight *) this_layer_data->object_data);

					// Add the dictionary stream to the output buffer
					swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

					break;

				case TYPE_MOUSE_CURSOR:
					tmp_byte_array = NULL;  // The code for this layer type hasn't been written
					break;
					// We're processing a mouse layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_MOUSE_CURSOR;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_mouse *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_mouse *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_mouse *) this_layer_data->object_data)->x_offset_finish - ((layer_mouse *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_mouse *) this_layer_data->object_data)->y_offset_finish - ((layer_mouse *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);

					// Create the dictionary shape for this layer
					tmp_byte_array = flash_create_tag_mouse((layer_mouse *) this_layer_data->object_data);

					// Add the dictionary stream to the output buffer
					swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

					break;

				case TYPE_TEXT:
					tmp_byte_array = NULL;  // The code for this layer type hasn't been written
					break;
					// We're processing a text layer
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_type = TYPE_TEXT;
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].layer_data = this_layer_data->object_data;
					element_x_position_start = ((layer_text *) this_layer_data->object_data)->x_offset_start;
					element_y_position_start = ((layer_text *) this_layer_data->object_data)->y_offset_start;
					element_x_position_increment = (((layer_text *) this_layer_data->object_data)->x_offset_finish - ((layer_text *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_text *) this_layer_data->object_data)->y_offset_finish - ((layer_text *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);

					// Create the dictionary shape for this layer
					tmp_byte_array = flash_create_tag_text((layer_text *) this_layer_data->object_data);

					// Add the dictionary stream to the output buffer
					swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

					break;

				default:
					// Unknown type
					display_warning("ED83: Unknown layer type in swf output");
					break;
			}

			// If the creation of the dictionary shape didn't work, we shouldn't add this layer to the list for processing
			if (NULL != tmp_byte_array)
			{
				// Free the temporary dictionary shape buffer
				g_byte_array_free(tmp_byte_array, TRUE);

				// Process through the element array, setting flags and info as required for this layer
				element_max = this_layer_data->finish_frame;
				for (element_counter = this_layer_data->start_frame; element_counter <= element_max; element_counter++)
				{
					// Mark that the element should be processed on this frame
					swf_timing_array[(layer_counter * max_frames) + element_counter].action_this = TRUE;

					// Give the object a unique character ID
					swf_timing_array[(layer_counter * max_frames) + element_counter].char_id = char_counter;

					// Store the x and y positions for each frame
					// fixme2: I think using element_counter in "element_counter * element_x_position_increment" here is wrong,
					// and that it should be a separate counter that starts with 0
					swf_timing_array[(layer_counter * max_frames) + element_counter].x_position = element_x_position_start + (element_counter * element_x_position_increment);
					swf_timing_array[(layer_counter * max_frames) + element_counter].y_position = element_y_position_start + (element_counter * element_y_position_increment);

					// Store the opacity setting for each frame
					// fixme2: Still need to calculate properly rather than hard code to 100% for the moment
					swf_timing_array[(layer_counter * max_frames) + element_counter].opacity = 65535;

					// Store the layer depth
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].depth = layer_counter + 1;
				}

				// Increment the character counter
				char_counter++;
			}
		}

		// * After all of the layers have been pre-processed we have an array  *
		// * with the per frame info of what should be where in the output swf *
		for (tmp_integer = 0; tmp_integer < max_frames; tmp_integer++)  // This loops _frame_ number of times
		{
			for (element_counter = 0; element_counter < num_layers; element_counter++)  // This loops _num_layers_ of times
			{
				// For each frame, access all of the layers then move to the next frame
				if (TRUE == swf_timing_array[(tmp_integer * num_layers) + element_counter].action_this)
				{
					// * There is something to be done in this frame for this layer *

					if (TRUE == swf_timing_array[(tmp_integer * num_layers) + element_counter].add)
					{
						// * Add the character to the swf display list *

						// Create the swf tag for adding this layer to the display list
						tmp_byte_array = flash_layer_display_list_add(
								swf_timing_array[(tmp_integer * num_layers) + element_counter].char_id,  // Character id
								swf_timing_array[(tmp_integer * num_layers) + element_counter].depth);  // Depth

						// Add this swf tag to the output stream
						swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

						// Free the temporary swf tag
						g_byte_array_free(tmp_byte_array, TRUE);
					}

					if (TRUE == swf_timing_array[(tmp_integer * num_layers) + element_counter].remove)
					{
						// * Remove the character to the swf display list *

						// Create the swf tag for removing this layer from the display list
						tmp_byte_array = flash_layer_display_list_remove(
								swf_timing_array[(tmp_integer * num_layers) + element_counter].depth);  // Depth

						// Add this swf tag to the output stream
						swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

						// Free the temporary swf tag
						g_byte_array_free(tmp_byte_array, TRUE);
					}
				}

				// Add the swf tag to show the frame
				tmp_byte_array = flash_create_tag_show_frame();
				swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

				// Free the temporary swf tag
				g_byte_array_free(tmp_byte_array, TRUE);
			}		
		}
	}

	// Add the swf tag that signifies the end of the movie
	tmp_byte_array = flash_movie_end();
	swf_buffer = g_byte_array_append(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

	// Free the temporary swf array
	g_byte_array_free(tmp_byte_array, TRUE);

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("The animation is %u frames long\n", total_frames);
	}

	// Create the swf header
	tmp_byte_array = flash_create_header(swf_buffer->len, total_frames, frames_per_second);

	// Add the header to the swf buffer
	swf_buffer = g_byte_array_prepend(swf_buffer, tmp_byte_array->data, tmp_byte_array->len);

	// Free the temporary swf array
	g_byte_array_free(tmp_byte_array, TRUE);

	return swf_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.14  2007/10/15 06:59:45  vapour
 * Passes character id to the swf bitmap definition function.
 *
 * Revision 1.13  2007/10/10 13:58:18  vapour
 * Fixed a crashing bug with swf export due to use of wrong variable.
 * Improved some of the swf debugging code.
 *
 * Revision 1.12  2007/10/07 14:15:23  vapour
 * Moved initial allocation of swf buffer into the inner function.
 *
 * Revision 1.11  2007/10/07 09:00:46  vapour
 * Moved the code for creating the swf header bytes into its own function.
 *
 * Revision 1.10  2007/10/07 08:44:37  vapour
 * Added initial (untested) code for creating the swf header.
 *
 * Revision 1.9  2007/10/07 06:31:41  vapour
 * Lots of initial (untested) code added for generating swf, mostly for bitmaps at the moment, just to see how things are hanging together.
 *
 * Revision 1.8  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.7  2007/10/04 11:50:27  vapour
 * Added initial (non working) stub functions for the creation of swf byte code for hightlight, mouse cursor, and text layers.
 *
 * Revision 1.6  2007/10/03 13:40:04  vapour
 * Updated parameter in call to flash bitmap function.
 *
 * Revision 1.5  2007/10/01 13:17:46  vapour
 * Updated swf character numbering to start with 1.
 *
 * Revision 1.4  2007/10/01 12:19:06  vapour
 * Updated to call stub function flash_create_tag_bitmap().
 *
 * Revision 1.3  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:14  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

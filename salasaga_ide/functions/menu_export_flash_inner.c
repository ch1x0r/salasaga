/*
 * $Id$
 *
 * Flame Project: Function to convert a slide structure into a Flash output file
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

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


SWFMovie menu_export_flash_inner(SWFMovie this_movie)
{
	// Local variables
	gboolean			dictionary_shape_ok;		// Temporary value indicating if a dictionary shape was created ok or not
	GError				*error = NULL;				// Pointer to error return structure
	guint				frame_counter;				// Holds the number of frames
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	guint				layer_counter;				// Holds the number of layers
	guint				max_frames = 0;				// The highest frame number in the slide
	guint				num_layers = 0;				// The number of layers in the slide
	guint				num_slides;					// The number of slides in the movie
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed jpeg image
	gsize				pixbuf_size;				// Is given the size of a compressed jpeg image
	guint				position_counter;			// Temporary counter integer
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return values
	guint				slide_counter;				// Holds the number of slides
	swf_frame_element	*swf_timing_array = NULL;	// Used to coordinate the actions in each frame
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	layer				*this_layer_data;			// Points to the data in the present layer
	layer				*this_layer_info;			// Used to point to layer data when looping
	slide				*this_slide_data;			// Points to the data in the present slide
	guint				total_frames;				// The total number of frames in the animation

	SWFDisplayItem		display_list_object;		// Temporary display list object
	SWFShape			highlight_box;				// Temporary swf shape used when constructing highlight boxes
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes

	guint				element_number;
	guint				frame_number;

	gint				element_x_position_finish = 0;
	gint				element_x_position_increment = 0;
	gint				element_x_position_start = 0;
	gint				element_y_position_finish = 0;
	gint				element_y_position_increment = 0;
	gint				element_y_position_start = 0;


	// Initialise variables
	total_frames = 0;

	// Create the fill style used in highlight boxes
	highlight_fill_style = newSWFSolidFillStyle(0x00, 0xff, 0x00, 0x40);
	if (NULL == highlight_fill_style)
	{
		// Something went wrong creating the fill style, so we don't proceed with creating the swf
		return NULL;
	}


// For now, this uses a two pass per slide approach
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

		// * At this stage we should know both the maximum frame number (max_frames) and number of layers (num_layers) in the slide *

		// Output some debugging info if requested
		if (debug_level)
		{
			printf("Number of layers in slide %u is %u\n", slide_counter, num_layers);
			printf("Maximum frame number in slide %u is %u\n", slide_counter, max_frames);
		}

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += max_frames;

		// Create an array that's layers x "number of frames in the slide"
		frame_number = num_layers * (max_frames + 1);  // +1 because if (ie.) we say slide 5, then we really mean the 6th slide (we start from 0)
		swf_timing_array = g_new0(swf_frame_element, frame_number); 

		// Point to the first layer again
		this_slide_data = g_list_nth_data(slides, slide_counter);

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// Mark this element as not having a dictionary shape created yet
			dictionary_shape_ok = FALSE;

			// Create the dictionary shape for this layer
			switch (this_layer_data->object_type)
			{
				case TYPE_GDK_PIXBUF:
					// * We're processing an image layer *

					// Scale the image to the correct dimensions
					// fixme2: This simple code that uses output_width and output_height will only work for background images!
					resized_pixbuf = gdk_pixbuf_scale_simple(((layer_image *) this_layer_data->object_data)->image_data, output_width, output_height, scaling_quality);
					if (NULL == resized_pixbuf)
					{
						// Something went wrong when creating the dictionary shape for this layer
						display_warning("Error ED90: Something went wrong when creating the dictionary shape for this image layer");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;

					}

					// Convert the compressed image into jpeg data
					return_code_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(resized_pixbuf),
									&pixbuf_buffer,  // Will come back filled out with location of jpeg data
									&pixbuf_size,  // Will come back filled out with size of jpeg data
									"jpeg",
									&error,
									"quality",
									"100",
									NULL);
					if (FALSE == return_code_bool)
					{
						// Something went wrong when encoding the image to jpeg format
						display_warning("ED51: Something went wrong when encoding a slide to jpeg format");

						// Free the memory allocated in this function
						g_error_free(error);
						if (NULL != resized_pixbuf)
							g_object_unref(resized_pixbuf);

						break;
					}

					// Create the dictionary shape for this layer
					SWFInput image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
					SWFJpegBitmap image_bitmap = newSWFJpegBitmap_fromInput(image_input);

					// Store the dictionary shape for future reference
					this_layer_data->dictionary_shape = (SWFBlock) image_bitmap;

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				case TYPE_HIGHLIGHT:

					// * We're processing a highlight layer *

					// Create the initial empty shape
					highlight_box = newSWFShape();
					if (NULL == highlight_box)
					{
						// Something went wrong when creating the empty shape, so we skip this layer
						break;
					}

					// Set the semi-transparent green fill for the highlight box
					SWFShape_setRightFillStyle(highlight_box, highlight_fill_style);

					// Set the line style
					SWFShape_setLine(highlight_box, 2, 0x00, 0xff, 0x00, 0xcc);  // Width = 2 seems to work ok

					// Work out the scaled dimensions of the highlight box
					highlight_box_width = ((layer_highlight *) this_layer_data->object_data)->width;  // fixme3: This needs to be scaled
					highlight_box_height = ((layer_highlight *) this_layer_data->object_data)->height;  // fixme3: This needs to be scaled

					// Create the highlight box
					SWFShape_drawLine(highlight_box, highlight_box_width, 0.0);
					SWFShape_drawLine(highlight_box, 0.0, highlight_box_height);
					SWFShape_drawLine(highlight_box, -(highlight_box_width), 0.0);
					SWFShape_drawLine(highlight_box, 0.0, -(highlight_box_height));

					// Store the dictionary shape for future reference
					this_layer_data->dictionary_shape = (SWFBlock) highlight_box;

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				case TYPE_MOUSE_CURSOR:

					// We're processing a mouse layer

					// fixme2: Create the dictionary shape for this layer
					
					break;

				case TYPE_TEXT:

					// We're processing a text layer

					// fixme2: Create the dictionary shape for this layer

					break;

				default:
					// Unknown type
					display_warning("ED83: Unknown layer type in swf output");
					break;
			}

			// If the creation of the dictionary shape worked, we add this layer to the list for processing
			if (TRUE == dictionary_shape_ok)
			{
				// Process the element array, setting flags and info as required for this layer
				element_x_position_start = ((layer_highlight *) this_layer_data->object_data)->x_offset_start;
				element_x_position_finish = ((layer_highlight *) this_layer_data->object_data)->x_offset_finish;
				element_y_position_start = ((layer_highlight *) this_layer_data->object_data)->y_offset_start;
				element_y_position_finish = ((layer_highlight *) this_layer_data->object_data)->y_offset_finish;

				// If the layer moves, flag this and calculate the increment in each direction
				if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
				{
					swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].is_moving = TRUE;
					element_x_position_increment = (((layer_highlight *) this_layer_data->object_data)->x_offset_finish - ((layer_highlight *) this_layer_data->object_data)->x_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);  // fixme3: This MAY need to be scaled
					element_y_position_increment = (((layer_highlight *) this_layer_data->object_data)->y_offset_finish - ((layer_highlight *) this_layer_data->object_data)->y_offset_start) / (this_layer_data->finish_frame - this_layer_data->start_frame);  // fixme3: This MAY need to be scaled
				}

				// Indicate on which frame the element should be displayed
				frame_number = (layer_counter * max_frames) + this_layer_data->start_frame;
				swf_timing_array[frame_number].add = TRUE;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting ADD value for layer %u in swf element %u\n", layer_counter, frame_number);
				}

				// Indicate on which frame the element should be removed from display
				frame_number = (layer_counter * max_frames) + this_layer_data->finish_frame;
				swf_timing_array[frame_number].remove = TRUE;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting REMOVE value for layer %u in swf element %u\n", layer_counter, frame_number);
				}

				// Re-initialise the position counter for each shape
				position_counter = 0;
				for (frame_number = this_layer_data->start_frame; frame_number <= this_layer_data->finish_frame; frame_number++)
				{
					element_number = (layer_counter * max_frames) + frame_number;
					this_frame_ptr = &swf_timing_array[element_number];

					// Display debugging info if requested
					if (debug_level)
					{
						printf("Setting values inside element # %u\n", element_number);
					}

					// Indicate the element should be processed on this frame
					this_frame_ptr->action_this = TRUE;

					// Ensure the appropriate layer data can be found by later code 
					this_frame_ptr->layer_info = this_layer_data;

					// If the layer moves, store the x and y positions for each frame
					if (TRUE == swf_timing_array[(layer_counter * max_frames) + this_layer_data->start_frame].is_moving)
					{
						this_frame_ptr->is_moving = TRUE;
						this_frame_ptr->x_position = element_x_position_start + (position_counter * element_x_position_increment);
						this_frame_ptr->y_position = element_y_position_start + (position_counter * element_y_position_increment);
						position_counter++;
					}

					// Store the opacity setting for each frame
					// fixme2: Still need to calculate properly rather than hard code to 100% for the moment
					this_frame_ptr->opacity = 65535;
				}

			}
		}

		// Debugging output, displaying what we have in the pre-processing element array thus far
		if (debug_level)
		{
			for (frame_counter = 0; frame_counter <= max_frames; frame_counter++)  // This loops _frame + 1_ number of times
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
						printf("Move frame:\tON\tX position:\t%u\tY position\t%u\t", this_frame_ptr->x_position, this_frame_ptr->y_position);
					}
					else
						printf("Move frame:\tOFF\t");

					printf("\n\n");
				}
			}
		}

		// * After all of the layers have been pre-processed we have an array with the per frame info of what should be *
		// * where in the output swf, plus we also have the swf dictionary created and ready to use                     *
		for (frame_counter = 0; frame_counter <= max_frames; frame_counter++)  // This loops _frames + 1_ number of times
		{
			for (layer_counter = 0; layer_counter < num_layers; layer_counter++)  // This loops _num_layers_ of times
			{
				// For each frame, access all of the layers then move to the next frame
				frame_number = (layer_counter * (max_frames + 1)) + frame_counter;
				this_frame_ptr = &swf_timing_array[frame_number];

				// Display debugging info if requested
				if (debug_level)
				{
					printf("Processing swf element # %u\n", frame_number);
				}

				// Is this a frame in which some action needs to take place?
				if (TRUE == this_frame_ptr->action_this)
				{
					// * There is something to be done in this frame for this layer *

					this_layer_info = this_frame_ptr->layer_info;

					// Is this the frame in which the layer is added to the display?
					if (TRUE == this_frame_ptr->add)
					{
						// * Add the character to the swf display list *
						display_list_object = SWFMovie_add(this_movie, (SWFBlock) this_layer_info->dictionary_shape);

						// Store the display list object for future reference
						this_layer_info->display_list_item = display_list_object;
					}

					// Is this the frame in which the layer is removed from the display?
					if (TRUE == this_frame_ptr->remove)
					{
						// * Remove the character from the swf display list *

						// Get the appropriate display list object
						display_list_object = this_layer_info->display_list_item;

						// Remove the character from the display list
						SWFDisplayItem_remove(display_list_object);
					}

					// Does the layer need to be moved/positioned in this frame?
					if (TRUE == this_frame_ptr->is_moving)
					{
						// * Adjust the x and y position of the character on the display list *

						// Get the appropriate display list object
						display_list_object = this_layer_info->display_list_item;

						// (Re-)position the object
						SWFDisplayItem_moveTo(display_list_object, this_frame_ptr->x_position, this_frame_ptr->y_position);
					}
				}
			}

			// Advance to the next frame
			SWFMovie_nextFrame(this_movie);
		}
	}

	// Output some debugging info if requested
	if (debug_level)
	{
		printf("The animation is %u frames long\n", total_frames);
	}

	// Return the swf movie
	return this_movie;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.26  2008/01/16 05:51:11  vapour
 * Added initial code for creating swf dictionary shapes for background images.  Needs testing.
 *
 * Revision 1.25  2008/01/15 20:13:19  vapour
 * Adjusted the order swf frames are processed in.  Now much better.
 *
 * Revision 1.24  2008/01/15 16:18:58  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.23  2008/01/15 13:23:42  vapour
 * Fixed an incorrect use of pointers, so now the main loop kind of works.
 *
 * Revision 1.22  2008/01/15 12:01:55  vapour
 * Significantly reworked much of the swf generation pre-processing loops and array processing code.  Added a lot of comments and debugging output as well.
 *
 * Revision 1.21  2008/01/13 12:24:31  vapour
 * Adjusted the size of the highlight box to be closer to the real desired size.  Still needs to be scaled though.
 *
 * Revision 1.20  2008/01/13 10:43:54  vapour
 * Updated to use Ming (0.4.0 beta 5).
 *
 * Revision 1.19  2008/01/08 02:43:25  vapour
 * Improved the clarity and verbosity of some of the comments.
 *
 * Revision 1.18  2007/10/21 12:21:29  vapour
 * Added code to add the new shape to the swf output stream.
 *
 * Revision 1.17  2007/10/18 11:28:53  vapour
 * Updated variable definition and declaration for the flash_create_tag_define_shape_bg_image function.
 * Updated code calling it to also pass the bitmap id.
 * Added initial untested code to include fill style information.
 * Still needs work and testing.
 *
 * Revision 1.16  2007/10/17 11:04:07  vapour
 * Adding initial code to define shapes to place the background images on.  Still needs work.
 *
 * Revision 1.15  2007/10/15 08:49:28  vapour
 * Updated to set the background color of the movie.
 *
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

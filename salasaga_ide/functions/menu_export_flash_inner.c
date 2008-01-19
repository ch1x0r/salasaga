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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


gint menu_export_flash_inner(gchar *output_filename)
{
	// Local variables
	gboolean			dictionary_shape_ok;		// Temporary value indicating if a dictionary shape was created ok or not
	GError				*error = NULL;				// Pointer to error return structure
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	guint				frame_counter;				// Holds the number of frames
	gint				num_bytes_written;			// Receives the number of bytes written to disk for the swf output
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	gint				image_height;				// Temporarily used to store the height of an image
	gint				image_width;				// Temporarily used to store the width of an image
	guint				layer_counter;				// Holds the number of layers
	guint				num_layers = 0;				// The number of layers in the slide
	guint				num_slides;					// The number of slides in the movie
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed jpeg image
	gsize				pixbuf_size;				// Is given the size of a compressed jpeg image
	guint				position_counter;			// Temporary counter integer
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return values
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	guint				slide_counter;				// Holds the number of slides
	guint				slide_duration;				// Holds the total number of frames in this slide
	swf_frame_element	*swf_timing_array = NULL;	// Used to coordinate the actions in each frame
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	layer				*this_layer_data;			// Points to the data in the present layer
	layer				*this_layer_info;			// Used to point to layer data when looping
	slide				*this_slide_data;			// Points to the data in the present slide
	guint				total_frames;				// The total number of frames in the animation

	SWFDisplayItem		display_list_object;		// Temporary display list object
	SWFFont				font_object;				// The font we use gets loaded into this
	SWFShape			highlight_box;				// Temporary swf shape used when constructing highlight boxes
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes
	SWFJpegBitmap		image_bitmap;				// Used to hold a scaled bitmap object
	SWFInput			image_input;				// Used to hold a swf input object
	SWFMovie			swf_movie;					// Swf movie object
	SWFShape			text_bg;					// The text background shape goes in this
	gint				text_bg_box_height;			// Used while generating swf output for text boxes
	gint				text_bg_box_width;			// Used while generating swf output for text boxes
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text object we're working on goes in this

	guint				element_number;
	guint				frame_number;

	gfloat				element_x_position_finish = 0;
	gfloat				element_x_position_increment = 0;
	gfloat				element_x_position_start = 0;
	gfloat				element_y_position_finish = 0;
	gfloat				element_y_position_increment = 0;
	gfloat				element_y_position_start = 0;


	// Initialise variables
	total_frames = 0;

	// Initialise Ming and create an empty swf movie object
	Ming_init();
	swf_movie = newSWFMovieWithVersion(7);
	Ming_setSWFCompression(9);

	// Set the output size of the swf movie 
	SWFMovie_setDimension(swf_movie, output_width, output_height);

	// Set the frame rate for the movie
	SWFMovie_setRate(swf_movie, frames_per_second);

	// Set the background color for the animation
	SWFMovie_setBackground(swf_movie, 0x00, 0x00, 0x00);  // RGB value - black

	// Create the fill style used in highlight boxes
	highlight_fill_style = newSWFSolidFillStyle(0x00, 0xff, 0x00, 0x40);
	if (NULL == highlight_fill_style)
	{
		// Something went wrong creating the fill style, so we don't proceed with creating the swf
		display_warning("Error ED97: Something went wrong when creating the highlight fill style for the swf");

		// Free the memory allocated in this function
		destroySWFMovie(swf_movie);

		return FALSE;
	}

	// Create the fill style used in text background shapes
	text_bg_fill_style = newSWFSolidFillStyle(0xff, 0xff, 0xcc, 0xff);
	if (NULL == text_bg_fill_style)
	{
		// Something went wrong creating the fill style, so we don't proceed with creating the swf
		display_warning("Error ED102: Something went wrong when creating the text background fill style for the swf");

		// Free the memory allocated in this function
		destroySWFMovie(swf_movie);
		destroySWFFillStyle(highlight_fill_style);

		return FALSE;
	}

	// Create the (one and only for now) font style used in text boxes
	font_pathname = g_build_path(G_DIR_SEPARATOR_S, font_path, "fdb", "Bitstream Vera Sans.fdb", NULL);
	font_object = newSWFFont_fromFile(font_pathname);
	if (NULL == font_object)
	{
		// Something went wrong when loading the font file, so return
		display_warning("Error ED96: Something went wrong when loading the font file");

		// Free the memory allocated in this function
		destroySWFMovie(swf_movie);
		destroySWFFillStyle(highlight_fill_style);
		destroySWFFillStyle(text_bg_fill_style);
		g_free(font_pathname);

		return FALSE;
	}

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) output_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) output_width / (gfloat) project_width;

	// Displaying debugging info if requested
	if (debug_level)
	{
		printf("Scaled height ratio: %.2f\n", scaled_height_ratio);
		printf("Scaled width ratio: %.2f\n", scaled_width_ratio);
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
		slide_duration = this_slide_data->duration;

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

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += slide_duration;

		// Create an array that's layers x "number of frames in the slide"
		frame_number = num_layers * (slide_duration + 1);  // +1 because if (ie.) we say slide 5, then we really mean the 6th slide (we start from 0)
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

					// Work out the correct dimensions for this image in the output
					image_height = ((layer_image *) this_layer_data->object_data)->height;
					scaled_height = roundf(scaled_height_ratio * (gfloat) image_height);
					image_width = ((layer_image *) this_layer_data->object_data)->width;
					scaled_width = roundf(scaled_width_ratio * (gfloat) image_width);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Image height: %d\n", image_height);
						printf("Scaled height: %d\n", scaled_height);
						printf("Image width: %d\n", image_width);
						printf("Scaled width: %d\n", scaled_width);
					}

					// Scale the image to the correct dimensions
					resized_pixbuf = gdk_pixbuf_scale_simple(((layer_image *) this_layer_data->object_data)->image_data, scaled_width, scaled_height, GDK_INTERP_HYPER);  // Do the scaling at best possible quality
					if (NULL == resized_pixbuf)
					{
						// Something went wrong when creating the dictionary shape for this layer
						display_warning("Error ED90: Something went wrong when creating the dictionary shape for an image layer");

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
						display_warning("Error ED92: Something went wrong when encoding an image to jpeg format");

						// Free the memory allocated in this function
						g_error_free(error);
						if (NULL != resized_pixbuf)
							g_object_unref(resized_pixbuf);

						break;
					}

					// Create the dictionary shape from the jpeg data
					image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
					image_bitmap = newSWFJpegBitmap_fromInput(image_input);

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
						display_warning("Error ED98: Something went wrong when creating a highlight layer for swf output");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;
					}

					// Set the semi-transparent green fill for the highlight box
					SWFShape_setRightFillStyle(highlight_box, highlight_fill_style);

					// Set the line style
					SWFShape_setLine(highlight_box, 2, 0x00, 0xff, 0x00, 0xcc);  // Width = 2 seems to work ok

					// Work out the scaled dimensions of the highlight box
					highlight_box_width = roundf(scaled_width_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->width);
					highlight_box_height = roundf(scaled_height_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->height);

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

					// * We're processing a mouse layer *

					// If we weren't able to load the mouse cursor graphic, we aren't going to be able to export it
					if (NULL == mouse_ptr_pixbuf)
					{
						// Something went wrong when creating the empty shape, so we skip this layer
						display_warning("Error ED99: Couldn't load the mouse pointer image, for adding to the swf output");

						break;
					}

					// Work out the correct dimensions for the mouse cursor in the output
					image_height = ((layer_mouse *) this_layer_data->object_data)->height;
					scaled_height = roundf(scaled_height_ratio * (gfloat) image_height);
					image_width = ((layer_mouse *) this_layer_data->object_data)->width;
					scaled_width = roundf(scaled_width_ratio * (gfloat) image_width);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Mouse cursor height: %d\n", image_height);
						printf("Scaled height: %d\n", scaled_height);
						printf("Mouse cursor: %d\n", image_width);
						printf("Scaled width: %d\n", scaled_width);
					}

					// Scale the mouse cursor to the correct dimensions
					resized_pixbuf = gdk_pixbuf_scale_simple(mouse_ptr_pixbuf, scaled_width, scaled_height, GDK_INTERP_HYPER);  // Do the scaling at best possible quality
					if (NULL == resized_pixbuf)
					{
						// Something went wrong when creating the dictionary shape for this layer
						display_warning("Error ED93: Something went wrong when creating the dictionary shape for the mouse cursor");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;

					}

					// Convert the scaled mouse cursor into jpeg data
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
						// Something went wrong when encoding the mouse cursor to jpeg format
						display_warning("Error ED94: Something went wrong when encoding a mouse cursor to jpeg format");

						// Free the memory allocated in this function
						g_error_free(error);
						if (NULL != resized_pixbuf)
							g_object_unref(resized_pixbuf);

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;
					}

					// Create the dictionary shape from the jpeg data
					image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
					image_bitmap = newSWFJpegBitmap_fromInput(image_input);

					// Store the dictionary shape for future reference
					this_layer_data->dictionary_shape = (SWFBlock) image_bitmap;

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				case TYPE_TEXT:

					// We're processing a text layer

					// * Create the text object *

					// Create the text object we'll be using
					text_object = newSWFText();

					// Assign a font to the text object
					SWFText_setFont(text_object, font_object);

					// Set the height we want for the text
					gfloat scaled_font_size = scaled_height_ratio * ((layer_text *) this_layer_data->object_data)->font_size;
					SWFText_setHeight(text_object, scaled_font_size);

					// Set the foreground color for the text
					guint16 red_component = ((layer_text *) this_layer_data->object_data)->text_color.red;
					guint16 green_component = ((layer_text *) this_layer_data->object_data)->text_color.green;
					guint16 blue_component = ((layer_text *) this_layer_data->object_data)->text_color.blue;
					SWFText_setColor(text_object, roundf(red_component / 255), roundf(green_component / 255), roundf(blue_component / 255), 0xff);

					// Add the required text to the text object
					SWFText_addString(text_object, "some text", NULL);

					// * Create the background for the text object *
					text_bg = newSWFShape();
					if (NULL == text_bg)
					{
						// Something went wrong when creating the empty shape, so we skip this layer
						display_warning("Error ED101: Something went wrong when creating a text layer background for swf output");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;
					}

					// Set the solid fill for the text background box
					SWFShape_setRightFillStyle(text_bg, text_bg_fill_style);

					// Set the line style
					SWFShape_setLine(text_bg, 2, 0x00, 0x00, 0x00, 0xff);  // Width = 2 seems to work ok

					// Work out the scaled dimensions of the text background box
					text_bg_box_height = roundf(scaled_height_ratio * scaled_font_size);
					text_bg_box_width = roundf(scaled_width_ratio * ((gfloat) SWFText_getStringWidth(text_object, "some text") + 10));

					// Move the start position of the text box vertically downwards
					SWFShape_movePenTo(text_bg, -(5 * scaled_height_ratio), (5 * scaled_height_ratio) - text_bg_box_height);

					// Create the text background box
					SWFShape_drawLine(text_bg, text_bg_box_width, 0.0);
					SWFShape_drawLine(text_bg, 0.0, text_bg_box_height);
					SWFShape_drawLine(text_bg, -(text_bg_box_width), 0.0);
					SWFShape_drawLine(text_bg, 0.0, -(text_bg_box_height));

					// * Create the swf movie clip object that holds the text background and text *
					text_movie_clip = newSWFMovieClip();

					// Add the text background to the movie clip
					SWFMovieClip_add(text_movie_clip, (SWFBlock) text_bg);

					// Add the text object to the movie clip
					SWFMovieClip_add(text_movie_clip, (SWFBlock) text_object);

					// Advance the movie clip one frame, else it won't be displayed
					SWFMovieClip_nextFrame(text_movie_clip);

					// Store the dictionary shape for future reference
					this_layer_data->dictionary_shape = (SWFBlock) text_movie_clip;

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				default:
					// Unknown type
					display_warning("Error ED83: Unknown layer type in swf output");
					break;
			}

			// If the creation of the dictionary shape worked, we add this layer to the list for processing
			if (TRUE == dictionary_shape_ok)
			{
				// * Process the element array, setting flags and info as required for this layer *

				// Calculate the scaled start and finish positions for each element
				element_x_position_start = roundf(scaled_width_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->x_offset_start);
				element_x_position_finish = roundf(scaled_width_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->x_offset_finish);
				element_y_position_start = roundf(scaled_height_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->y_offset_start);
				element_y_position_finish = roundf(scaled_height_ratio * (gfloat) ((layer_highlight *) this_layer_data->object_data)->y_offset_finish);

				// If the layer moves, flag this and calculate the increment in each direction
				if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
				{
					swf_timing_array[(layer_counter * (slide_duration + 1)) + this_layer_data->start_frame].is_moving = TRUE;
					element_x_position_increment = (((layer_highlight *) this_layer_data->object_data)->x_offset_finish - ((layer_highlight *) this_layer_data->object_data)->x_offset_start) / (gfloat) (this_layer_data->finish_frame - this_layer_data->start_frame);
					element_y_position_increment = (((layer_highlight *) this_layer_data->object_data)->y_offset_finish - ((layer_highlight *) this_layer_data->object_data)->y_offset_start) / (gfloat) (this_layer_data->finish_frame - this_layer_data->start_frame);
				}

				// Indicate on which frame the element should be displayed
				frame_number = (layer_counter * (slide_duration + 1)) + this_layer_data->start_frame;
				swf_timing_array[frame_number].add = TRUE;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting ADD value for layer %u in swf element %u\n", layer_counter, frame_number);
				}

				// Indicate on which frame the element should be removed from display
				frame_number = (layer_counter * (slide_duration + 1)) + this_layer_data->finish_frame;
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
					element_number = (layer_counter * (slide_duration + 1)) + frame_number;
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
					if (TRUE == swf_timing_array[(layer_counter * (slide_duration + 1)) + this_layer_data->start_frame].is_moving)
					{
						this_frame_ptr->is_moving = TRUE;
						this_frame_ptr->x_position = roundf(element_x_position_start + (position_counter * element_x_position_increment));
						this_frame_ptr->y_position = roundf(element_y_position_start + (position_counter * element_y_position_increment));
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
			for (frame_counter = 0; frame_counter <= slide_duration; frame_counter++)  // This loops _frame + 1_ number of times
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
		for (frame_counter = 0; frame_counter <= slide_duration; frame_counter++)  // This loops _frames + 1_ number of times
		{
			for (layer_counter = 0; layer_counter < num_layers; layer_counter++)  // This loops _num_layers_ of times
			{
				// For each frame, access all of the layers then move to the next frame
				frame_number = (layer_counter * (slide_duration + 1)) + frame_counter;
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
						display_list_object = SWFMovie_add(swf_movie, (SWFBlock) this_layer_info->dictionary_shape);

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
			SWFMovie_nextFrame(swf_movie);
		}
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
		g_free(font_pathname);

		// Indicate something went wrong when saving the swf
		return FALSE;
	}

	// Free the memory allocated in this function
	destroySWFMovie(swf_movie);
	destroySWFFillStyle(highlight_fill_style);
	destroySWFFillStyle(text_bg_fill_style);
	g_free(font_pathname);

	// Indicate that the swf was created successfully
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.29  2008/01/19 22:56:54  vapour
 * Text and it's background are now added to a movie clip for swf output.  Scaling not working very well yet though.
 *
 * Revision 1.28  2008/01/19 06:52:19  vapour
 *  + Moved initial swf movie creation and saving code in here.
 *  + Initial working swf text generation code in place.  Still needs more work though.
 *  + Many tweaks to existing code.
 *
 * Revision 1.27  2008/01/16 12:49:11  vapour
 * + Images now have swf code created for them (scaled and all).
 * + Highlight boxes are now scaled.
 * + Slide duration is taken from the slide properties, rather than manually calculated.
 *
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

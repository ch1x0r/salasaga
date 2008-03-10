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

// GTK include
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_export_flash_control_bar.h"


gint menu_export_flash_inner(gchar *output_filename)
{
	// Local variables
	GString				*as_gstring;				// Used for constructing action script statements
	gboolean			dictionary_shape_ok;		// Temporary value indicating if a dictionary shape was created ok or not
	gint				display_depth;				// The depth at which an item is displayed in the swf output
	GError				*error = NULL;				// Pointer to error return structure
	gfloat				finish_x_position_unscaled;
	gfloat				finish_y_position_unscaled;
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
	guint				out_res_index;				// Index into the array of output resolution entries 
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	guint				position_counter;			// Temporary counter integer
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	guint				slide_counter;				// Holds the number of slides
	guint				slide_duration;				// Holds the total number of frames in this slide
	GString				*slide_name_tmp;			// Temporary slide names are constructed with this
	gfloat				start_x_position_unscaled;
	gfloat				start_y_position_unscaled;
	swf_frame_element	*swf_timing_array = NULL;	// Used to coordinate the actions in each frame
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	layer				*this_layer_data;			// Points to the data in the present layer
	layer				*this_layer_info;			// Used to point to layer data when looping
	slide				*this_slide_data;			// Points to the data in the present slide
	guint				total_frames;				// The total number of frames in the animation
	gboolean			unknown_resolution;

	SWFDisplayItem		display_list_object;		// Temporary display list object
	SWFAction			end_action;					// The actionscript for the end behaviour
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFFont				font_object;				// The font we use gets loaded into this
	SWFShape			highlight_box;				// Temporary swf shape used when constructing highlight boxes
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	SWFInput			image_input;				// Used to hold a swf input object
	SWFShape			image_shape;				// Used to hold a swf shape object
	SWFAction			inc_slide_counter_action;	// Swf action object used to run some action script
	SWFAction			swf_action;					// Used when constructing action script
	SWFButton			swf_button;					// Holds a swf button
	SWFButtonRecord		swf_button_record;			// Holds a swf button record
	SWFMovie			swf_movie;					// Swf movie object

	SWFShape			text_bg;					// The text background shape goes in this
	gfloat				text_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				text_bg_box_width;			// Used while generating swf output for text boxes
	SWFDisplayItem		text_bg_display_item;
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	layer_text			*text_data;					// Points to the text object data inside the layer
	SWFDisplayItem		text_display_item;
	gfloat				text_leading;				// Spacing to use at the edges of the font
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text object we're working on goes in this
	gfloat				text_real_font_size;

	guint16				blue_component;				// Used when retrieving the foreground color of text
	gfloat				current_ming_scale;			// Used when creating text swf output
	guint16				green_component;			// Used when retrieving the foreground color of text
	gint				num_text_lines;				// Number of text lines in a particular text layer
	guint16				red_component;				// Used when retrieving the foreground color of text
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	GtkTextIter			text_end;					// End position of text buffer
	gint				text_lines_counter;			// Counter used when processing text
	GtkTextIter			text_start;					// Start position of text buffer
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gchar				*visible_string;			// Text string is retrieved into this
	gfloat				widest_text_string_width;	// Used when calculating how wide to draw the text background box

	guint				element_number;
	guint				frame_number;

	gfloat				element_x_position_finish = 0;
	gfloat				element_x_position_increment = 0;
	gfloat				element_x_position_start = 0;
	gfloat				element_y_position_finish = 0;
	gfloat				element_y_position_increment = 0;
	gfloat				element_y_position_start = 0;

	// (Hopefully) temporary variables put in place to get around a *bizarre*
	// problem whereby calculating basic stuff like "0 - output_width" gives bogus results (computers are infallible eh?) :(
	// (Suspect it's caused by some optimisation at compile time going wrong)
	gfloat				layer_down;
	gfloat				layer_left;
	gfloat				layer_right;
	gfloat				layer_up;


	// Initialise variables
	as_gstring = g_string_new(NULL);
	slide_name_tmp = g_string_new(NULL);
	total_frames = 0;

	// Determine which of the control bar resolutions to use
	unknown_resolution = FALSE;
	switch (output_width)
	{
		case 1920:
			
			if (1200 == output_height)
			{
				out_res_index = 0;
				break;
			}
			if (1080 == output_height)
			{
				out_res_index = 1;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 1600:

			if (1200 == output_height)
			{
				out_res_index = 2;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 1280:
			
			if (1024 == output_height)
			{
				out_res_index = 3;
				break;
			}
			if (720 == output_height)
			{
				out_res_index = 4;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 1024:

			if (768 == output_height)
			{
				out_res_index = 5;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 800:

			if (600 == output_height)
			{
				out_res_index = 6;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 720:

			if (480 == output_height)
			{
				out_res_index = 7;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 640:

			if (480 == output_height)
			{
				out_res_index = 8;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 352:

			if (288 == output_height)
			{
				out_res_index = 9;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 320:

			if (240 == output_height)
			{
				out_res_index = 10;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 176:

			if (144 == output_height)
			{
				out_res_index = 11;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 160:

			if (120 == output_height)
			{
				out_res_index = 12;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		case 128:

			if (96 == output_height)
			{
				out_res_index = 13;
				break;
			}

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;

		default:

			// We're using an unknown output resolution
			unknown_resolution = TRUE;
			break;
	}

	// If an unknown output resolution is given, indicate an error and return
	if (TRUE == unknown_resolution)
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
	if (debug_level) printf("Full path name to font file is: %s\n", font_pathname);
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

	// Add the swf control bar to the movie
	return_code_bool = menu_export_flash_control_bar(swf_movie, out_res_index);
	if (TRUE != return_code_bool)
	{
		// Something went wrong when adding the control bar to the movie

		// Free the memory allocated in this function
		destroySWFMovie(swf_movie);
		destroySWFFillStyle(highlight_fill_style);
		destroySWFFillStyle(text_bg_fill_style);
		g_free(font_pathname);

		return FALSE;
	}

// For now, this uses a two pass per slide approach
//	1st pass gathers timing info, and creates an array of timing information for each slide
//		The array has one element for each frame, times each layer of the slide (i.e. num elements = num frames x num layers)
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
		slides = g_list_first(slides);
		this_slide_data = g_list_nth_data(slides, slide_counter);
		slide_duration = this_slide_data->duration;

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

		// Add the frames for this slide to the total count of frames for the animation
		total_frames += slide_duration;

		// Create an array that's layers x "number of frames in the slide"
		frame_number = num_layers * (slide_duration + 1);  // +1 because if (ie.) we say slide 5, then we really mean the 6th slide (we start from 0)
		swf_timing_array = g_try_new0(swf_frame_element, frame_number); 

		// Point to the first layer again
		this_slide_data = g_list_nth_data(slides, slide_counter);

		// (Re-)Initialise the depth at which the next layer will be displayed
		display_depth = num_layers;

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		this_slide_data->layers = g_list_first(this_slide_data->layers);
		for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
		{
			this_layer_data = g_list_nth_data(this_slide_data->layers, layer_counter);

			// If this layer is not visible, then we skip it
			if (FALSE == this_layer_data->visible)
				continue;

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
						printf("Image width: %d\n", image_width);
						printf("Scaled width: %d\n", scaled_width);
						printf("Image height: %d\n", image_height);
						printf("Scaled height: %d\n", scaled_height);
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

					// Convert the compressed image into image data
					return_code_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(resized_pixbuf),
									&pixbuf_buffer,  // Will come back filled out with location of image data
									&pixbuf_size,  // Will come back filled out with size of image data
									"png",
									&error,
									NULL);
					if (FALSE == return_code_bool)
					{
						// Something went wrong when encoding the image to required format
						display_warning("Error ED92: Something went wrong when encoding an image to required format");

						// Free the memory allocated in this function
						g_error_free(error);
						if (NULL != resized_pixbuf)
							g_object_unref(resized_pixbuf);

						break;
					}

					// Turn the image data into a swf bitmap
					image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
					image_bitmap = newSWFBitmap_fromInput(image_input);

					// If this layer has an external link associated with it, turn it into a button
					if (0 < this_layer_data->external_link->len)
					{
						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This image has an external link: '%s'\n", this_layer_data->external_link->str);
						}

						// Turn the swf image into a swf shape 
						image_shape = newSWFShapeFromBitmap(image_bitmap, SWFFILL_CLIPPED_BITMAP);
						if (NULL == image_shape)
						{
							// Something went wrong when encoding the image to required format
							display_warning("Error ED109: Something went wrong converting an image to a swf shape object");
						
							// Free the memory allocated in this function
							g_error_free(error);
							if (NULL != resized_pixbuf)
								g_object_unref(resized_pixbuf);
							destroySWFBitmap(image_bitmap);
							destroySWFInput(image_input);
						
							break;
						}

						// Create an empty button object we can use
						swf_button = newSWFButton();

						// Add the shape to the button for all of its states
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) image_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

						// Add action script to the button, jumping to the external link
						g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
						swf_action = newSWFAction(as_gstring->str);
						SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) swf_button;
					} else
					{
						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) image_bitmap;
					}

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				case TYPE_EMPTY:

					// * We're processing an empty layer *

					// Create the initial empty shape
					empty_layer_shape = newSWFShape();
					if (NULL == highlight_box)
					{
						// Something went wrong when creating the empty shape, so we skip this layer
						display_warning("Error ED111: Something went wrong when creating an empty layer for swf output");

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;
					}

					// Create the required fill color for the empty layer
					red_component = ((layer_empty *) this_layer_data->object_data)->bg_color.red;
					green_component = ((layer_empty *) this_layer_data->object_data)->bg_color.green;
					blue_component = ((layer_empty *) this_layer_data->object_data)->bg_color.blue;
					empty_layer_fill = SWFShape_addSolidFillStyle(empty_layer_shape, red_component / 255, green_component / 255, blue_component / 255, 0xff);  // Alpha value - solid fill
					SWFShape_setRightFillStyle(empty_layer_shape, empty_layer_fill);

					// Set the line style
					SWFShape_setLine(empty_layer_shape,
							1,  // Line width
							red_component / 255, green_component / 255, blue_component / 255, 0xff);  // Alpha value - solid fill

					// Create the empty layer object
					layer_right = output_width;
					layer_down = output_height;
					layer_left = 0 - layer_right;  // This is bizarre.  Should NOT have to do this to get a correct result. :(
					layer_up = 0 - layer_down;  // This is bizarre.  Should NOT have to do this to get a correct result. :(
					SWFShape_movePenTo(empty_layer_shape, 0.0, 0.0);
					SWFShape_drawLine(empty_layer_shape, layer_right, 0.0);
					SWFShape_drawLine(empty_layer_shape, 0.0, layer_down);
					SWFShape_drawLine(empty_layer_shape, layer_left, 0.0);
					SWFShape_drawLine(empty_layer_shape, 0.0, layer_up);

					// If this layer has an external link associated with it, turn it into a button
					if (0 < this_layer_data->external_link->len)
					{
						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This empty layer has an external link: '%s'\n", this_layer_data->external_link->str);
						}

						// Create an empty button object we can use
						swf_button = newSWFButton();

						// Add the shape to the button for all of its states
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) empty_layer_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

						// Add action script to the button, jumping to the external link
						g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
						swf_action = newSWFAction(as_gstring->str);
						SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) swf_button;
					} else
					{
						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) empty_layer_shape;
					}

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

					// If this layer has an external link associated with it, turn it into a button
					if (0 < this_layer_data->external_link->len)
					{
						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This highlight has an external link: '%s'\n", this_layer_data->external_link->str);
						}

						// Create an empty button object we can use
						swf_button = newSWFButton();

						// Add the shape to the button for all of its states
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) highlight_box, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

						// Add action script to the button, jumping to the external link
						g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
						swf_action = newSWFAction(as_gstring->str);
						SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) swf_button;
					} else
					{
						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) highlight_box;
					}

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

					// Convert the scaled mouse cursor into image data
					return_code_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(resized_pixbuf),
									&pixbuf_buffer,  // Will come back filled out with location of image data
									&pixbuf_size,  // Will come back filled out with size of image data
									"png",
									&error,
									NULL);
					if (FALSE == return_code_bool)
					{
						// Something went wrong when encoding the mouse cursor to image format
						display_warning("Error ED94: Something went wrong when encoding a mouse cursor to required format");

						// Free the memory allocated in this function
						g_error_free(error);
						if (NULL != resized_pixbuf)
							g_object_unref(resized_pixbuf);

						// * We don't want to process this layer any further, so we skip adding it to the dictionary
						break;
					}

					// Turn the image data into a swf bitmap
					image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
					image_bitmap = newSWFBitmap_fromInput(image_input);

					// If this layer has an external link associated with it, turn it into a button
					if (0 < this_layer_data->external_link->len)
					{
						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This mouse cursor layer has an external link: '%s'\n", this_layer_data->external_link->str);
						}

						// Turn the swf image into a swf shape 
						image_shape = newSWFShapeFromBitmap(image_bitmap, SWFFILL_CLIPPED_BITMAP);
						if (NULL == image_shape)
						{
							// Something went wrong when encoding the image to required format
							display_warning("Error ED110: Something went wrong converting an image to a swf shape object");
						
							// Free the memory allocated in this function
							g_error_free(error);
							if (NULL != resized_pixbuf)
								g_object_unref(resized_pixbuf);
							destroySWFBitmap(image_bitmap);
							destroySWFInput(image_input);
						
							break;
						}

						// Create an empty button object we can use
						swf_button = newSWFButton();

						// Add the shape to the button for all of its states
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) image_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

						// Add action script to the button, jumping to the external link
						g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
						swf_action = newSWFAction(as_gstring->str);
						SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) swf_button;
					} else
					{
						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) image_bitmap;
					}

					// Indicate that the dictionary shape for this layer was created ok
					dictionary_shape_ok = TRUE;

					break;

				case TYPE_TEXT:

					// We're processing a text layer

					// Simplify pointers and work out element positioning info
					text_data = (layer_text *) this_layer_data->object_data;

					// * Create the text object *

					// Create the text object we'll be using
					text_object = newSWFText();

					// Assign a font to the text object
					SWFText_setFont(text_object, font_object);

					// Set the height we want for the text
					scaled_font_size = scaled_height_ratio * text_data->font_size;
					SWFText_setHeight(text_object, scaled_font_size);
					text_real_font_size = SWFText_getAscent(text_object) + SWFText_getDescent(text_object);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Scaled font size: '%.2f'\n", scaled_font_size);
						printf("Real font size: '%.2f'\n", text_real_font_size);
						printf("'Leading' for this text is: %.2f\n", SWFText_getLeading(text_object));
						printf("'Ascent' for this text is: %.2f\n", SWFText_getAscent(text_object));
						printf("'Descent' for this text is: %.2f\n", SWFText_getDescent(text_object));
					}

					// Set the foreground color for the text
					red_component = text_data->text_color.red;
					green_component = text_data->text_color.green;
					blue_component = text_data->text_color.blue;
					SWFText_setColor(text_object, roundf(red_component / 255), roundf(green_component / 255), roundf(blue_component / 255), 0xff);

					// Work out how many lines of text we're dealing with
					num_text_lines = gtk_text_buffer_get_line_count(text_data->text_buffer);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Number of lines of text: %d\n", num_text_lines);
					}

					// Add each line of text to the output
					widest_text_string_width = 0;
					for (text_lines_counter = 0; text_lines_counter < num_text_lines; text_lines_counter++)
					{
						// Select the start and end positions for the given line, in the text buffer
						gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(text_data->text_buffer), &text_start, text_lines_counter);
						text_end = text_start;
						gtk_text_iter_forward_to_line_end(&text_end);

						// Retrieve the text for the given line, and add it to the text object
						visible_string = gtk_text_iter_get_visible_text(&text_start, &text_end);
						SWFText_addString(text_object, visible_string, NULL);

						// * We need to know which of the strings is widest, so we can calculate the width of the text background box *

						// If this is the widest string, we keep the value of this one
						this_text_string_width = SWFText_getStringWidth(text_object, (guchar *) visible_string);
						if (this_text_string_width > widest_text_string_width)
							widest_text_string_width = this_text_string_width;

						// * Move the pen down to the start of the next line *

						// Move to the appropriate Y position
						SWFText_moveTo(text_object, 0, (text_lines_counter + 1) * text_real_font_size);

						// Try and move X as close as possible to 0.  We can't use 0 in SWFText_moveTo() due to a bug in Ming
						current_ming_scale = Ming_getScale();
						Ming_setScale(1);
						SWFText_moveTo(text_object, 1, 0);
						Ming_setScale(current_ming_scale);

						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("Line %d of %d: %s\n", text_lines_counter, num_text_lines, gtk_text_iter_get_visible_text(&text_start, &text_end));
							printf("Width of this string: %.2f\n", this_text_string_width);
							printf("Width of widest string thus far: %.2f\n", widest_text_string_width);
						}
					}

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
					SWFShape_setLine(text_bg, 1, 0x00, 0x00, 0x00, 0xff);  // Width = 1 seems to work ok

					// Work out the scaled dimensions of the text background box
					text_leading = SWFText_getLeading(text_object);
					text_bg_box_height = (text_real_font_size * num_text_lines) * 1.02;
					text_bg_box_width = widest_text_string_width + (text_leading * 2);

					// Create the text background box
					SWFShape_drawLine(text_bg, text_bg_box_width, 0.0);
					SWFShape_drawLine(text_bg, 0.0, text_bg_box_height);
					SWFShape_drawLine(text_bg, -(text_bg_box_width), 0.0);
					SWFShape_drawLine(text_bg, 0.0, -(text_bg_box_height));

					// * Create the swf movie clip object that holds the text background and text *
					text_movie_clip = newSWFMovieClip();

					// Add the text background to the movie clip
					text_bg_display_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text_bg);

					// Add the text object to the movie clip
					text_display_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text_object);

					// Position the background and text elements
					SWFDisplayItem_moveTo(text_bg_display_item, 0.0, 0.0);
					SWFDisplayItem_moveTo(text_display_item, text_leading, SWFText_getAscent(text_object));

					// Advance the movie clip one frame, else it won't be displayed
					SWFMovieClip_nextFrame(text_movie_clip);

					// If this layer has an external link associated with it, turn it into a button
					if (0 < this_layer_data->external_link->len)
					{
						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This text has an external link: '%s'\n", this_layer_data->external_link->str);
						}

						// Create an empty button object we can use
						swf_button = newSWFButton();

						// Add the shape to the button for all of its states, excluding the hit state
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) text_movie_clip, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN);

						// Use the text background area as the hit state
						swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) text_bg, SWFBUTTON_HIT);

						// Add action script to the button, jumping to the external link
						g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
						swf_action = newSWFAction(as_gstring->str);
						SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) swf_button;
					} else
					{
						// Store the dictionary shape for future reference
						this_layer_data->dictionary_shape = (SWFBlock) text_movie_clip;
					}

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
				start_x_position_unscaled = this_layer_data->x_offset_start;
				start_y_position_unscaled = this_layer_data->y_offset_start;
				finish_x_position_unscaled = this_layer_data->x_offset_finish;
				finish_y_position_unscaled = this_layer_data->y_offset_finish;
				guint start_frame = this_layer_data->start_frame;
				guint finish_frame = this_layer_data->finish_frame;
				guint num_displayed_frames = (finish_frame - start_frame) + 1;
				element_x_position_start = scaled_width_ratio * start_x_position_unscaled;
				element_x_position_finish = scaled_width_ratio * finish_x_position_unscaled;
				element_y_position_start = scaled_height_ratio * start_y_position_unscaled;
				element_y_position_finish = scaled_height_ratio * finish_y_position_unscaled;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Dict. shape OK. Start frame: %u\tFinish frame: %u\t # of displayed frames: %u\n", start_frame, finish_frame, num_displayed_frames);
					printf("Start X pos (unscaled): %.2f\tStart Y pos (unscaled): %.2f\tFinish X pos (unscaled): %.2f\tFinish Y pos (unscaled): %.2f\n", start_x_position_unscaled, start_y_position_unscaled, finish_x_position_unscaled, finish_y_position_unscaled);
				}

				// If the layer moves, flag this and calculate the increment in each direction
				if ((element_x_position_start != element_x_position_finish) || (element_y_position_start != element_y_position_finish))
				{
					swf_timing_array[(layer_counter * (slide_duration + 1)) + start_frame].is_moving = TRUE;
					element_x_position_increment = (element_x_position_finish - element_x_position_start) / (num_displayed_frames - 1);
					element_y_position_increment = (element_y_position_finish - element_y_position_start) / (num_displayed_frames - 1);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("This shape moves! X increment: %.2f\tY increment: %.2f\n", element_x_position_increment, element_y_position_increment);
					}
				}

				// Indicate on which frame the element should be displayed, at what display depth, and its co-ordinates
				frame_number = (layer_counter * (slide_duration + 1)) + start_frame;
				swf_timing_array[frame_number].add = TRUE;
				swf_timing_array[frame_number].depth = display_depth;
				swf_timing_array[frame_number].x_position = element_x_position_start;
				swf_timing_array[frame_number].y_position = element_y_position_start;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting ADD value for layer %u in swf element %u, with display depth of %u\n", layer_counter, frame_number, display_depth);
				}

				// Indicate on which frame the element should be removed from display
				frame_number = (layer_counter * (slide_duration + 1)) + finish_frame;
				swf_timing_array[frame_number].remove = TRUE;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting REMOVE value for layer %u in swf element %u\n", layer_counter, frame_number);
				}

				// Re-initialise the position counter for each shape
				position_counter = 0;
				for (frame_number = start_frame; frame_number <= finish_frame; frame_number++)
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
					if (TRUE == swf_timing_array[(layer_counter * (slide_duration + 1)) + start_frame].is_moving)
					{
						this_frame_ptr->is_moving = TRUE;
						this_frame_ptr->x_position = element_x_position_start + (element_x_position_increment * position_counter);
						this_frame_ptr->y_position = element_y_position_start + (element_y_position_increment * position_counter);
						position_counter++;

						// Display debugging info if requested
						if (debug_level)
						{
							printf("Scaled X position: %.2f\tScaled Y position: %.2f\n", this_frame_ptr->x_position, this_frame_ptr->y_position);
						}
					}

					// Store the opacity setting for each frame
					// fixme2: Still need to calculate properly rather than hard code to 100% for the moment
					this_frame_ptr->opacity = 65535;
				}
			}

			// Decrement the depth at which this element will be displayed
			display_depth--;
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

						// Ensure the object is at the correct display depth
						SWFDisplayItem_setDepth(display_list_object, this_frame_ptr->depth);

						// Position the object
						SWFDisplayItem_moveTo(display_list_object, this_frame_ptr->x_position, this_frame_ptr->y_position);
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
			end_action = newSWFAction("_root.playing = false; cb_main.cb_play._visible = true; _root.gotoAndPlay(1);");
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
		g_free(font_pathname);

		// Indicate something went wrong when saving the swf
		return FALSE;
	}

	// Free the memory allocated in this function
	destroySWFMovie(swf_movie);
	destroySWFFillStyle(highlight_fill_style);
	destroySWFFillStyle(text_bg_fill_style);
	destroySWFFont(font_object);
	g_free(font_pathname);

	// Indicate that the swf was created successfully
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.65  2008/03/10 06:37:58  vapour
 * Updated the actionscript to work with the new swf start behaviour project preference.
 *
 * Revision 1.64  2008/03/09 14:54:26  vapour
 * Renamed the enums for end behaviour.
 *
 * Revision 1.63  2008/03/06 00:18:40  vapour
 * Added code to action the new end behaviour project preference.
 *
 * Revision 1.62  2008/03/03 08:20:22  vapour
 * Replaced text element positioning system with new working version.  THAT took a lot of effort. :-/
 *
 * Revision 1.61  2008/03/01 10:37:57  vapour
 * Added reasonable text element position information for the remaining swf output resolutions.
 *
 * Revision 1.60  2008/03/01 10:26:44  vapour
 * Added reasonable text element position information for 1280 x 720 and 1280 x 1024 swf output.
 *
 * Revision 1.59  2008/03/01 10:09:19  vapour
 * Added text element position information for 160 x 120 and 128 x 96 swf output.
 *
 * Revision 1.58  2008/03/01 09:47:43  vapour
 * Added text element position information for 176 x 144 swf output.
 *
 * Revision 1.57  2008/03/01 09:19:02  vapour
 * Added reasonable working text element position information for 320 x 240 swf output.
 *
 * Revision 1.56  2008/03/01 09:11:36  vapour
 * Added reasonable working text element position information for 352 x 288 swf output.
 *
 * Revision 1.55  2008/03/01 08:22:48  vapour
 * Added reasonable working text element position information for 640 x 480 swf output.
 *
 * Revision 1.54  2008/03/01 08:17:32  vapour
 * Added reasonable working text element position information for 720 x 480 swf output.
 *
 * Revision 1.53  2008/03/01 07:58:58  vapour
 * Added reasonable working text element position information for 1024 x 768 swf output.
 *
 * Revision 1.52  2008/03/01 01:59:12  vapour
 * Updated code for better text positioning over a wider range of font sizes (10 pt to 80 pt).  800 x 600 resolution seems ok for now.
 *
 * Revision 1.51  2008/02/29 04:08:54  vapour
 * Updated to use the text element positioning information, and included decent positioning information for 800 x 600 swf output.
 *
 * Revision 1.50  2008/02/28 16:55:44  vapour
 *  + Removed Ming initialisation from this function.
 *  + Forced swf output resolution to 800 x 600 for now, while getting text element placement to work correctly.
 *
 * Revision 1.49  2008/02/28 05:41:32  vapour
 * Moved selection of the output resolution array index up a function level for swf output.  This is so the same approach can be used for text element positioning in swf output.
 *
 * Revision 1.48  2008/02/27 01:50:41  vapour
 * Updated for the new definition of the menu export flash inner function.
 *
 * Revision 1.47  2008/02/12 14:15:29  vapour
 * Updated to ignore non-visible layers.
 *
 * Revision 1.46  2008/02/12 05:24:32  vapour
 * Adjusted to work with the new, slightly simplified layer structure.
 *
 * Revision 1.45  2008/02/03 05:09:53  vapour
 * Adjusted swf output frame elements to use floating point references instead of integer references, in order to address a stuttering-playback-of-swf-elements bug reported by Bobby Powers.
 *
 * Revision 1.44  2008/02/01 10:48:42  vapour
 *  + Added working code to handle empty layers.
 *  + Added code to create buttons for layers with an external link.
 *
 * Revision 1.43  2008/01/31 01:48:36  vapour
 * Improved gap calculation between text and its background in swf output.
 *
 * Revision 1.42  2008/01/31 01:34:55  vapour
 * Improved the calculation of how much gap to leave between text and the edge of its background box, so letter going down low don't hit the edge of the box.
 *
 * Revision 1.41  2008/01/31 01:19:21  vapour
 * Adjusted the height of swf output text box background to be slightly taller, though this will need more work to handle certain characters better.
 *
 * Revision 1.40  2008/01/30 15:47:39  vapour
 *  + Added a variable to track whether the user is playing or pausing the swf movie, as it needs special handling to provide a good experience.
 *
 * Revision 1.39  2008/01/30 14:46:45  vapour
 *  + Added a variable to track whether the user is reversing through the swf movie or not, as it needs special handling to provide a good experience.
 *
 * Revision 1.38  2008/01/30 09:54:29  vapour
 *  + Reworked the action script embedded in swf output to include debugging statements, if the IDE was started in debugging mode. (-d)
 *
 * Revision 1.37  2008/01/27 17:41:26  vapour
 * Updated to explicitly refer to the _root scope for a swf variable.
 *
 * Revision 1.36  2008/01/25 16:13:11  vapour
 * Added code to embed frame labels in the output swf.
 *
 * Revision 1.35  2008/01/23 17:34:37  vapour
 * Adjusted internal image conversion functions to use png rather than jpeg, to be more sure that we're not losing image quality over time.
 *
 * Revision 1.34  2008/01/23 02:10:35  vapour
 * Updated to call the new swf output control bar creation function.
 *
 * Revision 1.33  2008/01/21 19:19:45  vapour
 * Display depths for swf objects are now explicitly set, which seems to help a lot.
 *
 * Revision 1.32  2008/01/20 13:21:15  vapour
 *  + Moved many references to local variables.
 *  + Fixed timing loop so objects positions are now correct.
 *  + Updated to load the jpeg image with newSWFBitmap_fromInput() instead, in case we change to using png format internally or something in future.
 *
 * Revision 1.31  2008/01/20 06:51:38  vapour
 * Text layer output to swf now works properly, including multi-line text.  Scales ok too.
 *
 * Revision 1.30  2008/01/20 04:46:07  vapour
 * Scaling now works properly for text layers.
 *
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

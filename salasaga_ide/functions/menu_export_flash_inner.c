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


// Turn on C99 compatibility - needed for roundf() to work
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
#include "menu_export_flash_control_bar.h"


gint menu_export_flash_inner(gchar *output_filename)
{
	// Local variables
	GString				*as_gstring;				// Used for constructing action script statements
	gboolean			dictionary_shape_ok;		// Temporary value indicating if a dictionary shape was created ok or not
	GError				*error = NULL;				// Pointer to error return structure
	guint				finish_frame;
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	guint				frame_counter;				// Holds the number of frames
	guint				new_opacity;				// Used when constructing the opacity value for an object
	gint				num_bytes_written;			// Receives the number of bytes written to disk for the swf output
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	gint				image_height;				// Temporarily used to store the height of an image
	gint				image_width;				// Temporarily used to store the width of an image
	GString				*initial_action_gstring;	// Swf action script can be constructed with this
	guint				layer_counter;				// Holds the number of layers
	guint				num_displayed_frames;
	guint				num_layers = 0;				// The number of layers in the slide
	guint				num_slides;					// The number of slides in the movie
	guint				opacity_count;				// Used when calculating object opacity
	gfloat				opacity_step;				// Used when calculating object opacity
	guint				out_res_index;				// Index into the array of output resolution entries 
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to 
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	guint				slide_counter;				// Holds the number of slides
	guint				slide_duration;				// Holds the total number of frames in this slide
	GString				*slide_name_tmp;			// Temporary slide names are constructed with this
	guint				start_frame;
	swf_frame_element	*swf_timing_array = NULL;	// Used to coordinate the actions in each frame
	swf_frame_element 	*this_frame_ptr;			// Points to frame information when looping
	layer				*this_layer_data;			// Points to the data in the present layer
	layer				*this_layer_info;			// Used to point to layer data when looping
	slide				*this_slide_data;			// Points to the data in the present slide
	guint				total_frames;				// The total number of frames in the animation
	guint				total_num_layers;			// The total number of layers in the animation
	gfloat				total_seconds;				// The duration of the entire animation in seconds	
	gboolean			unknown_resolution;
	guint				x_position;					// Used in calculating layer object position
	guint				y_position;					// Used in calculating layer object position

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
	SWFAction			initial_action;				// Swf action object used to run some action script
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
	total_seconds = 0;
	initial_action_gstring = g_string_new(NULL);

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
	
			// Free the memory allocated in this function
			destroySWFMovie(swf_movie);
			destroySWFFillStyle(highlight_fill_style);
			destroySWFFillStyle(text_bg_fill_style);
			g_free(font_pathname);
	
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
		frame_number = num_layers * (slide_duration + 1);  // +1 because if (ie.) we say slide 5, then we really mean the 6th slide (we start from 0)
		swf_timing_array = g_try_new0(swf_frame_element, frame_number);
		if (NULL == swf_timing_array)
		{
			display_warning("Error ED339: We couldn't allocate enough memory to generate the swf.  Export aborted.");
			return FALSE;
		}

		// Process each layer in turn.  For every frame the layer is in, store in the array
		// whether the object in the layer is visible, it's position, transparency, etc
		this_slide_data = g_list_nth_data(slides, slide_counter);
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
						this_text_string_width = SWFText_getStringWidth(text_object, (gchar *) visible_string);
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
					swf_timing_array[frame_number].add = TRUE;
					swf_timing_array[frame_number].depth = total_num_layers;
					swf_timing_array[frame_number].x_position = element_x_position_start;
					swf_timing_array[frame_number].y_position = element_y_position_start;
					swf_timing_array[frame_number].action_this = TRUE;
					swf_timing_array[frame_number].object_name = g_string_new(NULL);
					g_string_printf(swf_timing_array[frame_number].object_name, "Object%d", total_num_layers);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Setting ADD value for layer %u in swf element %u, with display depth of %u\n", layer_counter, frame_number, total_num_layers);
					}

					// Work out how much opacity to increment each frame by
					opacity_step = 100 / (this_layer_data->transition_in_duration * frames_per_second);
					opacity_count = 0;

					// Loop through each frame of the fade in, setting the opacity values
					for (frame_counter = start_frame; frame_counter <= finish_frame; frame_counter++)
					{
						// Point to the desired element 
						element_number = (layer_counter * (slide_duration + 1)) + frame_counter;
						this_frame_ptr = &swf_timing_array[element_number];

						// Mark this element as needing action taken
						this_frame_ptr->action_this = TRUE;
						this_frame_ptr->opacity_change = TRUE;

						// Ensure the appropriate layer data can be found by later code
						this_frame_ptr->layer_info = this_layer_data;

						// Point to this object's display list item object name
						this_frame_ptr->object_name = swf_timing_array[frame_number].object_name;

						// Store the opacity value for this layer on this frame
						this_frame_ptr->opacity = opacity_count;
						opacity_count += floorf(opacity_step);
					}
				} else
				{
					// Indicate on which frame the element should be displayed, at what display depth, and its starting co-ordinates
					frame_number = (layer_counter * (slide_duration + 1)) + (this_layer_data->start_time * frames_per_second);
					swf_timing_array[frame_number].add = TRUE;
					swf_timing_array[frame_number].depth = total_num_layers;
					swf_timing_array[frame_number].x_position = element_x_position_start;
					swf_timing_array[frame_number].y_position = element_y_position_start;
					swf_timing_array[frame_number].action_this = TRUE;
					swf_timing_array[frame_number].object_name = g_string_new(NULL);
					g_string_printf(swf_timing_array[frame_number].object_name, "Object%d", total_num_layers);

					// Displaying debugging info if requested
					if (debug_level)
					{
						printf("Setting ADD value for layer %u in swf element %u, with display depth of %u\n", layer_counter, frame_number, total_num_layers);
					}
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
						this_frame_ptr = &swf_timing_array[element_number];

						// Mark this element as needing action taken
						this_frame_ptr->action_this = TRUE;
						this_frame_ptr->opacity_change = TRUE;

						// Ensure the appropriate layer data can be found by later code
						this_frame_ptr->layer_info = this_layer_data;

						// Point to this object's display list item object name
						this_frame_ptr->object_name = swf_timing_array[frame_number].object_name;

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

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Dict. shape OK. Start frame: %u\tFinish frame: %u\t # of displayed frames: %u\n", start_frame, finish_frame, num_displayed_frames);
				}

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
					this_frame_ptr = &swf_timing_array[element_number];

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

						// Displaying debugging info if requested
						if (debug_level)
						{
							printf("This shape moves! X increment: %.2f\tY increment: %.2f\n", element_x_position_increment, element_y_position_increment);
						}
					}

					// Store the opacity setting for each frame
					this_frame_ptr->opacity = 100;
				}
/*
				// Indicate on which frame the element should be removed from display
				frame_number = (layer_counter * (slide_duration + 1)) + finish_frame + 1;
				frame_number = (layer_counter * (slide_duration + 1)) + finish_frame;
				swf_timing_array[frame_number].remove = TRUE;

				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("Setting REMOVE value for layer %u in swf element %u\n", layer_counter, frame_number);
				}
*/
			}

			// Decrement the depth at which this element will be displayed
			total_num_layers--;
		}

SWFAction	opacity_action;
GString		*opacity_gstring;
			opacity_gstring = g_string_new(NULL);

		// Debugging output, displaying what we have in the pre-processing element array thus far
		if (3 == debug_level)
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
					printf("Processing slide %d, swf array element # %u\n", slide_counter, frame_number);
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

						// Ensure the object has it's name set, so we can reference it with action script
						SWFDisplayItem_setName(display_list_object, this_frame_ptr->object_name->str);

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
						// fixme3: We could possibly try setting the _visible properly of the object to false instead
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

					// Does the layer have an opacity change that needs to be actioned in this frame?
					if (TRUE == this_frame_ptr->opacity_change)
					{
						// Get the appropriate display list object
						display_list_object = this_layer_info->display_list_item;

						// Set the opacity level for the object
						new_opacity = this_frame_ptr->opacity;
						g_string_printf(opacity_gstring, "%s._alpha = %d;", this_frame_ptr->object_name->str, new_opacity);
						opacity_action = newSWFAction(opacity_gstring->str);
						SWFMovie_add(swf_movie, (SWFBlock) opacity_action);
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

/*
 * $Id$
 *
 * Salasaga: Creates a swf shape from a given layer
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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

// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"

gboolean menu_export_flash_create_shape(layer *this_layer_data)
{
	// Local variables
	GString				*as_gstring;				// Used for constructing action script statements
	guint16				blue_component;				// Used when retrieving the foreground color of text
	gfloat				current_ming_scale;			// Used when creating text swf output
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	SWFFont				font_object;				// The font we use gets loaded into this
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	GError				*error = NULL;				// Pointer to error return structure
	guint16				green_component;			// Used when retrieving the foreground color of text
	SWFShape			highlight_box;				// Temporary swf shape used when constructing highlight boxes
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	gint				image_height;				// Temporarily used to store the height of an image
	SWFInput			image_input;				// Used to hold a swf input object
	SWFShape			image_shape;				// Used to hold a swf shape object
	gint				image_width;				// Temporarily used to store the width of an image
	gint				num_text_lines;				// Number of text lines in a particular text layer
	SWFBlock			our_shape;					// The swf shape before it gets added to a swf movie clip
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	guint16				red_component;				// Used when retrieving the foreground color of text
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	SWFAction			swf_action;					// Used when constructing action script
	SWFButton			swf_button;					// Holds a swf button
	SWFButtonRecord		swf_button_record;			// Holds a swf button record
	SWFShape			text_bg;					// The text background shape goes in this
	gfloat				text_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				text_bg_box_width;			// Used while generating swf output for text boxes
	SWFDisplayItem		text_bg_display_item;
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	layer_text			*text_data;					// Points to the text object data inside the layer
	SWFDisplayItem		text_display_item;
	GtkTextIter			text_end;					// End position of text buffer
	gfloat				text_leading;				// Spacing to use at the edges of the font
	gint				text_lines_counter;			// Counter used when processing text
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text object we're working on goes in this
	gfloat				text_real_font_size;
	GtkTextIter			text_start;					// Start position of text buffer
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gchar				*visible_string;			// Text string is retrieved into this
	gfloat				widest_text_string_width;	// Used when calculating how wide to draw the text background box

	// (Hopefully) temporary variables put in place to get around a *bizarre*
	// problem whereby calculating basic stuff like "0 - output_width" gives bogus results (computers are infallible eh?) :(
	// (Suspect it's caused by some optimisation at compile time going wrong)
	gfloat				layer_down;
	gfloat				layer_left;
	gfloat				layer_right;
	gfloat				layer_up;


	// Create the fill style used in highlight boxes
	highlight_fill_style = newSWFSolidFillStyle(0x00, 0xff, 0x00, 0x40);
	if (NULL == highlight_fill_style)
	{
		// Something went wrong creating the fill style, so we don't proceed with creating the swf
		display_warning("Error ED97: Something went wrong when creating the highlight fill style for the swf");
		return FALSE;
	}

	// Create the fill style used in text background shapes
	text_bg_fill_style = newSWFSolidFillStyle(0xff, 0xff, 0xcc, 0xff);
	if (NULL == text_bg_fill_style)
	{
		// Something went wrong creating the fill style, so we don't proceed with creating the swf
		display_warning("Error ED102: Something went wrong when creating the text background fill style for the swf");

		// Free the memory allocated in this function
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
		destroySWFFillStyle(highlight_fill_style);
		destroySWFFillStyle(text_bg_fill_style);
		g_free(font_pathname);

		return FALSE;
	}

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) output_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) output_width / (gfloat) project_width;

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
				return FALSE;
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
					g_object_unref(GDK_PIXBUF(resized_pixbuf));
				return FALSE;
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
						g_object_unref(GDK_PIXBUF(resized_pixbuf));
					destroySWFBitmap(image_bitmap);
					destroySWFInput(image_input);
					return FALSE;
				}

				// Create an empty button object we can use
				swf_button = newSWFButton();

				// Add the shape to the button for all of its states
				swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) image_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = newSWFAction(as_gstring->str);
				SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) swf_button;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) image_bitmap;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_EMPTY:

			// * We're processing an empty layer *

			// Create the initial empty shape
			empty_layer_shape = newSWFShape();
			if (NULL == highlight_box)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED111: Something went wrong when creating an empty layer for swf output");
				return FALSE;
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

				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) swf_button;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) empty_layer_shape;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_HIGHLIGHT:

			// * We're processing a highlight layer *

			// Create the initial empty shape
			highlight_box = newSWFShape();
			if (NULL == highlight_box)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED98: Something went wrong when creating a highlight layer for swf output");
				return FALSE;
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

				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) swf_button;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) highlight_box;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_MOUSE_CURSOR:

			// * We're processing a mouse layer *

			// If we weren't able to load the mouse cursor graphic, we aren't going to be able to export it
			if (NULL == mouse_ptr_pixbuf)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED99: Couldn't load the mouse pointer image, for adding to the swf output");
				return FALSE;
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
				return FALSE;
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
					g_object_unref(GDK_PIXBUF(resized_pixbuf));
				return FALSE;
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
						g_object_unref(GDK_PIXBUF(resized_pixbuf));
					destroySWFBitmap(image_bitmap);
					destroySWFInput(image_input);
					return FALSE;
				}

				// Create an empty button object we can use
				swf_button = newSWFButton();

				// Add the shape to the button for all of its states
				swf_button_record = SWFButton_addCharacter(swf_button, (SWFCharacter) image_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = newSWFAction(as_gstring->str);
				SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) swf_button;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) image_bitmap;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

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
				return FALSE;
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

				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) swf_button;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				our_shape = (SWFBlock) text_movie_clip;
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) our_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		default:
			// Unknown type
			display_warning("Error ED83: Unknown layer type in swf output");
			return FALSE;
	}
}

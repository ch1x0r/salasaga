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
#include <stdio.h>

// GTK includes
#include <gtk/gtk.h>

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "../../display_warning.h"
#include "swf_add_mouse_click.h"


gboolean export_swf_create_shape(SWFMovie this_movie, layer *this_layer_data)
{
	// Local variables
	GString				*as_gstring = NULL;			// Used for constructing action script statements
	guint16				blue_component;				// Used when retrieving the foreground color of text
	SWFDisplayItem		container_display_item;
	SWFMovieClip		container_movie_clip;		// The movie clip that contains the layer object
	gfloat				current_ming_scale;			// Used when creating text swf output
	layer_empty			*empty_data;				// Points to the empty object data inside the layer
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	GError				*error = NULL;				// Pointer to error return structure
	guint				final_opacity;				// Used when calculating the final opacity figure for a highlight layer
	FILE				*font_file;					// The file we load the font from
	SWFFont				font_object;				// The font we use gets loaded into this
	gchar				*font_pathname;				// Full pathname to a font file to load is constructed in this
	guint16				green_component;			// Used when retrieving the foreground color of text
	SWFShape			highlight_box = NULL;		// Temporary swf shape used when constructing highlight boxes
	gint				highlight_box_height;		// Used while generating swf output for highlight boxes
	gint				highlight_box_width;		// Used while generating swf output for highlight boxes
	layer_highlight		*highlight_data;			// Points to the highlight object data inside the layer
	SWFFillStyle		highlight_fill_style;		// Fill style used when constructing highlight boxes
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	gint				image_height;				// Temporarily used to store the height of an image
	SWFInput			image_input;				// Used to hold a swf input object
	SWFShape			image_shape;				// Used to hold a swf shape object
	gint				image_width;				// Temporarily used to store the width of an image
	layer_mouse			*mouse_data;				// Points to the mouse object data inside the layer
	SWFFillStyle		mouse_fill_style;			// Fill style used when constructing mouse pointers
	SWFShape			mouse_shape = NULL;			// 
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
	SWFShape			text_bg = NULL;				// The text background shape goes in this
	gfloat				text_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				text_bg_box_width;			// Used while generating swf output for text boxes
	SWFDisplayItem		text_bg_display_item;
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	layer_text			*text_data;					// Points to the text object data inside the layer
	SWFDisplayItem		text_display_item;
	GtkTextIter			text_end;					// End position of text buffer
	gfloat				text_leading = 0.0;			// Spacing to use at the edges of the font
	gint				text_lines_counter;			// Counter used when processing text
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text object we're working on goes in this
	gfloat				text_real_font_size;
	GtkTextIter			text_start;					// Start position of text buffer
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gchar				*visible_string;			// Text string is retrieved into this
	gfloat				widest_text_string_width;	// Used when calculating how wide to draw the text background box


	// Initialisation
	as_gstring = g_string_new(NULL);

	// Calculate the height and width scaling values needed for this swf output
	scaled_height_ratio = (gfloat) output_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) output_width / (gfloat) project_width;

	// Create the font object we'll be using
	font_pathname = g_build_path(G_DIR_SEPARATOR_S, font_path, "fdb", "Bitstream Vera Sans.fdb", NULL);
	if (debug_level) printf("Full path name to font file is: %s\n", font_pathname);

	// Load the font file if needed
	font_file = fopen(font_pathname, "r");
	if (NULL == font_file)
	{
		// Something went wrong when loading the font file, so return
		display_warning("Error ED380: Something went wrong when opening the font file");

		return FALSE;
	}
	font_object = loadSWFFontFromFile(font_file);
	if (NULL == font_object)
	{
		// Something went wrong when loading the font file, so return
		display_warning("Error ED96: Something went wrong when loading the font file");

		// Free the memory allocated in this function
		g_free(font_pathname);

		return FALSE;
	}

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
				SWFButton_addShape(swf_button, (SWFCharacter) image_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = compileSWFActionCode(as_gstring->str);
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
			empty_data = (layer_empty *) this_layer_data->object_data;
			empty_layer_shape = newSWFShape();
			if (NULL == empty_layer_shape)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED111: Something went wrong when creating an empty layer for swf output");
				return FALSE;
			}

			// Create the required fill color for the empty layer
			red_component = roundf(empty_data->bg_color.red / 256);
			green_component = roundf(empty_data->bg_color.green / 256);
			blue_component = roundf(empty_data->bg_color.blue / 256);
			empty_layer_fill = SWFShape_addSolidFillStyle(empty_layer_shape, red_component, green_component, blue_component, 0xff);  // Alpha value - solid fill
			SWFShape_setRightFillStyle(empty_layer_shape, empty_layer_fill);

			// Set the line style
			SWFShape_setLine(empty_layer_shape, 0, red_component, green_component, blue_component, 0xff);

			// Create the empty layer object
			SWFShape_movePenTo(empty_layer_shape, 0.0, 0.0);
			SWFShape_drawLine(empty_layer_shape, output_width, 0.0);
			SWFShape_drawLine(empty_layer_shape, 0.0, output_height);
			SWFShape_drawLine(empty_layer_shape, -((gint) output_width), 0.0);
			SWFShape_drawLineTo(empty_layer_shape, 0.0, 0.0);

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
				SWFButton_addShape(swf_button, (SWFCharacter) empty_layer_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = compileSWFActionCode(as_gstring->str);
				SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

				// Add the dictionary shape to a movie clip, then store for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) swf_button);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) empty_layer_shape);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_HIGHLIGHT:

			// * We're processing a highlight layer *

			// Simplify pointers and work out element positioning info
			highlight_data = (layer_highlight *) this_layer_data->object_data;
			final_opacity = roundf((highlight_data->opacity / 100) * 256);

			// Create the swf movie clip object that holds the layer object
			container_movie_clip = newSWFMovieClip();

			// Create an empty object to make the highlight out of
			highlight_box = newSWFShape();
			if (NULL == highlight_box)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED98: Something went wrong when creating a highlight layer for swf output");
				return FALSE;
			}

			// Set the fill for the highlight box
			red_component = roundf(highlight_data->fill_colour.red / 256);
			green_component = roundf(highlight_data->fill_colour.green / 256);
			blue_component = roundf(highlight_data->fill_colour.blue / 256);
			highlight_fill_style = SWFShape_addSolidFillStyle(highlight_box, red_component, green_component, blue_component, final_opacity);
			SWFShape_setRightFillStyle(highlight_box, highlight_fill_style);

			// Set the border style
			red_component = roundf(highlight_data->border_colour.red / 256);
			green_component = roundf(highlight_data->border_colour.green / 256);
			blue_component = roundf(highlight_data->border_colour.blue / 256);
			SWFShape_setLine(highlight_box, highlight_data->border_width, red_component, green_component, blue_component, final_opacity);

			// Work out the scaled dimensions of the highlight box
			highlight_box_width = roundf(scaled_width_ratio * (gfloat) highlight_data->width);
			highlight_box_height = roundf(scaled_height_ratio * (gfloat) highlight_data->height);

			// Create the highlight box
			SWFShape_drawLine(highlight_box, highlight_box_width, 0.0);
			SWFShape_drawLine(highlight_box, 0.0, highlight_box_height);
			SWFShape_drawLine(highlight_box, -(highlight_box_width), 0.0);
			SWFShape_drawLine(highlight_box, 0.0, -(highlight_box_height));

			// Add the highlight object to the movie clip
			container_display_item = SWFMovieClip_add(container_movie_clip, (SWFBlock) highlight_box);

			// Position the background
			SWFDisplayItem_moveTo(container_display_item, 0.0, 0.0);

			// Advance the movie clip one frame, else it won't be displayed
			SWFMovieClip_nextFrame(container_movie_clip);

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
				SWFButton_addShape(swf_button, (SWFCharacter) container_movie_clip, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = compileSWFActionCode(as_gstring->str);
				SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

				// Add the dictionary shape to a movie clip, then store for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) swf_button);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{
				// Add the dictionary shape to a movie clip, then store for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) container_movie_clip);

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_MOUSE_CURSOR:

			// * We're processing a mouse layer *

			// Create the swf movie clip object that holds the layer object
			container_movie_clip = newSWFMovieClip();

			// Create the initial empty shape
			mouse_shape = newSWFShape();
			if (NULL == mouse_shape)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED414: Something went wrong when creating a mouse pointer shape for swf output");
				return FALSE;
			}

			// Set the fill for the mouse pointer
			mouse_fill_style = SWFShape_addSolidFillStyle(mouse_shape, 0xff, 0xff, 0xff, 0xff);
			SWFShape_setRightFillStyle(mouse_shape, mouse_fill_style);

			// Set the border style
			SWFShape_setLine(mouse_shape, 1, 0x00, 0x00, 0x00, 0xff);

			// Create the mouse pointer
			SWFShape_drawLine(mouse_shape, 22.0 * scaled_width_ratio, 17.0 * scaled_height_ratio);
			SWFShape_drawLine(mouse_shape, -11.0 * scaled_width_ratio, 0.0);
			SWFShape_drawLine(mouse_shape, 9.0 * scaled_width_ratio, 14.0 * scaled_height_ratio);
			SWFShape_drawLine(mouse_shape, -6.0 * scaled_width_ratio, 0.0);
			SWFShape_drawLine(mouse_shape, -8.0 * scaled_width_ratio, -13.0 * scaled_height_ratio);
			SWFShape_drawLine(mouse_shape, -6.0 * scaled_width_ratio, 8.0 * scaled_height_ratio);
			SWFShape_drawLineTo(mouse_shape, 0.0, 0.0);

			// Add the mouse layer object to the movie clip
			container_display_item = SWFMovieClip_add(container_movie_clip, (SWFBlock) mouse_shape);

			// Position the background
			SWFDisplayItem_moveTo(container_display_item, 0.0, 0.0);

			// Advance the movie clip one frame, else it won't be displayed
			SWFMovieClip_nextFrame(container_movie_clip);

			// If this layer has an external link associated with it, turn it into a button
			if (0 < this_layer_data->external_link->len)
			{
				// Displaying debugging info if requested
				if (debug_level)
				{
					printf("This mouse cursor layer has an external link: '%s'\n", this_layer_data->external_link->str);
				}

				// Create an empty button object we can use
				swf_button = newSWFButton();

				// Add the shape to the button for all of its states
				SWFButton_addShape(swf_button, (SWFCharacter) container_movie_clip, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = compileSWFActionCode(as_gstring->str);
				SWFButton_addAction(swf_button, swf_action, SWFBUTTON_MOUSEUP);

				// Add the button to a movie clip, then keep for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) swf_button);

				// If this object has a mouse click, then add the sound
				mouse_data = (layer_mouse *) this_layer_data->object_data;
				if (MOUSE_NONE != mouse_data->click)
				{
					swf_add_mouse_click(this_movie, mouse_data->click);
				}

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			} else
			{

				// Add the dictionary shape to a movie clip, then store for future reference
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) container_movie_clip);

				// If this object has a mouse click, then add the sound
				mouse_data = (layer_mouse *) this_layer_data->object_data;
				if (MOUSE_NONE != mouse_data->click)
				{
					swf_add_mouse_click(this_movie, mouse_data->click);
				}

				// Advance the movie clip one frame, else it doesn't get displayed
				SWFMovieClip_nextFrame(this_layer_data->dictionary_shape);
			}

			// Indicate that the dictionary shape for this layer was created ok
			return TRUE;

		case TYPE_TEXT:

			// We're processing a text layer

			// Simplify pointers and work out element positioning info
			text_data = (layer_text *) this_layer_data->object_data;

			// Create the text object we'll be using
			text_object = newSWFText();

			// Assign a font to the text object
			SWFText_setFont(text_object, font_object);

			// Set the height we want for the text
			scaled_font_size = scaled_height_ratio * text_data->font_size;
			SWFText_setHeight(text_object, scaled_font_size);
			text_real_font_size = SWFText_getAscent(text_object) + SWFText_getDescent(text_object);

			// Set the foreground color for the text
			red_component = roundf(text_data->text_color.red / 256);
			green_component = roundf(text_data->text_color.green / 256);
			blue_component = roundf(text_data->text_color.blue / 256);
			SWFText_setColor(text_object, red_component, green_component, blue_component, 0xff);

			// Work out how many lines of text we're dealing with
			num_text_lines = gtk_text_buffer_get_line_count(text_data->text_buffer);

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

			// * Create the swf movie clip object that holds the text background and text *
			text_movie_clip = newSWFMovieClip();

			// * Create the background for the text object *
			if (TRUE == text_data->show_bg)
			{
				text_bg = newSWFShape();
				if (NULL == text_bg)
				{
					// Something went wrong when creating the empty shape, so we skip this layer
					display_warning("Error ED101: Something went wrong when creating a text layer background for swf output");
					return FALSE;
				}

				// Set the solid fill for the text background box
				red_component = roundf(text_data->bg_fill_colour.red / 256);
				green_component = roundf(text_data->bg_fill_colour.green / 256);
				blue_component = roundf(text_data->bg_fill_colour.blue / 256);
				text_bg_fill_style = SWFShape_addSolidFillStyle(text_bg, red_component, green_component, blue_component, 0xff); // Alpha of 0xff is full opacity
				SWFShape_setRightFillStyle(text_bg, text_bg_fill_style);

				// Set the line style
				red_component = roundf(text_data->bg_border_colour.red / 256);
				green_component = roundf(text_data->bg_border_colour.green / 256);
				blue_component = roundf(text_data->bg_border_colour.blue / 256);
				SWFShape_setLine(text_bg, text_data->bg_border_width, red_component, green_component, blue_component, 0xff); // Alpha of 0xff is full opacity

				// Work out the scaled dimensions of the text background box
				text_leading = SWFText_getLeading(text_object);
				text_bg_box_height = (text_real_font_size * num_text_lines) * 1.02;
				text_bg_box_width = widest_text_string_width + (text_leading * 2);

				// Create the text background box
				SWFShape_drawLine(text_bg, text_bg_box_width, 0.0);
				SWFShape_drawLine(text_bg, 0.0, text_bg_box_height);
				SWFShape_drawLine(text_bg, -(text_bg_box_width), 0.0);
				SWFShape_drawLine(text_bg, 0.0, -(text_bg_box_height));

				// Add the text background to the movie clip
				text_bg_display_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text_bg);

				// Position the background
				SWFDisplayItem_moveTo(text_bg_display_item, 0.0, 0.0);
			}

			// Add the text object to the movie clip
			text_display_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text_object);

			// Position the text elements
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
				SWFButton_addShape(swf_button, (SWFCharacter) text_movie_clip, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN);

				// Use the text background area as the hit state
				if (TRUE == text_data->show_bg)
				{
					SWFButton_addShape(swf_button, (SWFCharacter) text_bg, SWFBUTTON_HIT);
				} else
				{
					SWFButton_addShape(swf_button, (SWFCharacter) text_movie_clip, SWFBUTTON_HIT);
				}

				// Add action script to the button, jumping to the external link
				g_string_printf(as_gstring, "getURL(\"%s\", \"%s\", \"POST\");", this_layer_data->external_link->str, this_layer_data->external_link_window->str);
				swf_action = compileSWFActionCode(as_gstring->str);
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

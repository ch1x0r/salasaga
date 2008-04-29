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
#include "create_swf_text_shape.h"


gboolean export_swf_create_shape(layer *this_layer_data)
{
	// Local variables
	GString				*as_gstring = NULL;			// Used for constructing action script statements
	guint16				blue_component;				// Used when retrieving the foreground color of text
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	guint				final_opacity;				// Used when calculating the final opacity figure for a highlight layer
	GError				*error = NULL;				// Pointer to error return structure
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
	gint				num_text_lines;				// Number of text lines in a particular text layer
	SWFBlock			our_shape;					// The swf shape before it gets added to a swf movie clip
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	guint16				red_component;				// Used when retrieving the foreground color of text
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	SWFAction			swf_action;					// Used when constructing action script
	SWFButton			swf_button;					// Holds a swf button
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
	gfloat				widest_text_string_width;	// Used when calculating how wide to draw the text background box

	// (Hopefully) temporary variables put in place to get around a *bizarre*
	// problem whereby calculating basic stuff like "0 - output_width" gives bogus results (computers are infallible eh?) :(
	// (Suspect it's caused by some optimisation at compile time going wrong)
	gfloat				layer_down;
	gfloat				layer_left;
	gfloat				layer_right;
	gfloat				layer_up;


	// Initialisation
	as_gstring = g_string_new(NULL);

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
			empty_layer_shape = newSWFShape();
			if (NULL == empty_layer_shape)
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
				SWFButton_addShape(swf_button, (SWFCharacter) empty_layer_shape, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

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
			highlight_data = (layer_highlight *) this_layer_data->object_data;
			final_opacity = roundf((highlight_data->opacity / 100) * 255);
			highlight_box = newSWFShape();
			if (NULL == highlight_box)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				display_warning("Error ED98: Something went wrong when creating a highlight layer for swf output");
				return FALSE;
			}

			// Set the fill for the highlight box
			red_component = highlight_data->fill_colour.red;
			green_component = highlight_data->fill_colour.green;
			blue_component = highlight_data->fill_colour.blue;
			highlight_fill_style = SWFShape_addSolidFillStyle(highlight_box,
					roundf(red_component / 255),
					roundf(green_component / 255),
					roundf(blue_component / 255),
					final_opacity);
			SWFShape_setRightFillStyle(highlight_box, highlight_fill_style);

			// Set the border style
			red_component = highlight_data->border_colour.red;
			green_component = highlight_data->border_colour.green;
			blue_component = highlight_data->border_colour.blue;
			SWFShape_setLine(highlight_box,
					highlight_data->border_width,
					roundf(red_component / 255),
					roundf(green_component / 255),
					roundf(blue_component / 255),
					final_opacity);

			// Work out the scaled dimensions of the highlight box
			highlight_box_width = roundf(scaled_width_ratio * (gfloat) highlight_data->width);
			highlight_box_height = roundf(scaled_height_ratio * (gfloat) highlight_data->height);

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
				SWFButton_addShape(swf_button, (SWFCharacter) highlight_box, SWFBUTTON_UP|SWFBUTTON_OVER|SWFBUTTON_DOWN|SWFBUTTON_HIT);

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

		case TYPE_TEXT:

			// We're processing a text layer

			// Simplify pointers and work out element positioning info
			text_data = (layer_text *) this_layer_data->object_data;

			// Create the text object
			text_object = create_swf_text_shape(text_data,
					scaled_width_ratio,
					scaled_height_ratio,
					&num_text_lines,
					&widest_text_string_width,
					&text_real_font_size);

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
				red_component = text_data->bg_fill_colour.red;
				green_component = text_data->bg_fill_colour.green;
				blue_component = text_data->bg_fill_colour.blue;
				text_bg_fill_style = SWFShape_addSolidFillStyle(text_bg,
						roundf(red_component / 255),
						roundf(green_component / 255),
						roundf(blue_component / 255),
						0xff); // Alpha of 255 is full opacity
				SWFShape_setRightFillStyle(text_bg, text_bg_fill_style);

				// Set the line style
				red_component = text_data->bg_border_colour.red;
				green_component = text_data->bg_border_colour.green;
				blue_component = text_data->bg_border_colour.blue;
				SWFShape_setLine(text_bg,
						text_data->bg_border_width,
						roundf(red_component / 255),
						roundf(green_component / 255),
						roundf(blue_component / 255),
						0xff); // Alpha of 255 is full opacity

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
				SWFButton_addShape(swf_button, (SWFCharacter) text_bg, SWFBUTTON_HIT);

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

/*
 * $Id$
 *
 * Salasaga: Creates a swf shape from a given layer
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "../../dialog/display_warning.h"
#include "swf_add_mouse_click.h"
#include "swf_shape_from_image_file.h"


gboolean export_swf_create_shape(SWFMovie this_movie, layer *this_layer_data)
{
	// Local variables
	GSList				*applied_tags;				// Receives a list of text tags applied at a position in a text buffer
	GString				*as_gstring = NULL;			// Used for constructing action script statements
	guint16				blue_component;				// Used when retrieving the foreground color of text
	gint				char_font_face = -1;		// Used for calculating the font face of a character
	gint				*char_font_ptr;				// Used for calculating the font face of a character
	SWFDisplayItem		container_display_item;
	SWFMovieClip		container_movie_clip;		// The movie clip that contains the layer object
	gfloat				current_ming_scale;			// Used when creating text swf output
	GtkTextIter			cursor_iter;				// Used for positioning in a text layer's text buffer
	layer_empty			*empty_data;				// Points to the empty object data inside the layer
	SWFFillStyle		empty_layer_fill;			// Fill style used when constructing empty layer shapes
	SWFShape			empty_layer_shape;			// Temporary swf shape used when constructing empty layers
	GError				*error = NULL;				// Pointer to error return structure
	GdkColor			*fg_colour;					// Foreground colour to set a text layer character to
	guint				final_opacity;				// Used when calculating the final opacity figure for a highlight layer
	gdouble				font_size;					// Used for retrieving the desired size of a character in a text layer
	gint				font_size_int;				// Used for calculating the scaled font size to display a text layer character at
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
	gint				line_counter;				// Simple loop counter variable
	gfloat				line_height;
	gfloat				line_width;
	gfloat				line_x_bearing;
	gboolean			line_x_bearing_known;		// Simple boolean to track if we know the x_bearing for line yet
	gint				loop_counter;				// Simple counter used in loops
	gfloat				max_line_width;
	GString				*message;					// Used to construct message strings
	gboolean			more_chars;					// Simple boolean used when rendering a text layer
	layer_mouse			*mouse_data;				// Points to the mouse object data inside the layer
	gint				mouse_ptr_width;			// Holds the calculated width for a mouse pointer graphic
	gint				mouse_ptr_height;			// Holds the calculated height for a mouse pointer graphic
	SWFShape			mouse_shape = NULL;			//
	gint				num_lines;
	gint				num_tags;					// Receives the total number of tags applied to a text character
	gint				num_text_lines;				// Number of text lines in a particular text layer
	SWFBlock			our_shape;					// The swf shape before it gets added to a swf movie clip
	PangoFontDescription	*pango_font_face;		// Text layer font face to use when drawing a character
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	gfloat				radius = 8;					// Radius to use for rounded rectangles
	guint16				red_component;				// Used when retrieving the foreground colour of text
	GString				*render_string;				// Used for rendering a text layer, one character at a time
	gint				*rendered_line_heights;		// Temporary variable used to hold the height of text lines
	GdkPixbuf			*resized_pixbuf;			// Temporary pixbuf used while scaling images
	gboolean			return_code_bool;			// Receives boolean return codes
	gfloat				scaled_font_size;			// Display height of a font in swf, when scaled to the desired output size
	gint				scaled_height;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_height_ratio;		// Used to calculate the final size an object should be scaled to
	gint				scaled_width;				// Used to calculate the final size an object should be scaled to
	gfloat				scaled_width_ratio;			// Used to calculate the final size an object should be scaled to
	SWFAction			swf_action;					// Used when constructing action script
	SWFButton			swf_button;					// Holds a swf button
	GtkTextAppearance	*text_appearance;			// Used in text layer rendering, to determine some of the attributes needed
	GtkTextAttributes	*text_attributes;			// Pointer to the attributes for a text layer character
	SWFShape			text_bg = NULL;				// The text background shape goes in this
	gfloat				text_bg_box_height;			// Used while generating swf output for text boxes
	gfloat				text_bg_box_width;			// Used while generating swf output for text boxes
	SWFDisplayItem		text_bg_display_item;
	SWFFillStyle		text_bg_fill_style;			// Fill style used when constructing text background shape
	GtkTextBuffer		*text_buffer;				// Pointer to the text buffer we're using
	layer_text			*text_data;					// Points to the text object data inside the layer
	SWFDisplayItem		text_display_item;
	GtkTextIter			text_end;					// End position of text buffer
	gfloat				text_leading = 0.0;			// Spacing to use at the edges of the font
	gint				text_lines_counter;			// Counter used when processing text
	SWFMovieClip		text_movie_clip;			// The movie clip that contains the text background and text
	SWFText				text_object;				// The text object we're working on goes in this
	gdouble				text_pos_x = 0;				// Used for positioning where swf text will be drawn
	gdouble				text_pos_y = 0;				// Used for positioning where swf text will be drawn
	gfloat				text_real_font_size;
	GtkTextIter			text_start;					// Start position of text buffer
	GtkWidget			*text_view;					// Pointer to a temporary text view
	GtkTextTag			*this_tag = NULL;			// Used in a loop for pointing to individual text tags
	gfloat				this_text_string_width;		// Used when calculating how wide to draw the text background box
	gfloat				total_text_height;
	gchar				*visible_string;			// Text string is retrieved into this
	gfloat				widest_text_string_width;	// Used when calculating how wide to draw the text background box


	// Initialisation
	as_gstring = g_string_new(NULL);
	message = g_string_new(NULL);
	render_string = g_string_new(NULL);

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
				printf(_("Image width: %d\n"), image_width);
				printf(_("Scaled width: %d\n"), scaled_width);
				printf(_("Image height: %d\n"), image_height);
				printf(_("Scaled height: %d\n"), scaled_height);
			}

			// Scale the image to the correct dimensions
			resized_pixbuf = gdk_pixbuf_scale_simple(((layer_image *) this_layer_data->object_data)->image_data, scaled_width, scaled_height, GDK_INTERP_HYPER);  // Do the scaling at best possible quality
			if (NULL == resized_pixbuf)
			{
				// Something went wrong when creating the dictionary shape for this layer
				g_string_printf(message, "%s ED90: %s", _("Error"), _("Something went wrong when creating the dictionary shape for an image layer."));
				display_warning(message->str);
				g_string_free(message, TRUE);
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
				g_string_printf(message, "%s ED92: %s", _("Error"), _("Something went wrong when encoding an image to required format."));
				display_warning(message->str);

				// Free the memory allocated in this function
				g_string_free(message, TRUE);
				g_error_free(error);
				if (NULL != resized_pixbuf)
					g_object_unref(GDK_PIXBUF(resized_pixbuf));
				return FALSE;
			}

			// Turn the image data into a swf bitmap
			image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
			image_bitmap = newSWFBitmap_fromInput(image_input);

			// Turn the swf image into a swf shape
			image_shape = newSWFShapeFromBitmap(image_bitmap, SWFFILL_CLIPPED_BITMAP);
			if (NULL == image_shape)
			{
				// Something went wrong when encoding the image to required format
				g_string_printf(message, "%s ED109: %s", _("Error"), _("Something went wrong converting an image to a swf shape object."));
				display_warning(message->str);

				// Free the memory allocated in this function
				g_string_free(message, TRUE);
				g_error_free(error);
				if (NULL != resized_pixbuf)
					g_object_unref(GDK_PIXBUF(resized_pixbuf));
				destroySWFBitmap(image_bitmap);
				destroySWFInput(image_input);
				return FALSE;
			}

			// If this layer has an external link associated with it, turn it into a button
			if (0 < this_layer_data->external_link->len)
			{
				// Displaying debugging info if requested
				if (debug_level)
				{
					printf(_("This image has an external link: '%s'\n"), this_layer_data->external_link->str);
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
				this_layer_data->dictionary_shape = newSWFMovieClip();
				SWFMovieClip_add(this_layer_data->dictionary_shape, (SWFBlock) image_shape);

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
				g_string_printf(message, "%s ED111: %s", _("Error"), _("Something went wrong when creating an empty layer for swf output."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}

			// Create the required fill colour for the empty layer
			red_component = roundf(empty_data->bg_color.red / 256);
			green_component = roundf(empty_data->bg_color.green / 256);
			blue_component = roundf(empty_data->bg_color.blue / 256);
			empty_layer_fill = SWFShape_addSolidFillStyle(empty_layer_shape, red_component, green_component, blue_component, 0xff);  // Alpha value - solid fill
			SWFShape_setRightFillStyle(empty_layer_shape, empty_layer_fill);

			// Ensure there is no border
			SWFShape_hideLine(empty_layer_shape);


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
					printf(_("This empty layer has an external link: '%s'\n"), this_layer_data->external_link->str);
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
				g_string_printf(message, "%s ED98: %s", _("Error"), _("Something went wrong when creating a highlight layer for swf output."));
				display_warning(message->str);
				g_string_free(message, TRUE);
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
					printf(_("This highlight has an external link: '%s'\n"), this_layer_data->external_link->str);
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

			// Load the mouse pointer graphic
			mouse_ptr_width = 21 * scaled_width_ratio;
			mouse_ptr_height = 30 * scaled_height_ratio;
			mouse_shape = swf_shape_from_image_file(mouse_ptr_string->str, mouse_ptr_width, mouse_ptr_height);
			if (NULL == mouse_shape)
			{
				// Something went wrong when creating the empty shape, so we skip this layer
				g_string_printf(message, "%s ED414: %s", _("Error"), _("Something went wrong when creating a mouse pointer shape for swf output."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}

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
					printf(_("This mouse cursor layer has an external link: '%s'\n"), this_layer_data->external_link->str);
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
			text_buffer = text_data->text_buffer;

			// Create the text object we'll be using
			text_object = newSWFText();

			// Create a text view, not for display, but instead to retrieve attribute information from the layer's text buffer
			text_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(text_buffer));

			// Retrieve the default attributes for the text view
			text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));

			// Determine how many lines are in the text buffer, so we can loop through them
			num_lines = gtk_text_buffer_get_line_count(text_buffer);
			rendered_line_heights = g_new0(gint, num_lines);

			for (line_counter = 0; line_counter < num_lines; line_counter++)
			{
				// Position the text iter at the start of the line
				gtk_text_buffer_get_iter_at_line(text_buffer, &cursor_iter, line_counter);

				// Loop around, processing all the characters in the text buffer
				line_height = 0;
				line_width = 0;
				line_x_bearing = 0;
				line_x_bearing_known = FALSE;
				more_chars = TRUE;
				while (more_chars)
				{
					// Simplify pointers
					text_appearance = &(text_attributes->appearance);
					pango_font_face = text_attributes->font;
					fg_colour = &(text_appearance->fg_color);

					// Get the character to be written
					g_string_printf(render_string, "%c", gtk_text_iter_get_char(&cursor_iter));
					SWFText_addUTF8String(text_object, render_string->str, NULL);

					// Calculate the size the character should be displayed at
					font_size_int = pango_font_description_get_size(pango_font_face);
					font_size = rint(scaled_height_ratio * font_size_int / PANGO_SCALE);

					// Get the text tags that apply to this iter
					applied_tags = gtk_text_iter_get_tags(&cursor_iter);

					// Run through the list of tags and extract the info that tells us which font face to use
					num_tags = g_slist_length(applied_tags);
					for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
					{
						this_tag = g_slist_nth_data(applied_tags, loop_counter);
						char_font_ptr = g_object_get_data(G_OBJECT(this_tag), "font-num");
						if (NULL != char_font_ptr)
						{
							// Found the required font face info, so set the font face with it
							char_font_face = *char_font_ptr;
							SWFText_setFont(text_object, fdb_font_object[char_font_face]);
						}
					}

					// Free the list of tags, as they're no longer needed
					g_slist_free(applied_tags);

					// Set the font size
					scaled_font_size = scaled_height_ratio * font_size;
					SWFText_setHeight(text_object, scaled_font_size);

					// Retrieve and store the on screen dimensions of this character
					text_real_font_size = SWFText_getAscent(text_object) + SWFText_getDescent(text_object);
					if (text_real_font_size > line_height)
					{
						// Keep the largest character height for this line
						line_height = text_real_font_size;
					}

					if (FALSE == line_x_bearing_known)
					{
						// We add the x_bearing for the very first character of a line, so
						// the display of text starts that little bit further to the right
						line_x_bearing = SWFText_getLeading(text_object);
						line_width += line_x_bearing;
						line_x_bearing_known = TRUE;
					}
					line_width += SWFText_getUTF8StringWidth(text_object, (guchar *) render_string->str);

					// Move to the next character in the text buffer
					gtk_text_iter_forward_cursor_position(&cursor_iter);

					// If we're at the end of the line, then break out of this loop
					if (TRUE == gtk_text_iter_ends_line(&cursor_iter))
					{
						more_chars = FALSE;
					}
				}

				// * At this point we've worked out the height of this line
				rendered_line_heights[line_counter] = line_height;  // Cache the value
				total_text_height += line_height + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);
				text_pos_x = line_x_bearing;
				text_pos_y += line_height + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);

				// Keep the largest line width known
				if (line_width > max_line_width)
				{
					max_line_width = line_width;
				}
			}

/*
			// Assign a font to the text object
			SWFText_setFont(text_object, fdb_font_object[text_data->font_face]);

			// Set the height we want for the text
			scaled_font_size = scaled_height_ratio * text_data->font_size;
			SWFText_setHeight(text_object, scaled_font_size);
			text_real_font_size = SWFText_getAscent(text_object) + SWFText_getDescent(text_object);

			// Set the foreground colour for the text
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
				SWFText_addUTF8String(text_object, visible_string, NULL);

				// * We need to know which of the strings is widest, so we can calculate the width of the text background box *

				// If this is the widest string, we keep the value of this one
				this_text_string_width = SWFText_getUTF8StringWidth(text_object, (guchar *) visible_string);
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
					printf(_("Line %d of %d: %s\n"), text_lines_counter, num_text_lines, gtk_text_iter_get_visible_text(&text_start, &text_end));
					printf(_("Width of this string: %.2f\n"), this_text_string_width);
					printf(_("Width of widest string thus far: %.2f\n"), widest_text_string_width);
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
					g_string_printf(message, "%s ED101: %s", _("Error"), _("Something went wrong when creating a text layer background for swf output."));
					display_warning(message->str);
					g_string_free(message, TRUE);
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

				// Rounded rectangle method from http://www.cairographics.org/cookbook/roundedrectangles/
				SWFShape_movePenTo(text_bg, radius, 0);
				SWFShape_drawLineTo(text_bg, text_bg_box_width - radius, 0);
				SWFShape_drawCubicTo(text_bg, text_bg_box_width, 0, text_bg_box_width, 0, text_bg_box_width, radius);
				SWFShape_drawLineTo(text_bg, text_bg_box_width, text_bg_box_height - radius);
				SWFShape_drawCubicTo(text_bg, text_bg_box_width, text_bg_box_height, text_bg_box_width, text_bg_box_height, text_bg_box_width - radius, text_bg_box_height);
				SWFShape_drawLineTo(text_bg, radius, text_bg_box_height);
				SWFShape_drawCubicTo(text_bg, 0, text_bg_box_height, 0, text_bg_box_height, 0, text_bg_box_height - radius);
				SWFShape_drawLineTo(text_bg, 0, radius);
				SWFShape_drawCubicTo(text_bg, 0, 0, 0, 0, radius, 0);

				// Add the text background to the movie clip
				text_bg_display_item = SWFMovieClip_add(text_movie_clip, (SWFBlock) text_bg);

				// Position the background
				SWFDisplayItem_moveTo(text_bg_display_item, 0.0, 0.0);
			}
*/
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
					printf(_("This text has an external link: '%s'\n"), this_layer_data->external_link->str);
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
			g_string_printf(message, "%s ED83: %s", _("Error"), _("Unknown layer type in swf output."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);
}

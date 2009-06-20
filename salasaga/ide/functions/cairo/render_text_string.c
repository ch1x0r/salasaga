/*
 * $Id$
 *
 * Salasaga: Function to calculate width and height of a text layer, optionally rendering it on screen too
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Cairo FreeType include
#include <cairo/cairo-ft.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


int render_text_string(cairo_t *existing_cairo_context, layer_text *text_object, gfloat scaled_height_ratio, gdouble incoming_cairo_pos_x, gdouble incoming_cairo_pos_y, gfloat time_alpha, gboolean display_onscreen)
{
	// Local variables
	GSList					*applied_tags;			// Receives a list of text tags applied at a position in a text buffer
	cairo_t					*cairo_context;			// Cairo drawing context
	gdouble					cairo_pos_x = 0;		// Used for positioning where cairo will draw, in text layers
	gdouble					cairo_pos_y = 0;		// Used for positioning where cairo will draw, in text layers
	GtkTextIter				cursor_iter;			// Used for positioning in a text buffer
	cairo_font_face_t		*font_array_face = NULL;  // Gets pointed to a cairo font face for each character when creating the text layer
	GdkColor				*fg_colour;				// Foreground colour to set a text layer character to
	PangoFontDescription	*font_face;				// Text layer font face to use when drawing a character
	gdouble					font_size;				// Used for retrieving the desired size of a character in a text layer
	gint					font_size_int;			// Used for calculating the scaled font size to display a text layer character at
	gint					line_counter;
	gfloat					line_height;
	gfloat					line_width;
	gint					loop_counter;			// Simple counter used in loops
	gfloat					max_line_width;
	gboolean				more_chars;				// Simple boolean used when rendering a text layer
	gint					num_lines;
	gint					num_tags;				// Receives the total number of tags applied to a text character
	GString					*render_string;			// Used for rendering a text layer, one character at a time
	GtkTextAppearance		*text_appearance;		// Used in text layer rendering, to determine some of the attributes needed
	GtkTextAttributes		*text_attributes;		// Pointer to the attributes for a text layer character
	GtkTextBuffer			*text_buffer;			// Pointer to the text buffer we're using
	GtkTextIter				text_end;				// The end position of the text buffer
	cairo_text_extents_t	text_extents;			// Meta information about an onscreen text string
	gfloat					text_height;
	GtkWidget				*text_view;				// Pointer to a temporary text view
	GtkTextTag				*this_tag = NULL;		// Used in a loop for pointing to individual text tags
	gfloat					total_text_height;


	// If we are given an existing cairo context, then reuse that, else create a new one
	if (NULL != existing_cairo_context)
	{
		cairo_context = existing_cairo_context;
	} else
	{
		cairo_context = gdk_cairo_create(GDK_PIXMAP(front_store));
	}

	// Set things up
	cairo_pos_x = incoming_cairo_pos_x;
	cairo_pos_y = incoming_cairo_pos_y;
	max_line_width = 0;
	render_string = g_string_new(NULL);
	text_buffer = text_object->text_buffer;
	text_height = 0;
	total_text_height = 0;

	// Create a text view, not for display, but instead to retrieve attribute information from the layer's text buffer
	text_view = gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(text_buffer));

	// Retrieve the default attributes for the text view
	text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));

	// Determine how many lines are in the text buffer, so we can loop through them
	num_lines = gtk_text_buffer_get_line_count(text_buffer);
	for (line_counter = 0; line_counter < num_lines; line_counter++)
	{
		// Position the text iter at the start of the line
		gtk_text_buffer_get_iter_at_line(text_buffer, &cursor_iter, line_counter);

		// Loop around, processing all the characters in the text buffer
		line_height = 0;
		line_width = 0;
		more_chars = TRUE;
		while (more_chars)
		{
			// Retrieve the attributes at this cursor position
			gtk_text_iter_get_attributes(&cursor_iter, text_attributes);

			// Simplify pointers
			text_appearance = &(text_attributes->appearance);
			font_face = text_attributes->font;
			fg_colour = &(text_appearance->fg_color);

			// Get the character to be written
			g_string_printf(render_string, "%c", gtk_text_iter_get_char(&cursor_iter));

			// Calculate the size the character should be displayed at
			font_size_int = pango_font_description_get_size(font_face);
			font_size = rint(scaled_height_ratio * font_size_int / PANGO_SCALE);

			// Get the text tags that apply to this iter
			applied_tags = gtk_text_iter_get_tags(&cursor_iter);

			// Run through the list of tags and extract the info that tells us which font face to use in the cairo font face array
			num_tags = g_slist_length(applied_tags);
			for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
			{
				this_tag = g_slist_nth_data(applied_tags, loop_counter);
				font_array_face = g_object_get_data(G_OBJECT(this_tag), "array-font");
				if (NULL != font_array_face)
				{
					// Found the required font face info, so set the font face with it
					cairo_set_font_face(cairo_context, font_array_face);
				}
			}

			// Free the list of tags, as they're no longer needed
			g_slist_free(applied_tags);

			// Set the font size
			cairo_set_font_size(cairo_context, font_size);

			// Retrieve and store the on screen dimensions of this character
			cairo_text_extents(cairo_context, render_string->str, &text_extents);
			if (text_extents.height > line_height)
			{
				// Keep the largest character height for this line
				line_height = text_extents.height;
			}
			line_width += text_extents.x_advance;

			// Move to the next character in the text buffer
			gtk_text_iter_forward_cursor_position(&cursor_iter);

			// If we're at the end of the line, then break out of this loop
			if (TRUE == gtk_text_iter_ends_line(&cursor_iter))
			{
				more_chars = FALSE;
			}
		}

		// * At this point we've worked out the height and width of this line
		total_text_height += line_height;
		cairo_pos_x = incoming_cairo_pos_x + (TEXT_BORDER_PADDING_WIDTH * scaled_height_ratio);
		cairo_pos_y += line_height + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);

		// If we've been requested to display the text on screen, then do so
		if (TRUE == display_onscreen)
		{
			// Reposition to the start of the line
			gtk_text_buffer_get_iter_at_line(text_buffer, &cursor_iter, line_counter);

			more_chars = TRUE;
			while (more_chars)
			{
				// Set the foreground colour for this character
				cairo_set_source_rgba(cairo_context, fg_colour->red / 65535.0, fg_colour->green / 65535.0, fg_colour->blue / 65535.0, time_alpha);

				// Get the character to be written
				g_string_printf(render_string, "%c", gtk_text_iter_get_char(&cursor_iter));

				// Calculate the size the character should be displayed at
				font_size_int = pango_font_description_get_size(font_face);
				font_size = rint(scaled_height_ratio * font_size_int / PANGO_SCALE);

				// Get the text tags that apply to this iter
				applied_tags = gtk_text_iter_get_tags(&cursor_iter);

				// Run through the list of tags and extract the info that tells us which font face to use in the cairo font face array
				num_tags = g_slist_length(applied_tags);
				for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
				{
					this_tag = g_slist_nth_data(applied_tags, loop_counter);
					font_array_face = g_object_get_data(G_OBJECT(this_tag), "array-font");
					if (NULL != font_array_face)
					{
						// Found the required font face info, so set the font face with it
						cairo_set_font_face(cairo_context, font_array_face);
					}
				}

				// Free the list of tags, as they're no longer needed
				g_slist_free(applied_tags);

				// Set the font size
				cairo_set_font_size(cairo_context, font_size);

				// Position the character on screen
				cairo_text_extents(cairo_context, render_string->str, &text_extents);
				cairo_move_to(cairo_context, cairo_pos_x, cairo_pos_y);
				cairo_pos_x += text_extents.x_advance;

				// Display the character on screen
				cairo_show_text(cairo_context, render_string->str);

				// Move to the next character in the text buffer
				gtk_text_iter_forward_cursor_position(&cursor_iter);

				// If we're at the end of the line, then break out of this loop
				if (TRUE == gtk_text_iter_ends_line(&cursor_iter))
				{
					more_chars = FALSE;
				}
			}
		}
	}

/*

	// Determine the on screen size of the text object
	max_line_width = 0;
	text_height = 0;
	num_lines = gtk_text_buffer_get_line_count(text_buffer);
	for (line_counter = 0; line_counter < num_lines; line_counter++)
	{
		gtk_text_buffer_get_iter_at_line(text_buffer, &text_start, line_counter);
		text_end = text_start;
		gtk_text_iter_forward_to_line_end(&text_end);
		if (NULL != text_string)
			g_free(text_string);
		text_string = gtk_text_iter_get_visible_text(&text_start, &text_end);
		cairo_text_extents(cairo_context, text_string, &text_extents);
		text_height += text_extents.height;
		if ((text_extents.width + text_extents.x_bearing) > max_line_width)
			max_line_width = text_extents.width + text_extents.x_bearing;
	}
	// Calculate the text object (including background) offsets and sizing
	cairo_text_extents(cairo_context, text_string, &text_extents);
	x_offset = layer_positions.x * scaled_width_ratio;
	y_offset = layer_positions.y * scaled_height_ratio;
	width = max_line_width + (TEXT_BORDER_PADDING_WIDTH * 4 * scaled_width_ratio);
	height = text_height + (TEXT_BORDER_PADDING_HEIGHT * (num_lines + 2) * scaled_height_ratio);

	// Store the rendered width of the text object with the layer itself, for use by bounding box code
	text_object->rendered_width = width / scaled_width_ratio;
	text_object->rendered_height = height / scaled_height_ratio;

	// * If required, draw the background for the text layer *
	if (TRUE == text_object->show_bg)
	{
		// Create the solid background fill
		cairo_set_operator(cairo_context, CAIRO_OPERATOR_SOURCE);
		selected_colour = &((layer_text *) this_layer_data->object_data)->bg_fill_colour;
		red_component = ((gfloat) selected_colour->red) / 65536;
		green_component = ((gfloat) selected_colour->green) / 65536;
		blue_component = ((gfloat) selected_colour->blue) / 65536;
		cairo_set_source_rgb(cairo_context, red_component, green_component, blue_component);

		// Rounded rectangle method from http://www.cairographics.org/cookbook/roundedrectangles/
		cairo_move_to(cairo_context, x_offset + radius, y_offset);
		cairo_line_to(cairo_context, x_offset + width - radius, y_offset);
		cairo_curve_to(cairo_context, x_offset + width, y_offset, x_offset + width, y_offset, x_offset + width, y_offset + radius);
		cairo_line_to(cairo_context, x_offset + width, y_offset + height - radius);
		cairo_curve_to(cairo_context, x_offset + width, y_offset + height, x_offset + width, y_offset + height, x_offset + width - radius, y_offset + height);
		cairo_line_to(cairo_context, x_offset + radius, y_offset + height);
		cairo_curve_to(cairo_context, x_offset, y_offset + height, x_offset, y_offset + height, x_offset, y_offset + height - radius);
		cairo_line_to(cairo_context, x_offset, y_offset + radius);
		cairo_curve_to(cairo_context, x_offset, y_offset, x_offset, y_offset, x_offset + radius, y_offset);
		cairo_clip(cairo_context);
		cairo_paint_with_alpha(cairo_context, time_alpha);

		// Draw the rounded rectangle border
		cairo_set_operator(cairo_context, CAIRO_OPERATOR_OVER);
		selected_colour = &((layer_text *) this_layer_data->object_data)->bg_border_colour;
		red_component = ((gfloat) selected_colour->red) / 65536;
		green_component = ((gfloat) selected_colour->green) / 65536;
		blue_component = ((gfloat) selected_colour->blue) / 65536;
		cairo_set_source_rgba(cairo_context, red_component, green_component, blue_component, time_alpha);
		cairo_set_line_width(cairo_context, ((layer_text *) this_layer_data->object_data)->bg_border_width);
		cairo_set_line_join(cairo_context, CAIRO_LINE_JOIN_ROUND);
		cairo_set_line_cap(cairo_context, CAIRO_LINE_CAP_ROUND);
		cairo_move_to(cairo_context, x_offset + radius, y_offset);
		cairo_line_to(cairo_context, x_offset + width - radius, y_offset);
		cairo_curve_to(cairo_context, x_offset + width, y_offset, x_offset + width, y_offset, x_offset + width, y_offset + radius);
		cairo_line_to(cairo_context, x_offset + width, y_offset + height - radius);
		cairo_curve_to(cairo_context, x_offset + width, y_offset + height, x_offset + width, y_offset + height, x_offset + width - radius, y_offset + height);
		cairo_line_to(cairo_context, x_offset + radius, y_offset + height);
		cairo_curve_to(cairo_context, x_offset, y_offset + height, x_offset, y_offset + height, x_offset, y_offset + height - radius);
		cairo_line_to(cairo_context, x_offset, y_offset + radius);
		cairo_curve_to(cairo_context, x_offset, y_offset, x_offset, y_offset, x_offset + radius, y_offset);
		cairo_stroke(cairo_context);
	}

	// * Draw the text string itself *

	// Set the desired font foreground colour
	selected_colour = &((layer_text *) this_layer_data->object_data)->text_color;
	red_component = ((gfloat) selected_colour->red) / 65536;
	green_component = ((gfloat) selected_colour->green) / 65536;
	blue_component = ((gfloat) selected_colour->blue) / 65536;
	cairo_set_source_rgba(cairo_context, red_component, green_component, blue_component, time_alpha);

	// Loop around, drawing lines of text
	text_top = y_offset + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);
	for (line_counter = 0; line_counter < num_lines; line_counter++)
	{
		// Retrieve a line of text
		gtk_text_buffer_get_iter_at_line(text_buffer, &text_start, line_counter);
		text_end = text_start;
		gtk_text_iter_forward_to_line_end(&text_end);
		if (NULL != text_string)
			g_free(text_string);
		text_string = gtk_text_iter_get_visible_text(&text_start, &text_end);

		// Move onscreen X and Y coordinates to correct position for the line of text
		cairo_text_extents(cairo_context, text_string, &text_extents);
		text_left = x_offset + text_extents.x_bearing + (TEXT_BORDER_PADDING_WIDTH * scaled_width_ratio);
		text_top += text_extents.height + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);
		text_adjustment = text_extents.height + text_extents.y_bearing;
		cairo_move_to(cairo_context, text_left, text_top -(text_adjustment));

		// Draw the line of text
		cairo_show_text(cairo_context, text_string);
	}
*/

	// If we created a new cairo context, then destroy it
	if (NULL == existing_cairo_context)
	{
		cairo_destroy(cairo_context);
	}

	// Free the memory used in this function
	g_object_unref(text_view);
	g_string_free(render_string, TRUE);

	return TRUE;
}

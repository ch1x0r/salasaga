/*
 * $Id$
 *
 * Salasaga: Function to calculate width and height of a text layer, prior to it ever having been rendered
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


gboolean calculate_text_object_size(layer_text *text_object)
{
	// Local variables
	cairo_t					*cairo_context;			// Cairo drawing context
	gint					height;					//
	gint					line_counter;
	gfloat					max_line_width;
	gint					num_lines;
	gint					pixmap_height;			// Receives the height of a given pixmap
	gint					pixmap_width;			// Receives the width of a given pixmap
	gfloat					scaled_height_ratio;	// Used to calculate a vertical scaling ratio 
	gfloat					scaled_width_ratio;		// Used to calculate a horizontal scaling ratio
	GtkTextBuffer			*text_buffer;			// Pointer to the text buffer we're using
	GtkTextIter				text_end;				// The end position of the text buffer
	cairo_text_extents_t	text_extents;			// Meta information about an onscreen text string
	gfloat					text_height;
	GtkTextIter				text_start;				// The start position of the text buffer
	gchar					*text_string;			// The text string to be displayed
	gint					time_x;					// Unscaled X position of the layer at our desired point in time
	gint					time_y;					// Unscaled Y position of the layer at our desired point in time
	gint					width;					//
	gint					x_offset;				// X coordinate of the object at the desired point in time
	gint					y_offset;				// Y coordinate of the object at the desired point in time


	// Simplify pointers
	text_buffer = text_object->text_buffer;

	// Calculate the height and width scaling values for the requested layer size
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) pixmap_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) pixmap_width / (gfloat) project_width;

	// Create a cairo drawing context
	cairo_context = gdk_cairo_create(GDK_PIXMAP(front_store));

	// Set the desired font size
	cairo_set_font_size(cairo_context, text_object->font_size * scaled_width_ratio);

	// Determine the on screen size of the text object
	max_line_width = 0;
	text_height = 0;
	num_lines = gtk_text_buffer_get_line_count(text_buffer);
	for (line_counter = 0; line_counter < num_lines; line_counter++)
	{
		gtk_text_buffer_get_iter_at_line(text_buffer, &text_start, line_counter);
		text_end = text_start;
		gtk_text_iter_forward_to_line_end(&text_end);
		text_string = gtk_text_iter_get_visible_text(&text_start, &text_end);
		cairo_text_extents(cairo_context, text_string, &text_extents);
		text_height += text_extents.height;
		if ((text_extents.width + text_extents.x_bearing) > max_line_width)
			max_line_width = text_extents.width + text_extents.x_bearing;
	}

	// Calculate the text object (including background) offsets and sizing
	cairo_text_extents(cairo_context, text_string, &text_extents);
	x_offset = time_x * scaled_width_ratio;
	y_offset = time_y * scaled_height_ratio;
	width = CLAMP(max_line_width + (TEXT_BORDER_PADDING_WIDTH * 2 * scaled_width_ratio),
				0, pixmap_width - x_offset - (TEXT_BORDER_PADDING_WIDTH * 2 * scaled_width_ratio) - 1);
	height = CLAMP(text_height + (TEXT_BORDER_PADDING_HEIGHT * (num_lines + 2) * scaled_height_ratio),
				0, pixmap_height - y_offset - (TEXT_BORDER_PADDING_HEIGHT * (num_lines + 2) * scaled_height_ratio) - 1);

	// Store the rendered width of the text object with the layer itself, for use by bounding box code
	text_object->rendered_width = width / scaled_width_ratio;
	text_object->rendered_height = height / scaled_height_ratio;

	// The cairo drawing context is no longer needed, so free it
	cairo_destroy(cairo_context);

	return TRUE;
}

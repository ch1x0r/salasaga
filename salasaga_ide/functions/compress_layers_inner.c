/*
 * $Id$
 *
 * Salasaga: Function to process the data for a layer, adding it to the main image being built 
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "cairo/create_cairo_pixbuf_pattern.h"


void compress_layers_inner(layer *this_layer_data, GdkPixmap *incoming_pixmap, gfloat time_position)
{
	// Local variables
	cairo_t					*cairo_context;			// Cairo drawing context
	gfloat					end_time;				// Time in seconds of the layer objects finish time
	gint					finish_x;				// X position at the layer objects finish time
	gint					finish_y;				// Y position at the layer objects finish time 
	gint					height;					//
	cairo_matrix_t			image_matrix;			// Transformation matrix used to position a cairo pattern
	gint					line_counter;
	gfloat					max_line_width;
	gint					num_lines;
	gint					pixmap_height;			// Receives the height of a given pixmap
	gint					pixmap_width;			// Receives the width of a given pixmap
	gfloat					scaled_height_ratio;	// Used to calculate a vertical scaling ratio 
	gfloat					scaled_width_ratio;		// Used to calculate a horizontal scaling ratio
	gfloat					start_time;				// Time in seconds of the layer objects start time
	gint					start_x;				// X position at the layer objects start time
	gint					start_y;				// Y position at the layer objects start time
	gfloat					text_adjustment;		// Y offset for a specific line
	gfloat					text_blue;				// Blue component of text fg colour
	GtkTextBuffer			*text_buffer;			// Pointer to the text buffer we're using
	GdkColor				*text_colour;			// Pointer to the foreground colour of the text
	GtkTextIter				text_end;				// The end position of the text buffer
	cairo_text_extents_t	text_extents;			// Meta information about an onscreen text string
	gfloat					text_green;				// Green component of text fg colour
	gfloat					text_height;
	gint					text_left;				// Pixel number onscreen for the left side of text
	layer_text				*text_object;			// Pointer to our object text data
	gfloat					text_red;				// Red component of text fg colour
	gint					text_top;				// Pixel number onscreen for the top of text
	GtkTextIter				text_start;				// The start position of the text buffer
	gchar					*text_string;			// The text string to be displayed
	layer_image				*this_image_data;		// Pointer to image layer data
	gfloat					time_alpha = 1.0;		// Alpha value to use at our desired point in time (defaulting to 1.0 = fall opacity)
	gfloat					time_offset;
	gint					time_x;					// Unscaled X position of the layer at our desired point in time
	gint					time_y;					// Unscaled Y position of the layer at our desired point in time
	gfloat					time_diff;				// Used when calculating the object position at the desired point in time
	gint					width;					//
	gint					x_diff;					// Used when calculating the object position at the desired point in time
	gint					x_offset;				// X coordinate of the object at the desired point in time
	gfloat					x_scale;				// Used when calculating the object position at the desired point in time
	gint					y_diff;					// Used when calculating the object position at the desired point in time
	gint					y_offset;				// Y coordinate of the object at the desired point in time
	gfloat					y_scale;				// Used when calculating the object position at the desired point in time


	// Is this layer invisible, or is it a background layer?
	if ((FALSE == this_layer_data->visible) || (TRUE == this_layer_data->background))
	{
		// We don't need to process this layer
		return;
	}

	// Simplify pointers
	finish_x = this_layer_data->x_offset_finish;
	finish_y = this_layer_data->y_offset_finish;
	start_time = this_layer_data->start_time;
	start_x = this_layer_data->x_offset_start;
	start_y = this_layer_data->y_offset_start;

	// If the layer isn't visible at the requested time, we don't need to proceed
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;
	if ((time_position < start_time) || (time_position > end_time))
	{
		return;
	}

	// Calculate how far into the layer movement we are
	time_offset = time_position - start_time;
	time_diff = end_time - start_time;
	x_diff = finish_x - start_x;
	x_scale = (((gfloat) x_diff) / time_diff);
	time_x = start_x + (x_scale * time_offset);
	y_diff = finish_y - start_y;
	y_scale = (((gfloat) y_diff) / time_diff);
	time_y = start_y + (y_scale * time_offset);

	// If the time position is during a transition, we need to determine the correct alpha value
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
	{
		// There is a transition in, so work out if the time position is during it
		end_time = start_time + this_layer_data->transition_in_duration;
		if ((time_position >= start_time) && (time_position < end_time))
		{
			// The time position is during a transition in
			time_diff = end_time - start_time;
			time_alpha = time_offset / time_diff;
		}
	}
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
	{
		// There is a transition out, so work out if the time position is during it
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			start_time += this_layer_data->transition_in_duration;
		start_time += this_layer_data->duration;
		time_offset = time_position - start_time;
		end_time = start_time + this_layer_data->transition_out_duration;
		if ((time_position > start_time) && (time_position <= end_time))
		{
			// The time position is during a transition out
			time_diff = end_time - start_time;
			time_alpha = 1 - (time_offset / time_diff);
		}
	}

	// Calculate the height and width scaling values for the requested layer size
	gdk_drawable_get_size(GDK_PIXMAP(incoming_pixmap), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) pixmap_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) pixmap_width / (gfloat) project_width;

	// Create a cairo drawing context
	cairo_context = gdk_cairo_create(GDK_PIXMAP(incoming_pixmap));

	// Determine the type of layer we're dealing with (we ignore background layers)
	switch (this_layer_data->object_type)
	{
		case TYPE_GDK_PIXBUF:

			// * We're processing an image layer *

			// Simplify pointers
			this_image_data = (layer_image *) this_layer_data->object_data;

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Position the pixbuf pattern at the desired x,y coordinates
			cairo_matrix_init_translate(&image_matrix, -(time_x), -(time_y));
			cairo_pattern_set_matrix(this_image_data->cairo_pattern, &image_matrix);

			// Set the correct scale for the pattern
			cairo_scale(cairo_context, scaled_width_ratio, scaled_height_ratio);

			// Set the pattern as the source
			cairo_set_source(cairo_context, this_image_data->cairo_pattern);

			// Draw the source onto our backing pixmap
			cairo_rectangle(cairo_context, time_x, time_y, this_image_data->width, this_image_data->height);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			return;

		case TYPE_MOUSE_CURSOR:

			// * Composite the mouse pointer image onto the backing pixmap *

			// If the mouse pointer image hasn't been loaded then we skip this layer
			if (NULL == mouse_ptr_pixbuf)
				return;

			// Simplify pointers
			width = ((layer_mouse *) this_layer_data->object_data)->width;
			height = ((layer_mouse *) this_layer_data->object_data)->height;

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Draw the mouse cursor
			cairo_scale(cairo_context, scaled_width_ratio, scaled_height_ratio);
			gdk_cairo_set_source_pixbuf(cairo_context, GDK_PIXBUF(mouse_ptr_pixbuf), time_x, time_y);
			cairo_rectangle(cairo_context, time_x, time_y, width, height);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			return;

		case TYPE_EMPTY:

			// Empty layer, just return
			return;

		case TYPE_TEXT:

			// * Draw the text layer *

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Simplify pointers
			text_object = (layer_text *) this_layer_data->object_data;
			text_buffer = text_object->text_buffer;

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

			// * Draw the background for the text layer *

			// Create the background fill
			cairo_set_operator(cairo_context, CAIRO_OPERATOR_SOURCE);
			cairo_set_source_rgb(cairo_context, 1, 1, 0.8);
			cairo_rectangle(cairo_context, x_offset, y_offset, width, height);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha);

			// Create the background border
			cairo_set_operator(cairo_context, CAIRO_OPERATOR_OVER);
			cairo_set_source_rgba(cairo_context, 0, 0, 0, time_alpha);
			cairo_set_line_width(cairo_context, 2.0);
			cairo_set_line_join(cairo_context, CAIRO_LINE_JOIN_ROUND);
			cairo_set_line_cap(cairo_context, CAIRO_LINE_CAP_ROUND);
			cairo_rectangle(cairo_context, x_offset, y_offset, width, height);
			cairo_stroke(cairo_context);

			// * Draw the text string itself *

			// Set the desired font foreground color
			text_colour = &((layer_text *) this_layer_data->object_data)->text_color;
			text_red = ((gfloat) text_colour->red) / 65536;
			text_green = ((gfloat) text_colour->green) / 65536;
			text_blue = ((gfloat) text_colour->blue) / 65536;
			cairo_set_source_rgba(cairo_context, text_red, text_green, text_blue, time_alpha);

			// Loop around, drawing lines of text
			text_height = 0;
			text_top = y_offset + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);
			for (line_counter = 0; line_counter < num_lines; line_counter++)
			{
				// Retrieve a line of text
				gtk_text_buffer_get_iter_at_line(text_buffer, &text_start, line_counter);
				text_end = text_start;
				gtk_text_iter_forward_to_line_end(&text_end);
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

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			break;

		case TYPE_HIGHLIGHT:

			// * Draw the highlight *

			// Calculate how much of the highlight will fit onto the pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_highlight *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_highlight *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixmap_width)
			{
				width = pixmap_width - x_offset;
			}
			if ((y_offset + height) > pixmap_height)
			{
				height = pixmap_height - y_offset;
			}

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Choose an operator for constructing the border
			cairo_set_operator(cairo_context, CAIRO_OPERATOR_OVER);

			// Create the border
			cairo_set_source_rgba(cairo_context, 0, 0.25, 0, 0.60 * time_alpha);
			cairo_set_line_width(cairo_context, 2.0);
			cairo_set_line_join(cairo_context, CAIRO_LINE_JOIN_ROUND);
			cairo_set_line_cap(cairo_context, CAIRO_LINE_CAP_ROUND);
			cairo_move_to(cairo_context, x_offset, y_offset);
			cairo_line_to(cairo_context, x_offset + width, y_offset);
			cairo_line_to(cairo_context, x_offset + width, y_offset + height);
			cairo_line_to(cairo_context, x_offset, y_offset + height);
			cairo_line_to(cairo_context, x_offset, y_offset);
			cairo_stroke(cairo_context);

			// Choose an operator for constructing the fill
			cairo_set_operator(cairo_context, CAIRO_OPERATOR_SOURCE);

			// Create the fill
			cairo_set_source_rgb(cairo_context, 0, 0.60, 0);
			cairo_rectangle(cairo_context, x_offset + 1, y_offset + 1, width - 2, height - 2);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, 0.40 * time_alpha);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			break;		

		default:
			display_warning("Error ED33: Unknown layer type");		
	}

	// The cairo drawing context is no longer needed, so free it
	cairo_destroy(cairo_context);
}

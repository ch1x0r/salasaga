/*
 * $Id$
 *
 * Salasaga: Function to process the data for a layer, adding it to the main image being built 
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

// Text padding defines (in pixels) 
#define TEXT_BORDER_PADDING_WIDTH 4
#define TEXT_BORDER_PADDING_HEIGHT 4


void compress_layers_inner(layer *this_layer_data, GdkPixmap *incoming_pixmap, gfloat time_position)
{
	// Local variables
	cairo_t					*cairo_context;			// Cairo drawing context
	gfloat					end_time;				// Time in seconds of the layer objects finish time
	gint					finish_x;				// X position at the layer objects finish time
	gint					finish_y;				// Y position at the layer objects finish time 
	GdkGC					*graphics_context;		// GDK graphics context
	gint					height;					//
	gint					pixmap_height;			// Receives the height of a given pixmap
	gint					pixmap_width;			// Receives the width of a given pixmap
	gfloat					scaled_height_ratio;	// Used to calculate a vertical scaling ratio 
	gfloat					scaled_width_ratio;		// Used to calculate a horizontal scaling ratio
	gfloat					start_time;				// Time in seconds of the layer objects start time
	gint					start_x;				// X position at the layer objects start time
	gint					start_y;				// Y position at the layer objects start time
	GdkColor				*text_colour;			// Pointer to the foreground colour of the text
	gfloat					text_blue;				// Blue component of text fg colour
	GtkTextIter				text_end;				// The end position of the text buffer
	cairo_text_extents_t	text_extents;			// Meta information about an onscreen text string
	gfloat					text_green;				// Green component of text fg colour
	gint					text_left;				// Pixel number onscreen for the left side of text
	gfloat					text_red;				// Red component of text fg colour
	gint					text_top;				// Pixel number onscreen for the top of text
	GtkTextIter				text_start;				// The start position of the text buffer
	gchar					*text_string;			// The text string to be displayed
	gfloat					time_alpha = 1.0;		// Alpha value to use at our desired point in time (defaulting to 1.0 = fall opacity)
	gfloat					time_offset;
	gint					time_x;					// Unscaled X position of the layer at our desired point in time
	gint					time_y;					// Unscaled Y position of the layer at our desired point in time
	gfloat					time_diff;				// Used when calculating the object position at the desired point in time
	GdkColormap				*tmp_colormap;			// Temporary colormap
	GdkPixmap				*tmp_pixmap;			// GDK Pixmap
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
		if ((time_position > start_time) && (time_position < end_time))
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
		if ((time_position > start_time) && (time_position < end_time))
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

			// Calculate how much of the source image will fit onto the backing pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_image *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_image *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixmap_width)
			{
				width = pixmap_width - x_offset;
			}
			if ((y_offset + height) > pixmap_height)
			{
				height = pixmap_height - y_offset;
			}
/*
			// Draw the image onto the backing pixbuf
			gdk_pixbuf_composite(((layer_image *) this_layer_data->object_data)->image_data,	// Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			scaled_width_ratio,				// Scale X
			scaled_height_ratio,			// Scale Y
			GDK_INTERP_TILES,				// Scaling type
			time_alpha);					// Alpha
*/
			return;

		case TYPE_MOUSE_CURSOR:

			// * Composite the mouse pointer image onto the backing pixmap *

			// If the mouse pointer image wasn't able to be loaded then we skip this layer, as the compositing wont work with it
			if (NULL == mouse_ptr_pixbuf)
			{
				return;
			}

			// Calculate how much of the source image will fit onto the backing pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_mouse *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_mouse *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixmap_width)
			{
				width = pixmap_width - x_offset;
			}
			if ((y_offset + height) > pixmap_height)
			{
				height = pixmap_height - y_offset;
			}
/*
			// Draw the mouse pointer onto the backing pixbuf
			gdk_pixbuf_composite(mouse_ptr_pixbuf,  // Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			scaled_width_ratio,				// Scale X
			scaled_height_ratio,			// Scale Y
			GDK_INTERP_TILES,				// Scaling type
			time_alpha);					// Alpha
*/
			return;

		case TYPE_EMPTY:

			// Empty layer, just return
			return;

		case TYPE_TEXT:

			// * Draw the text layer *

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Retrieve the text to display
			gtk_text_buffer_get_bounds(((layer_text *) this_layer_data->object_data)->text_buffer, &text_start, &text_end);
			text_string = gtk_text_buffer_get_text(((layer_text *) this_layer_data->object_data)->text_buffer, &text_start, &text_end, FALSE);

			// Determine the on screen size of the text string itself
			cairo_set_font_size(cairo_context, ((layer_text *) this_layer_data->object_data)->font_size * scaled_width_ratio);
			cairo_text_extents(cairo_context, text_string, &text_extents);

			// Calculate the text object (including background) offsets and sizing
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = CLAMP(text_extents.width + (TEXT_BORDER_PADDING_WIDTH * 2 * scaled_width_ratio),
						0, pixmap_width - x_offset - (TEXT_BORDER_PADDING_WIDTH * 2 * scaled_width_ratio) - 1);
			height = CLAMP(text_extents.height + (TEXT_BORDER_PADDING_HEIGHT * 2 * scaled_height_ratio),
						0, pixmap_height - y_offset - (TEXT_BORDER_PADDING_HEIGHT * 2 * scaled_height_ratio) - 1);

			// Store the rendered width of the text object with the layer itself, for use by bounding box code
			((layer_text *) this_layer_data->object_data)->rendered_width = width / scaled_width_ratio;
			((layer_text *) this_layer_data->object_data)->rendered_height = height / scaled_height_ratio;

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

			// Move to the desired text location
			text_left = x_offset;
			text_top = y_offset + text_extents.height + (TEXT_BORDER_PADDING_HEIGHT * scaled_height_ratio);
			cairo_move_to(cairo_context, text_left, text_top);

			// Set the desired font foreground color
			text_colour = &((layer_text *) this_layer_data->object_data)->text_color;
			text_red = ((gfloat) text_colour->red) / 65536;
			text_green = ((gfloat) text_colour->green) / 65536;
			text_blue = ((gfloat) text_colour->blue) / 65536;
			cairo_set_source_rgba(cairo_context, text_red, text_green, text_blue, time_alpha);

			// Draw the font text
			cairo_show_text(cairo_context, text_string);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			break;

		case TYPE_HIGHLIGHT:

			// * Draw the highlight *

			// Calculate how much of the highlight will fit onto the backing pixmap
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

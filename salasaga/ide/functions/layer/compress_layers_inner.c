/*
 * $Id$
 *
 * Salasaga: Function to process the data for a layer, adding it to the main image being built
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../cairo/render_text_string.h"
#include "../dialog/display_warning.h"
#include "../preference/project_preferences.h"
#include "get_layer_position.h"


void compress_layers_inner(layer *this_layer_data, GdkPixmap *incoming_pixmap, gfloat time_position)
{
	// Local variables
	gfloat					blue_component;			// Blue component of a colour
	cairo_t					*cairo_context;			// Cairo drawing context
	gdouble					cairo_pos_x = 0;		// Used for positioning where cairo will draw, in text layers
	gdouble					cairo_pos_y = 0;		// Used for positioning where cairo will draw, in text layers
	gfloat					end_time;				// Time in seconds of the layer objects finish time
	gfloat					green_component;		// Green component of a colour
	gint					height;					//
	cairo_matrix_t			image_matrix;			// Transformation matrix used to position a cairo pattern
	GtkAllocation			layer_positions;		// Offset and dimensions for a given layer object
	GString					*message;				// Used to construct message strings
	gint					pixmap_height;			// Receives the height of a given pixmap
	gint					pixmap_width;			// Receives the width of a given pixmap
	gfloat					red_component;			// Red component of a colour
	gboolean				return_code_gbool;		// Receives gboolean return codes
	gfloat					scaled_height_ratio;	// Used to calculate a vertical scaling ratio
	gfloat					scaled_width_ratio;		// Used to calculate a horizontal scaling ratio
	GdkColor				*selected_colour;		// Pointer to a foreground colour
	gfloat					start_time;				// Time in seconds of the layer objects start time
	layer_text				*text_object;			// Pointer to our object text data
	layer_image				*this_image_data;		// Pointer to image layer data
	gfloat					time_alpha = 1.0;		// Alpha value to use at our desired point in time (defaulting to 1.0 = full opacity)
	gint					width;					//
	gint					x_offset;				// X coordinate of the object at the desired point in time
	gint					y_offset;				// Y coordinate of the object at the desired point in time


	// Is this layer invisible, or is it a background layer?
	if ((FALSE == this_layer_data->visible) || (TRUE == this_layer_data->background))
	{
		// We don't need to process this layer
		return;
	}

	// If the layer isn't visible at the requested time, we don't need to proceed
	start_time = this_layer_data->start_time;
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;
	if ((time_position < start_time) || (time_position > end_time))
	{
		return;
	}

	// Retrieve the layer position and alpha for the given point in time
	return_code_gbool = get_layer_position(&layer_positions, this_layer_data, time_position, &time_alpha);
	if (FALSE == return_code_gbool)
		return;

	// Calculate the height and width scaling values for the requested layer size
	gdk_drawable_get_size(GDK_PIXMAP(incoming_pixmap), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) pixmap_height / (gfloat) get_project_height();
	scaled_width_ratio = (gfloat) pixmap_width / (gfloat) get_project_width();

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
			cairo_matrix_init_translate(&image_matrix, -(layer_positions.x), -(layer_positions.y));
			cairo_pattern_set_matrix(this_image_data->cairo_pattern, &image_matrix);

			// Set the correct scale for the pattern
			cairo_scale(cairo_context, scaled_width_ratio, scaled_height_ratio);

			// Set the pattern as the source
			cairo_set_source(cairo_context, this_image_data->cairo_pattern);

			// Draw the source onto our backing pixmap
			cairo_rectangle(cairo_context, layer_positions.x, layer_positions.y, this_image_data->width, this_image_data->height);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			// The cairo drawing context is no longer needed, so free it
			cairo_destroy(cairo_context);

			return;

		case TYPE_MOUSE_CURSOR:

			// * Composite the mouse pointer image onto the backing pixmap *

			// If the mouse pointer image hasn't been loaded then we skip this layer
			if (NULL == get_mouse_ptr_pixbuf())
			{
				// The cairo drawing context is no longer needed, so free it
				cairo_destroy(cairo_context);

				return;
			}

			// Simplify pointers
			width = ((layer_mouse *) this_layer_data->object_data)->width;
			height = ((layer_mouse *) this_layer_data->object_data)->height;

			// Save the existing cairo state before making changes (i.e. clip region)
			cairo_save(cairo_context);

			// Draw the mouse cursor
			cairo_scale(cairo_context, scaled_width_ratio, scaled_height_ratio);
			gdk_cairo_set_source_pixbuf(cairo_context, GDK_PIXBUF(get_mouse_ptr_pixbuf()), layer_positions.x, layer_positions.y);
			cairo_rectangle(cairo_context, layer_positions.x, layer_positions.y, width, height);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			// The cairo drawing context is no longer needed, so free it
			cairo_destroy(cairo_context);

			return;

		case TYPE_EMPTY:

			// * Empty layer, just return *

			// The cairo drawing context is no longer needed, so free it
			cairo_destroy(cairo_context);

			return;

		case TYPE_TEXT:

			// * Draw the text layer *

			// Simplify pointers
			text_object = (layer_text *) this_layer_data->object_data;

			// Save the existing cairo state before we use it
			cairo_save(cairo_context);

			// Render the text layer on screen
			cairo_pos_x = layer_positions.x * scaled_width_ratio;
			cairo_pos_y = layer_positions.y * scaled_height_ratio;
			render_text_string(cairo_context, text_object, scaled_width_ratio, scaled_height_ratio, cairo_pos_x, cairo_pos_y, time_alpha, TRUE);

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);
			break;

		case TYPE_HIGHLIGHT:

			// * Draw the highlight *

			// Calculate how much of the highlight will fit onto the pixmap
			x_offset = layer_positions.x * scaled_width_ratio;
			y_offset = layer_positions.y * scaled_height_ratio;
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
			selected_colour = &((layer_highlight *) this_layer_data->object_data)->border_colour;
			red_component = ((gfloat) selected_colour->red) / 65536;
			green_component = ((gfloat) selected_colour->green) / 65536;
			blue_component = ((gfloat) selected_colour->blue) / 65536;
			cairo_set_source_rgba(cairo_context, red_component, green_component, blue_component,
					time_alpha * (((layer_highlight *) this_layer_data->object_data)->opacity / 100.0));
			cairo_set_line_width(cairo_context, ((layer_highlight *) this_layer_data->object_data)->border_width);
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
			selected_colour = &((layer_highlight *) this_layer_data->object_data)->fill_colour;
			red_component = ((gfloat) selected_colour->red) / 65536;
			green_component = ((gfloat) selected_colour->green) / 65536;
			blue_component = ((gfloat) selected_colour->blue) / 65536;
			cairo_set_source_rgb(cairo_context, red_component, green_component, blue_component);
			cairo_rectangle(cairo_context, x_offset + 1, y_offset + 1, width - 2, height - 2);
			cairo_clip(cairo_context);
			cairo_paint_with_alpha(cairo_context, time_alpha * (((layer_highlight *) this_layer_data->object_data)->opacity / 100.0));

			// Restore the cairo state to the way it was
			cairo_restore(cairo_context);

			break;

		default:
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED33: %s", _("Error"), _("Unknown layer type."));
			display_warning(message->str);
			g_string_free(message, TRUE);
	}

	// The cairo drawing context is no longer needed, so free it
	cairo_destroy(cairo_context);
}

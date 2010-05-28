/*
 * $Id$
 *
 * Salasaga: Draw the layer duration onto the display buffer
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "time_line_get_left_border_width.h"


gboolean time_line_internal_draw_layer_duration(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	const GdkColor		colour_fade = {0, (160 << 8), (160 << 8), (190 << 8) };
	const GdkColor		colour_fully_visible = {0, (200 << 8), (200 << 8), (230 << 8) };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static GdkGC		*display_buffer_gc = NULL;
	gint				layer_height;
	layer				*layer_data;
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				layer_width;
	gint				layer_x;
	gint				layer_y;
	gint				left_border;


	// Initialisation
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_COLORMAP(colourmap));
	}
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer_bot_right));
	}
	left_border = 0;// time_line_get_left_border_width(priv);

	// Select the layer we're working with
	layer_pointer = get_current_slide_layers_pointer();
	layer_pointer = g_list_first(layer_pointer);
	layer_data = g_list_nth_data(layer_pointer, layer_number);

	// Set the height related variables
	layer_y =  (layer_number * priv->row_height) + 2;
	layer_height = priv->row_height - 3;

	// Check if there's a fade in transition for this layer
	if (TRANS_LAYER_FADE == layer_data->transition_in_type)
	{
		// Draw the fade in
		layer_x = left_border + (layer_data->start_time * time_line_get_pixels_per_second()) + 1;
		layer_width = (layer_data->transition_in_duration * time_line_get_pixels_per_second());
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fade);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), FALSE,
				layer_x, layer_y, layer_width, layer_height - 1);

		// Draw the fully visible duration
		layer_x = left_border + ((layer_data->start_time + layer_data->transition_in_duration) * time_line_get_pixels_per_second()) + 1;
		layer_width = (layer_data->duration * time_line_get_pixels_per_second());
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height-1);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), FALSE,
				layer_x - 1, layer_y, layer_width, layer_height);
	} else
	{
		// There's no fade in transition for this layer
		layer_x = left_border + (layer_data->start_time * time_line_get_pixels_per_second()) + 1;
		layer_width = (layer_data->duration * time_line_get_pixels_per_second());
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), FALSE,
				layer_x, layer_y, layer_width - 1, layer_height - 1);
	}

	// Check if there's a fade out transition for this layer
	if (TRANS_LAYER_FADE == layer_data->transition_out_type)
	{
		// Draw the fade out
		layer_x += (layer_data->duration * time_line_get_pixels_per_second());
		layer_width = (layer_data->transition_out_duration * time_line_get_pixels_per_second());
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fade);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_GC(display_buffer_gc), FALSE,
				layer_x - 1, layer_y, layer_width, layer_height - 1);
	}

	return TRUE;
}

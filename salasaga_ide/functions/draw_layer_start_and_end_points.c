/*
 * $Id$
 *
 * Salasaga: Draws the start and end points for a layer 
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
#include "../salasaga_types.h"
#include "../externs.h"
#include "widgets/time_line.h"


gboolean draw_layer_start_and_end_points()
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	const GdkColor		colour_green = {0, 0, 40000, 0};
	const GdkColor		colour_red = {0, 40000, 0, 0 };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	gint				finish_mid_point_x;
	gint				finish_mid_point_y;
	gint				finish_x;					// X position at the layer objects finish time
	gint				finish_y;					// Y position at the layer objects finish time 
	GList				*layer_pointer;
	gint				old_start_x = 0;			// Value used in the previous run
	gint				old_start_y = 0;			// Value used in the previous run
	gint				old_width = 0;				// Value used in the previous run
	gint				old_height = 0;				// Value used in the previous run
	gint				pixmap_height;				// Receives the height of a given pixmap
	gint				pixmap_width;				// Receives the width of a given pixmap
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gint				selected_row;				// Number of the row selected in the time line
	gint				start_mid_point_x;
	gint				start_mid_point_y;
	gint				start_x;					// X position at the layer objects start time
	gint				start_y;					// Y position at the layer objects start time
	layer 				*this_layer_data;			// Pointer to the data for the selected layer
	slide				*this_slide_data;			// Pointer to the data for the selected slide
	static GdkGC		*widget_gc = NULL;


	// Determine which row is selected in the time line
	this_slide_data = (slide *) current_slide->data;
	selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);
	layer_pointer = g_list_first(this_slide_data->layers);
	this_layer_data = g_list_nth_data(layer_pointer, selected_row);

	// Is this layer invisible, or is it a background layer?
	if ((FALSE == this_layer_data->visible) || (TRUE == this_layer_data->background))
	{
		// We don't need to process this layer
		return TRUE;
	}

	// Initialise colourmap and graphic context
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(main_drawing_area->window), GDK_COLORMAP(colourmap));
	}
	if (NULL == widget_gc)
	{
		widget_gc = gdk_gc_new(GDK_DRAWABLE(main_drawing_area->window));
	}

	// Refresh the area covered by the old end points
	gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc),
		GDK_PIXMAP(front_store), old_start_x, old_start_y, old_start_x, old_start_y, old_width, old_height);

	// Calculate the height and width scaling values for the requested layer
	gdk_drawable_get_size(GDK_PIXMAP(front_store), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) pixmap_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) pixmap_width / (gfloat) project_width;

	// Calculate start and end points
	finish_x = (this_layer_data->x_offset_finish * scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
	finish_y = (this_layer_data->y_offset_finish * scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;
	start_x = (this_layer_data->x_offset_start * scaled_width_ratio) + END_POINT_HORIZONTAL_OFFSET;
	start_y = (this_layer_data->y_offset_start * scaled_height_ratio) + END_POINT_VERTICAL_OFFSET;

	start_mid_point_x = start_x + (END_POINT_WIDTH / 2);
	start_mid_point_y = start_y + (END_POINT_HEIGHT / 2);
	finish_mid_point_x = finish_x + (END_POINT_WIDTH / 2);
	finish_mid_point_y = finish_y + (END_POINT_HEIGHT / 2);

	// Store the end points, for the next refresh
	old_start_x = start_x;
	old_start_y = start_y;
	old_width = finish_x + END_POINT_WIDTH;
	old_height = finish_y + END_POINT_HEIGHT;

	// Draw a line joining the start and end points
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_gc_set_line_attributes(GDK_GC(widget_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_draw_line(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc),
			start_mid_point_x, start_mid_point_y, finish_mid_point_x, finish_mid_point_y);

	// Draw end point
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_red);
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc), TRUE,
			finish_x, finish_y, END_POINT_WIDTH, END_POINT_HEIGHT);
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc), FALSE,
			finish_x, finish_y, END_POINT_WIDTH, END_POINT_HEIGHT);

	// Draw start point
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_green);
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc), TRUE,
			start_x, start_y, END_POINT_WIDTH, END_POINT_HEIGHT);
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(widget_gc), FALSE,
			start_x, start_y, END_POINT_WIDTH, END_POINT_HEIGHT);

	return TRUE;
}

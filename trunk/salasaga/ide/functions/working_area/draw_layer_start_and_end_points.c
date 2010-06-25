/*
 * $Id$
 *
 * Salasaga: Draws the start and end points for a layer
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../preference/project_preferences.h"
#include "../time_line/time_line_get_selected_layer_num.h"


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
	GdkRectangle		line_clip_region;			// Used as a clip mask region
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
	GtkWidget			*temp_widget;				// Temporarily holds a pointer to the main drawing area widget
	layer 				*this_layer_data;			// Pointer to the data for the selected layer
	slide				*this_slide_data;			// Pointer to the data for the selected slide
	static GdkGC		*widget_gc = NULL;


	// Determine which row is selected in the time line
	this_slide_data = get_current_slide_data();
	selected_row = time_line_get_selected_layer_num(this_slide_data->timeline_widget);
	layer_pointer = g_list_first(this_slide_data->layers);
	this_layer_data = g_list_nth_data(layer_pointer, selected_row);
	temp_widget = get_main_drawing_area();

	// If the layer data isn't accessible, then don't run this function
	if (NULL == this_layer_data)
	{
		return TRUE;
	}

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
		gdk_drawable_set_colormap(GDK_DRAWABLE(temp_widget->window), GDK_COLORMAP(colourmap));
	}
	if (NULL == widget_gc)
	{
		widget_gc = gdk_gc_new(GDK_DRAWABLE(temp_widget->window));
	}

	// Refresh the area covered by the old end points
	gdk_draw_drawable(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc),
		GDK_PIXMAP(get_front_store()), old_start_x, old_start_y, old_start_x, old_start_y, old_width, old_height);

	// Calculate the height and width scaling values for the requested layer
	gdk_drawable_get_size(GDK_PIXMAP(get_front_store()), &pixmap_width, &pixmap_height);
	scaled_height_ratio = (gfloat) pixmap_height / (gfloat) get_project_height();
	scaled_width_ratio = (gfloat) pixmap_width / (gfloat) get_project_width();

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
	line_clip_region.x = 1;
	line_clip_region.y = 1;
	line_clip_region.width = (gint) pixmap_width - 2;
	line_clip_region.height = (gint) pixmap_height - 2;
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_gc_set_line_attributes(GDK_GC(widget_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_clip_rectangle(GDK_GC(widget_gc), &line_clip_region);
	gdk_draw_line(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc),
			start_mid_point_x, start_mid_point_y, finish_mid_point_x, finish_mid_point_y);

	// Draw end point
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_red);
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc), TRUE,
			finish_x, finish_y, END_POINT_WIDTH, END_POINT_HEIGHT);
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc), FALSE,
			finish_x, finish_y, END_POINT_WIDTH, END_POINT_HEIGHT);

	// Draw start point
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_green);
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc), TRUE,
			start_x, start_y, END_POINT_WIDTH, END_POINT_HEIGHT);
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(temp_widget->window), GDK_GC(widget_gc), FALSE,
			start_x, start_y, END_POINT_WIDTH, END_POINT_HEIGHT);

	return TRUE;
}

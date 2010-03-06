/*
 * $Id$
 *
 * Salasaga: Function that redraws the workspace
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../layer/compress_layers.h"
#include "../widgets/time_line/time_line_get_cursor_position.h"
#include "../widgets/time_line/time_line_new.h"
#include "../widgets/time_line/time_line_set_stored_slide_duration.h"


void draw_workspace(void)
{
	// Local variables
	gfloat				cursor_position;
	gint				front_store_height;
	gint				front_store_width;
	const GdkColor		line_fg_colour = { 0, 0x00, 0x00, 0x00 };
	static GdkGC		*line_gc = NULL;
	GdkSegment			lines[4];
	slide				*current_slide_data;
	GdkRectangle		tmp_rectangle;


	// If the current slide hasn't been initialised, or there is no project active don't run this function
	if ((NULL == current_slide) || (FALSE == get_project_active()))
	{
		return;
	}

	// If the slide doesn't have a timeline widget constructed for it yet, then make one
	current_slide_data = current_slide->data;
	if (NULL == current_slide_data->timeline_widget)
	{
		// Construct the widget used to display the slide in the timeline
		current_slide_data->timeline_widget = time_line_new();
		time_line_set_stored_slide_duration(current_slide_data->timeline_widget, current_slide_data->duration);
	}

	// Recalculate the size of the working area
	front_store_height = working_height;
	front_store_width = working_width;

	// Get the current time line cursor position
	cursor_position = time_line_get_cursor_position(current_slide_data->timeline_widget);

	// Create a new front store from the current slide
	if (NULL != front_store)
	{
		g_object_unref(GDK_PIXMAP(front_store));
		front_store = NULL;
	}
	front_store = compress_layers(current_slide, cursor_position, working_width, working_height);
	if (NULL == front_store)
	{
		return;
	}

	// Make a 1 pixel border around the front store, to separate it visually from its background
	if (NULL == line_gc)
	{
		line_gc = gdk_gc_new(GDK_DRAWABLE(front_store));
		gdk_gc_set_rgb_fg_color(line_gc, &line_fg_colour);
	}
	lines[0].x1 = 0;
	lines[0].y1 = 0;
	lines[0].x2 = front_store_width - 1;
	lines[0].y2 = 0;
	lines[1].x1 = front_store_width - 1;
	lines[1].y1 = 0;
	lines[1].x2 = front_store_width - 1;
	lines[1].y2 = front_store_height - 1;
	lines[2].x1 = front_store_width - 1;
	lines[2].y1 = front_store_height - 1;
	lines[2].x2 = 0;
	lines[2].y2 = front_store_height - 1;
	lines[3].x1 = 0;
	lines[3].y1 = front_store_height - 1;
	lines[3].x2 = 0;
	lines[3].y2 = 0;
	gdk_draw_segments(GDK_DRAWABLE(front_store), line_gc, lines, 4);

	// Tell the window system to redraw the working area
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = front_store_width;
	tmp_rectangle.height = front_store_height;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

	// Update the workspace
	gtk_widget_queue_draw(GTK_WIDGET(main_drawing_area));
}

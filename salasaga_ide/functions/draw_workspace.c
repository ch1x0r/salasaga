/*
 * $Id$
 *
 * Salasaga: Function that redraws the workspace 
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "compress_layers.h"
#include "widgets/time_line.h"


void draw_workspace(void)
{
	// Local variables
	gfloat				cursor_position;
	static GdkColormap	*front_store_colourmap = NULL;  // Colormap used for the front store
	gint				front_store_height;
	gint				front_store_width;
	const GdkColor		line_fg_colour = { 0, 0x00, 0x00, 0x00 };
	static GdkGC		*line_gc = NULL;
	GdkSegment			lines[4];
	GdkRectangle		tmp_rectangle;


	// If the current slide hasn't been initialised, or there is no project active don't run this function
	if ((NULL == current_slide) || (FALSE == project_active))
	{
		return;
	}

	// Recalculate the size of the working area
	front_store_height = working_height;
	front_store_width = working_width;

	// Get the current time line cursor position
	cursor_position = time_line_get_cursor_position(((slide *) current_slide->data)->timeline_widget);

	// Create a new backing store from the current slide
	if (NULL != backing_store)
		g_object_unref(GDK_PIXBUF(backing_store));
	backing_store = compress_layers(current_slide, cursor_position, working_width - 2, working_height - 2);

	// Create the colourmap if needed
	if (NULL == front_store_colourmap)
	{
		front_store_colourmap = gdk_colormap_get_system();
	}

	// Create the front store if needed
	if (NULL == front_store)
	{
		// We don't have a front store yet, so we create a new one
		front_store = gdk_pixmap_new(NULL, front_store_width, front_store_height, front_store_colourmap->visual->depth);
		gdk_drawable_set_colormap(GDK_DRAWABLE(front_store), GDK_COLORMAP(front_store_colourmap));
	}

	// Copy the backing store to the front store
	gdk_draw_pixbuf(GDK_DRAWABLE(front_store), NULL, GDK_PIXBUF(backing_store), 0, 0, 1, 1, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);

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

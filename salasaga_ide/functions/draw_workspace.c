/*
 * $Id$
 *
 * Flame Project: Function that redraws the workspace 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "calculate_object_boundaries.h"
#include "compress_layers.h"


void draw_workspace(void)
{
	// Local variables
	static GdkColormap		*front_store_colourmap = NULL;  // Colormap used for the front store
	gint					front_store_height;
	gint					front_store_width;
	const GdkColor			line_fg_colour = { 0, 0x00, 0x00, 0x00 };
	static GdkGC			*line_gc = NULL;
	GdkSegment				lines[4];
	GdkRectangle			tmp_rectangle;


	// If the current slide hasn't been initialised, don't run this function
	if (NULL == current_slide)
	{
		return;
	}

	// Recalculate the size of the working area plus 1 pixel border
	front_store_height = working_height;
	front_store_width = working_width;

	// Create a new backing store from the current slide
	if (NULL != backing_store)
		g_object_unref(backing_store);
	backing_store = compress_layers(current_slide, working_width - 2, working_height - 2);

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

	// Update the collision detection boundaries
//	calculate_object_boundaries();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.8  2008/03/05 12:43:19  vapour
 * Updated to support double buffering.
 *
 * Revision 1.7  2008/02/27 13:47:50  vapour
 * The one pixel black border around the drawing area should now always be drawn clearly.
 *
 * Revision 1.6  2008/02/11 12:01:38  vapour
 * Swapped around a few lines, to make more sense of program flow.
 *
 * Revision 1.5  2008/02/07 12:35:27  vapour
 * Reworked this function to be more efficient.
 *
 * Revision 1.4  2008/02/04 16:48:01  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.3  2008/01/15 16:19:03  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/04 19:51:11  vapour
 * Began adjusting the includes in each function to only call the ones it needs, in order to dramatically speed up recompilation time for developers.
 *
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

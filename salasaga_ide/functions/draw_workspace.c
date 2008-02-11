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
	static GdkColormap		*backing_colormap = NULL;  // Colormap used for the backing store
	gint					backing_height;
	gint					backing_width;
	const GdkColor			line_fg_color = { 0, 0x00, 0x00, 0x00 };
	static GdkGC			*line_gc = NULL;
	GdkSegment				lines[] = { {0, 0, backing_width, 0},
										{backing_width, 0, backing_width, backing_height},
										{backing_width, backing_height, 0, backing_height},
										{0, backing_height, 0, 0} };

	GdkPixbuf				*tmp_pixbuf;
	GdkRectangle			tmp_rectangle;


	// If the current slide hasn't been initialised, don't run this function
	if (NULL == current_slide)
	{
		return;
	}

	// Initialise various things
	backing_height = working_height + 2;
	backing_width = working_width + 2;

	// Create a new pixbuf with all the layers of the current slide
	tmp_pixbuf = compress_layers(current_slide, working_width, working_height);

	// Create the colormap if needed
	if (NULL == backing_colormap)
	{
		backing_colormap = gdk_colormap_get_system();
	}

	// Create the backing store if needed
	if (NULL == backing_store)
	{
		// We don't have a backing store yet, so we create a new one
		backing_store = gdk_pixmap_new(NULL, backing_width, backing_height, backing_colormap->visual->depth);
		gdk_drawable_set_colormap(GDK_DRAWABLE(backing_store), GDK_COLORMAP(backing_colormap));
	}

	// Copy the pixbuf to a pixmap
	gdk_draw_pixbuf(GDK_DRAWABLE(backing_store), NULL, GDK_PIXBUF(tmp_pixbuf), 0, 0, 1, 1, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);

	// Make a 1 pixel border around the pixbuf, to separate it visually from its background
	if (NULL == line_gc)
	{
		line_gc = gdk_gc_new(GDK_DRAWABLE(backing_store));
		gdk_gc_set_rgb_fg_color(line_gc, &line_fg_color);
	}
	gdk_draw_segments(GDK_DRAWABLE(backing_store), line_gc, lines, 4);

	// Tell the window system to redraw the working area
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = backing_width;
	tmp_rectangle.height = backing_height;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

	// Update the workspace
	gtk_widget_queue_draw(GTK_WIDGET(main_drawing_area));

	// Free allocated memory
	if (NULL != tmp_pixbuf)
		g_object_unref(tmp_pixbuf);

	// Update the collision detection boundaries
	calculate_object_boundaries();
}


/*
 * History
 * +++++++
 * 
 * $Log$
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

/*
 * $Id$
 *
 * Flame Project: Function that redraws the workspace 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void draw_workspace(void)
{
	// Local variables
	GdkPixbuf				*bordered_pixbuf;		// Final pixbuf

	GdkColormap				*tmp_colormap;			// Temporary colormap
	GdkPixbuf				*tmp_pixbuf;			//
	GdkPixmap				*tmp_pixmap;			//
	GdkRectangle				tmp_rectangle;			//


	// If the current slide hasn't been initialised, don't run this function
	if (NULL == current_slide)
	{
		return;
	}

	// Create a new pixbuf with all the layers of the current slide
	tmp_pixmap = backing_store;
	tmp_pixbuf = compress_layers(current_slide, working_width - 2, working_height - 2);

	// Make a 1 pixel edge around the pixbuf, to separate it visually from its background
	bordered_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, working_width, working_height);
	gdk_pixbuf_fill(bordered_pixbuf, 0x000000FF); // fixme4: Drawing four lines around the edges would probably be faster than a bulk memory set
	gdk_pixbuf_copy_area(tmp_pixbuf, 0, 0, working_width - 2, working_height - 2, bordered_pixbuf, 1, 1);

	// Turn the pixbuf into a pixmap, then update the backing store with it
	tmp_colormap = gdk_colormap_get_system();
	// fixme3: Would it be better to just go over the existing backing store memory?
	backing_store = gdk_pixmap_new(NULL, working_width, working_height, tmp_colormap->visual->depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE(backing_store), GDK_COLORMAP(tmp_colormap));
	gdk_draw_pixbuf(GDK_DRAWABLE(backing_store), NULL, GDK_PIXBUF(bordered_pixbuf), 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);

	// Tell the window system to redraw the working area
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = working_width;
	tmp_rectangle.height = working_height;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

	// Update the workspace
	gtk_widget_queue_draw(GTK_WIDGET(main_drawing_area));

	// Free allocated memory
	if (NULL != tmp_pixbuf) g_object_unref(tmp_pixbuf);
	if (NULL != tmp_pixmap)
	{
		g_object_unref(tmp_pixmap);
	}

	// Update the collision detection boundaries
	calculate_object_boundaries();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

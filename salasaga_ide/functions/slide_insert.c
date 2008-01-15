/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Insert from the top menu 
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
#include "create_timeline_slider.h"
#include "create_tooltips.h"


void slide_insert(void)
{
	// Local variables
	GtkTreeIter			film_strip_iter;
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers

	GdkPixbuf			*tmp_gdk_pixbuf;			// Temporary GDK Pixbuf
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	layer				*tmp_layer;				// Temporary layer
	slide				*tmp_slide;				// Temporary slide


	// Create a new, empty slide
	tmp_slide = g_new(slide, 1);
	tmp_slide->layers = NULL;
	tmp_slide->name = NULL;
	tmp_slide->duration = default_slide_length;

	// Allocate a new layer structure for use in the slide
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_EMPTY;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Empty");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->object_data = (GObject *) g_new(layer_empty, 1);
	((layer_empty *) tmp_layer->object_data)->bg_color.red = default_bg_colour.red;
	((layer_empty *) tmp_layer->object_data)->bg_color.green = default_bg_colour.green;
	((layer_empty *) tmp_layer->object_data)->bg_color.blue = default_bg_colour.blue;

	// Create a blank thumbnail using the default background color
	tmp_gdk_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, preview_width, (guint) preview_width * 0.75);
	gdk_pixbuf_fill(tmp_gdk_pixbuf, ((default_bg_colour.red / 255) << 24)
		+ ((default_bg_colour.green / 255) << 16)
		+ ((default_bg_colour.blue / 255) << 8) + 0xff);

	// Add the empty layer to the new slide being created
	tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

	// Create duration image
	layer_pixbuf = NULL;
	layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, 0, 180);

	// Create the List store the slide layer data is kept in
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
									G_TYPE_STRING,  // TIMELINE_NAME
									G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
									GDK_TYPE_PIXBUF,  // TIMELINE_DURATION
									G_TYPE_UINT,  // TIMELINE_X_OFF_START
									G_TYPE_UINT,  // TIMELINE_Y_OFF_START
									G_TYPE_UINT,  // TIMELINE_X_OFF_FINISH
									G_TYPE_UINT);  // TIMELINE_Y_OFF_FINISH
	gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
	gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
					TIMELINE_NAME, tmp_layer->name->str,
					TIMELINE_VISIBILITY, TRUE,
					TIMELINE_DURATION, layer_pixbuf,
					TIMELINE_X_OFF_START, 0,
					TIMELINE_Y_OFF_START, 0,
					TIMELINE_X_OFF_FINISH, 0,
					TIMELINE_Y_OFF_FINISH, 0,
					-1);

	// Add the thumbnail to the new slide structure
	tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));

	// Add the thumbnail to the GtkListView based film strip
	gtk_list_store_append(film_strip_store, &film_strip_iter);  // Acquire an iterator
	gtk_list_store_set(film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(tmp_slide->thumbnail), -1);

	// Mark the tooltip for the slide as not-yet-created
	tmp_slide->tooltip = NULL;

	// Set the timeline widget for the slide to NULL, so we know to create it later on
	tmp_slide->timeline_widget = NULL;

	// If the current slide hasn't been initialised (this is the first slide), then we initialise it
	if (NULL == current_slide)
	{
		slides = g_list_append(slides, tmp_slide);
		current_slide = slides;
	} else
	{
		// Add the newly created slide to the end of the slides
		slides = g_list_append(slides, tmp_slide);
	}

	// Recreate the slide tooltips
	create_tooltips();

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, "Added new slide.");
	gdk_flush();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:37:24  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

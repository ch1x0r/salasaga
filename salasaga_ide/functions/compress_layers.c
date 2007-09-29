/*
 * $Id$
 *
 * Flame Project: Function that gets all of the layers for a slide, then creates a pixel buffer from them 
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


GdkPixbuf *compress_layers(GList *which_slide, guint width, guint height)
{
	// Local variables
	GdkPixbuf			*bg_pixbuf;				// Points to the background layer
	GdkPixbuf			*backing_pixbuf;			// Pixbuf buffer things draw onto
	layer				*layer_data;				// Pointer to the layer data
	GList				*layer_pointer;				// Pointer to the layer GList
	GdkPixbuf			*scaled_pixbuf;				// Smaller pixbuf
	slide				*slide_ref;				// Pointer to the slide data


	// Simplify various pointers
	slide_ref = (slide *) which_slide->data;
	layer_pointer = g_list_last(slide_ref->layers);  // The background layer is always the last (bottom) layer
	layer_data = layer_pointer->data;

	// Check if this is an empty slide
	if (TYPE_EMPTY == layer_data->object_type)
	{
		// This is an empty layer
		bg_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, project_width, project_height);
		gdk_pixbuf_fill(bg_pixbuf, ((((layer_empty *) layer_data->object_data)->bg_color.red / 255) << 24)
			+ ((((layer_empty *) layer_data->object_data)->bg_color.green / 255) << 16)
			+ ((((layer_empty *) layer_data->object_data)->bg_color.blue / 255) << 8) + 0xff);
	} else
	{
		bg_pixbuf = GDK_PIXBUF((GObject *) ((layer_image *) layer_data->object_data)->image_data);
	}

	// Copy the background image onto the drawing buffer (backing_pixbuf)
	backing_pixbuf = gdk_pixbuf_add_alpha(bg_pixbuf, FALSE, 0, 0, 0);

	// Process each layer's data in turn.  We do the reverse() twice so things are drawn in visually correct order
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_reverse(layer_pointer);
	g_list_foreach(layer_pointer, compress_layers_inner, backing_pixbuf);
	layer_pointer = g_list_reverse(layer_pointer);

	// Scale the composited pixbuf down to the desired size
	scaled_pixbuf = gdk_pixbuf_scale_simple(backing_pixbuf, width, height, scaling_quality);

	// Free the various pixbufs, pixmaps, etc
	if (NULL != backing_pixbuf) g_object_unref(backing_pixbuf);
	if (TYPE_EMPTY == layer_data->object_type)
	{
		g_object_unref(bg_pixbuf);
	}

	return scaled_pixbuf;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

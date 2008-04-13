/*
 * $Id$
 *
 * Salasaga: Function that gets all of the layers for a slide, then creates a pixel buffer from them 
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
#include "compress_layers_inner.h"


GdkPixbuf *compress_layers(GList *which_slide, gfloat time_position, guint width, guint height)
{
	// Local variables
	GdkPixbuf			*bg_pixbuf;					// Points to the background layer
	GdkPixbuf			*backing_pixbuf;			// Pixbuf buffer things draw onto
	layer				*layer_data;				// Pointer to the layer data
	gint				layer_counter;				// Simple counter
	GList				*layer_pointer;				// Pointer to the layer GList
	gboolean			use_cached_pixbuf;			// Should we use the cached pixbuf?
	layer				*this_layer_data;			// Layer data
	slide				*this_slide_data;			// Pointer to the slide data


	// Simplify various pointers
	this_slide_data = (slide *) which_slide->data;
	layer_pointer = g_list_last(this_slide_data->layers);  // The background layer is always the last (bottom) layer
	layer_data = layer_pointer->data;

	// Determine if we have a cached background pixbuf we can reuse
	use_cached_pixbuf = FALSE;
	if (TRUE == this_slide_data->cached_pixbuf_valid)
	{
		// There's a pixbuf cached, but is it the required size?
		if ((height == gdk_pixbuf_get_height(this_slide_data->scaled_cached_pixbuf)) && (width == gdk_pixbuf_get_width(this_slide_data->scaled_cached_pixbuf)))
			use_cached_pixbuf = TRUE;
	}

	// Do we have a cached background pixbuf we can use?
	if (FALSE == use_cached_pixbuf)
	{
		// * No we don't, so we need to generate a new one *

		// Check if this is an empty slide
		if (TYPE_EMPTY == layer_data->object_type)
		{
			// This is an empty layer
			backing_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
			gdk_pixbuf_fill(backing_pixbuf, ((((layer_empty *) layer_data->object_data)->bg_color.red / 255) << 24)
				+ ((((layer_empty *) layer_data->object_data)->bg_color.green / 255) << 16)
				+ ((((layer_empty *) layer_data->object_data)->bg_color.blue / 255) << 8) + 0xff);
		} else
		{
			// Point to the background image pixbuf for the source
			bg_pixbuf = GDK_PIXBUF((GObject *) ((layer_image *) layer_data->object_data)->image_data);
	
			if ((width != gdk_pixbuf_get_width(bg_pixbuf)) || (height != gdk_pixbuf_get_height(bg_pixbuf)))
			{
				// Scale the background image pixbuf to the desired size
				backing_pixbuf = gdk_pixbuf_scale_simple(bg_pixbuf, width, height, GDK_INTERP_TILES);
			} else
			{
				// The existing background image pixbuf is already the correct size
				backing_pixbuf = gdk_pixbuf_copy(bg_pixbuf);
			}
		}

		// If there's an existing cache backing pixbuf, we free it
		if (NULL != this_slide_data->scaled_cached_pixbuf)
		{
			g_object_unref(GDK_PIXBUF(this_slide_data->scaled_cached_pixbuf));
		}

		// Cache the new backing pixbuf
		this_slide_data->scaled_cached_pixbuf = gdk_pixbuf_copy(backing_pixbuf);
		this_slide_data->cached_pixbuf_valid = TRUE;
	} else
	{
		// Yes we do, so reuse the existing pixbuf
		backing_pixbuf = gdk_pixbuf_copy(this_slide_data->scaled_cached_pixbuf);
	}

	// Process each layer's data in turn
	for (layer_counter = this_slide_data->num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, layer_counter);
		compress_layers_inner(this_layer_data, GDK_PIXBUF(backing_pixbuf), time_position);
	}

	// Return the newly scaled pixbuf
	return backing_pixbuf;
}

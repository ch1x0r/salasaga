/*
 * $Id$
 *
 * Salasaga: Function that gets all of the layers for a slide, then creates a pixel buffer from them
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
#include <assert.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "compress_layers_inner.h"


GdkPixmap *compress_layers(GList *which_slide, gfloat time_position, guint width, guint height)
{
	// Local variables
	GdkPixbuf			*bg_pixbuf;					// Points to the background layer
	GdkPixbuf			*backing_pixbuf;			// Pixbuf buffer things draw onto
	GdkPixmap			*backing_pixmap;			// Pixmap buffer things draw onto
	layer				*layer_data;				// Pointer to the layer data
	gint				layer_counter;				// Simple counter
	GList				*layer_pointer;				// Pointer to the layer GList
	GString				*message;					// Used to construct message strings
	GdkGC				*pixmap_gc;
	gint				pixmap_height;				// Receives the height of a given pixmap
	gint				pixmap_width;				// Receives the width of a given pixmap
	static GdkColormap	*system_colourmap = NULL;	// Colormap used for drawing
	gboolean			use_cached_pixmap;			// Should we use the cached pixmap?
	layer				*this_layer_data;			// Layer data
	slide				*this_slide_data;			// Pointer to the slide data


	// Initialisation
	if (NULL == system_colourmap)
	{
		system_colourmap = gdk_colormap_get_system();
	}

	// Simplify various pointers
	assert(which_slide != NULL);
	this_slide_data = (slide *) which_slide->data;
	assert(this_slide_data != NULL);
	layer_pointer = g_list_last(this_slide_data->layers);  // The background layer is always the last (bottom) layer
	assert(layer_pointer != NULL);
	layer_data = layer_pointer->data;
	assert(layer_data != NULL);

	// Determine if we have a cached background pixmap we can reuse
	use_cached_pixmap = FALSE;
	if (TRUE == this_slide_data->cached_pixmap_valid)
	{
		// There's a pixmap cached, but is it the required size?
		gdk_drawable_get_size(GDK_PIXMAP(this_slide_data->scaled_cached_pixmap), &pixmap_width, &pixmap_height);
		if ((height == pixmap_height) && (width == pixmap_width))
			use_cached_pixmap = TRUE;
	}

	// Do we have a cached background pixmap we can use?
	if (FALSE == use_cached_pixmap)
	{
		// * No we don't, so we need to generate a new one *

		// Check if this is an empty slide
		if (TYPE_EMPTY == layer_data->object_type)
		{
			// This is an empty layer
			backing_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
			if (NULL == backing_pixbuf)
			{
				// Couldn't create a GDK Pixbuf
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED463: %s", _("Error"), _("Couldn't create a GDK Pixbuf."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return NULL;
			}
			gdk_pixbuf_fill(GDK_PIXBUF(backing_pixbuf), ((((layer_empty *) layer_data->object_data)->bg_color.red / 256) << 24)
				+ ((((layer_empty *) layer_data->object_data)->bg_color.green / 256) << 16)
				+ ((((layer_empty *) layer_data->object_data)->bg_color.blue / 256) << 8) + 0xff);
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
				g_object_ref(bg_pixbuf);
				backing_pixbuf = bg_pixbuf;
			}
		}

		// If there's an existing cache backing pixmap, we free it
		if (NULL != this_slide_data->scaled_cached_pixmap)
		{
			g_object_unref(GDK_PIXMAP(this_slide_data->scaled_cached_pixmap));
			this_slide_data->scaled_cached_pixmap = NULL;
		}

		// Create a new backing pixmap
		backing_pixmap = gdk_pixmap_new(NULL, width, height, system_colourmap->visual->depth);
		gdk_drawable_set_colormap(GDK_DRAWABLE(backing_pixmap), GDK_COLORMAP(system_colourmap));
		gdk_draw_pixbuf(GDK_PIXMAP(backing_pixmap), NULL, GDK_PIXBUF(backing_pixbuf),
				0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
		g_object_unref(backing_pixbuf);

		// Cache the new backing pixmap
		this_slide_data->scaled_cached_pixmap = gdk_pixmap_new(NULL, width, height, system_colourmap->visual->depth);
		gdk_drawable_set_colormap(GDK_DRAWABLE(this_slide_data->scaled_cached_pixmap), GDK_COLORMAP(system_colourmap));
		pixmap_gc = gdk_gc_new(GDK_DRAWABLE(this_slide_data->scaled_cached_pixmap));
		gdk_draw_drawable(GDK_DRAWABLE(this_slide_data->scaled_cached_pixmap), GDK_GC(pixmap_gc), GDK_DRAWABLE(backing_pixmap),
				0, 0, 0, 0, -1, -1);
		this_slide_data->cached_pixmap_valid = TRUE;
	} else
	{
		// Yes we do, so reuse the existing pixmap
		backing_pixmap = gdk_pixmap_new(NULL, width, height, system_colourmap->visual->depth);
		gdk_drawable_set_colormap(GDK_DRAWABLE(backing_pixmap), GDK_COLORMAP(system_colourmap));
		pixmap_gc = gdk_gc_new(GDK_DRAWABLE(this_slide_data->scaled_cached_pixmap));
		gdk_draw_drawable(GDK_DRAWABLE(backing_pixmap), GDK_GC(pixmap_gc), GDK_DRAWABLE(this_slide_data->scaled_cached_pixmap),
				0, 0, 0, 0, -1, -1);
	}

	// Process each layer's data in turn
	for (layer_counter = this_slide_data->num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, layer_counter);
		compress_layers_inner(this_layer_data, backing_pixmap, time_position);
	}

	// Free the memory used in this function
	g_object_unref(pixmap_gc);

	// Return the updated pixmap
	return backing_pixmap;
}

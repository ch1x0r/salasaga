/*
 * $Id$
 *
 * Salasaga: Creates a cairo (surface) pattern from a given GDK pixbuf
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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
#include "../../externs.h"
#include "../display_warning.h"


cairo_pattern_t *create_cairo_pixbuf_pattern(GdkPixbuf *source_pixbuf)
{
	// Local variables
	guchar				*dest_buffer;
	guchar				*dest_start;
	guchar				*dest_ptr;
	cairo_pattern_t		*image_pattern;
	cairo_surface_t		*image_surface;
	guint				num_channels;
	guchar				*row_end;
	guint				row_stride;
	gint				source_height;
	guchar				*source_ptr;
	guchar				*source_start;
	gint				source_width;
	gint				y_counter;


	// Get info about the source pixbuf
	row_stride = gdk_pixbuf_get_rowstride(source_pixbuf);
	num_channels = gdk_pixbuf_get_n_channels(source_pixbuf);
	source_height = gdk_pixbuf_get_height(source_pixbuf);
	source_width = gdk_pixbuf_get_width(source_pixbuf);
	source_start = gdk_pixbuf_get_pixels(source_pixbuf);

	// Create a memory buffer for re-formatting our pixbuf data 
	dest_buffer = g_try_new0(guchar, source_width * source_height * 4);
	if (NULL == dest_buffer)
	{
		display_warning("Error ED371: Unable to allocate memory for pixel buffer");
		return NULL;
	}
	dest_start = dest_buffer;

	// Reformat the pixbuf data to something cairo can use
	for (y_counter = 0; y_counter < source_height; y_counter++)
	{
		source_ptr = source_start;
		row_end = source_ptr + (3 * source_width);
		dest_ptr = dest_start;

		// Reformat a row of data 
		while (source_ptr < row_end)
		{
			// * Native byte ordering is used, so we have to be careful *

#if G_LITTLE_ENDIAN == G_BYTE_ORDER 

			// Little endian (i.e. x86)
			dest_ptr[2] = source_ptr[0];
			dest_ptr[1] = source_ptr[1];
			dest_ptr[0] = source_ptr[2];

#else

			// Big endian (i.e. SPARC)
			dest_ptr[1] = source_ptr[0];
			dest_ptr[2] = source_ptr[1];
			dest_ptr[3] = source_ptr[2];

#endif

			// Point to the next source and destination pixel
			source_ptr += 3;
			dest_ptr += 4;
		}

		// Point to the start of the next source and destination rows
		source_start += row_stride;
		dest_start += 4 * source_width;
	}

	// Turn the reformatted buffer into a cairo surface
	image_surface = cairo_image_surface_create_for_data(dest_buffer, CAIRO_FORMAT_RGB24, source_width, source_height, source_width * 4);

	// Turn the surface into a cairo pattern
	image_pattern = cairo_pattern_create_for_surface(image_surface);
	cairo_surface_destroy(image_surface);

	return image_pattern;
}

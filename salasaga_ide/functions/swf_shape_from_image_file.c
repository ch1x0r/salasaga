/*
 * $Id$
 *
 * Flame Project: Loads a given image file and converts it to a SWF shape
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

// Ming include
#include <ming.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


SWFShape swf_shape_from_image_file(gchar *filename, gint width, gint height)
{
	// Create local variables
	GError				*error = NULL;				// Pointer to error return structure
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	SWFInput			image_input;				// Used to hold a swf input object
	GdkPixbuf			*image_pixbuf;				// Temporary GDK Pixbuf
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed jpeg image
	gsize				pixbuf_size;				// Is given the size of a compressed jpeg image
	gboolean			return_code_bool;			// Receives boolean return codes
	SWFShape			swf_shape;					// Holds the newly created swf shape


	image_pixbuf = gdk_pixbuf_new_from_file_at_size(filename, width, height, NULL);
	if (NULL == image_pixbuf)
	{
		// Something went wrong when loading the Play button's UP stat image
		printf("Error ED104: Couldn't load image file: %s\n", filename);
		display_warning("Error ED104: Something went wrong when loading the image for conversion to a swf shape");

		return NULL;
	}

	// Use gtk functions to convert scaled image to PNG. (using PNG to ensure no image quality loss)
	return_code_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(image_pixbuf),
					&pixbuf_buffer,  // Will come back filled out with location of image data
					&pixbuf_size,  // Will come back filled out with size of image data
					"png",
					&error,
					NULL);
	if (FALSE == return_code_bool)
	{
		// Something went wrong when encoding the image to required format
		display_warning("Error ED105: Something went wrong when encoding an image to png format");

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(image_pixbuf);

		return NULL;
	}

	// Use newSWFBitmap_fromInput() to convert to a SWF Bitmap
	image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
	if (NULL == image_input)
	{
		// Something went wrong when encoding the image to required format
		display_warning("Error ED106: Something went wrong converting an image to a swf input object");

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(image_pixbuf);
		// fixme4: Wonder how we free the pixbuf_buffer?

		return NULL;
	}

	image_bitmap = newSWFBitmap_fromInput(image_input);
	if (NULL == image_bitmap)
	{
		// Something went wrong when encoding the image to required format
		display_warning("Error ED107: Something went wrong converting an image to a swf bitmap object");

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(image_pixbuf);
		// fixme4: Wonder how we free the pixbuf_buffer?
		destroySWFInput(image_input);

		return NULL;
	}

	// Use newSWFShapeFromBitmap to create the shape
	swf_shape = newSWFShapeFromBitmap(image_bitmap, SWFFILL_CLIPPED_BITMAP);
	if (NULL == swf_shape)
	{
		// Something went wrong when encoding the image to required format
		display_warning("Error ED108: Something went wrong converting an image to a swf shape object");

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(image_pixbuf);
		// fixme4: Wonder how we free the pixbuf_buffer?
		destroySWFBitmap(image_bitmap);
		destroySWFInput(image_input);

		return NULL;
	}

	return swf_shape;
}

/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2008/01/23 17:20:34  vapour
 * Added a new function, for loading a SWF Shape bitmap directly from a given filename.
 *
 */

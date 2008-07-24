/*
 * $Id$
 *
 * Salasaga: Loads a given image file and converts it to a SWF shape
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

// Ming include
#include <ming.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "../../dialog/display_warning.h"


SWFShape swf_shape_from_image_file(gchar *filename, gint width, gint height)
{
	// Create local variables
	GError				*error = NULL;				// Pointer to error return structure
	SWFBitmap			image_bitmap;				// Used to hold a scaled bitmap object
	SWFInput			image_input;				// Used to hold a swf input object
	GdkPixbuf			*image_pixbuf;				// Temporary GDK Pixbuf
	GString				*message;					// Used to construct message strings
	gchar				*pixbuf_buffer;				// Is given a pointer to a compressed png image
	gsize				pixbuf_size;				// Is given the size of a compressed png image
	gboolean			return_code_bool;			// Receives boolean return codes
	SWFShape			swf_shape;					// Holds the newly created swf shape


	// Initialisation
	message = g_string_new(NULL);

	image_pixbuf = gdk_pixbuf_new_from_file_at_size(filename, width, height, NULL);
	if (NULL == image_pixbuf)
	{
		// Something went wrong when loading the Play button's UP stat image
		g_string_printf(message, "%s ED104: %s - '%'", _("Error"), _("Something went wrong when loading the image for conversion to a swf shape"), filename);
		display_warning(message->str);
		g_string_free(message, TRUE);
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
		g_string_printf(message, "%s ED105: %s", _("Error"), _("Something went wrong when encoding an image to png format."));
		display_warning(message->str);
		g_string_free(message, TRUE);

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(GDK_PIXBUF(image_pixbuf));

		return NULL;
	}

	// Use newSWFBitmap_fromInput() to convert to a SWF Bitmap
	image_input = newSWFInput_buffer((guchar *) pixbuf_buffer, pixbuf_size);
	if (NULL == image_input)
	{
		// Something went wrong when encoding the image to required format
		g_string_printf(message, "%s ED106: %s", _("Error"), _("Something went wrong converting an image to a swf input object."));
		display_warning(message->str);
		g_string_free(message, TRUE);

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(GDK_PIXBUF(image_pixbuf));
		if (NULL != pixbuf_buffer)
			g_object_unref(GDK_PIXBUF(pixbuf_buffer));

		return NULL;
	}

	image_bitmap = newSWFBitmap_fromInput(image_input);
	if (NULL == image_bitmap)
	{
		// Something went wrong when encoding the image to required format
		g_string_printf(message, "%s ED107: %s", _("Error"), _("Something went wrong converting an image to a swf bitmap object."));
		display_warning(message->str);
		g_string_free(message, TRUE);

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(GDK_PIXBUF(image_pixbuf));
		if (NULL != pixbuf_buffer)
			g_object_unref(GDK_PIXBUF(pixbuf_buffer));
		destroySWFInput(image_input);

		return NULL;
	}

	// Use newSWFShapeFromBitmap to create the shape
	swf_shape = newSWFShapeFromBitmap(image_bitmap, SWFFILL_CLIPPED_BITMAP);
	if (NULL == swf_shape)
	{
		// Something went wrong when encoding the image to required format
		g_string_printf(message, "%s ED108: %s", _("Error"), _("Something went wrong converting an image to a swf shape object."));
		display_warning(message->str);
		g_string_free(message, TRUE);

		// Free the memory allocated in this function
		g_error_free(error);
		if (NULL != image_pixbuf)
			g_object_unref(GDK_PIXBUF(image_pixbuf));
		if (NULL != pixbuf_buffer)
			g_object_unref(GDK_PIXBUF(pixbuf_buffer));
		destroySWFBitmap(image_bitmap);
		destroySWFInput(image_input);

		return NULL;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	return swf_shape;
}

/*
 * $Id$
 *
 * Flame Project: Function to create a Flash dictionary shape for an image layer
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
#include <gtk/gtk.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


GByteArray *flash_create_tag_bitmap(layer_image *layer_data)
{
	// Local variables
	gsize				buffer_size;
	GError				*error = NULL;
	gchar				*jpeg_buffer;
	GByteArray			*output_buffer;
	guint16				record_header;
	guint16				tag_code;
	guint16				tag_header;
	guint16				tag_length;
	gboolean			return_code_bool;


	// Initialise variables
	output_buffer = g_byte_array_new();
	record_header = 0;

	// Not sure if we should use SWF_TAG_DEFINE_BITS_JPEG2 or SWF_TAG_DEFINE_BITS_LOSSLESS
	// Might start with SWF_TAG_DEFINE_BITS_JPEG2, as I think we already have jpeg encoding functions
	tag_code = SWF_TAG_DEFINE_BITS_JPEG2 << 6;

	// Jpeg encode the image data
	return_code_bool = gdk_pixbuf_save_to_buffer(layer_data->image_data,	// Pointer to GDK-pixbuf data
												 &jpeg_buffer,				// Output buffer
												 &buffer_size,				// Size of output buffer
												 "jpeg",					// Format to save data in
												 &error,					// Error indicator
												 "quality",					// Key string
												 "100",						// Value string
												 NULL);						// NULL terminated list of options
	if (TRUE == return_code_bool)
	{
		// Something went wrong when creating the jpeg data, so display a warning
		display_warning(error->message);

		// Free resources allocated in this function
		g_error_free(error);

		// Return to the calling function
		return NULL;
	}

	// Add the image data to the output buffer
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) jpeg_buffer, buffer_size);

	// Embed the length of the output buffer in the tag header
	tag_length = output_buffer->len;
	tag_header = tag_code | tag_length;

	// Prepend the tag type and length
	g_byte_array_prepend(output_buffer, (guint8 *) &tag_header, 2);

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/10/03 13:41:23  vapour
 * Added initial untested code for converting slide layers to flash bitmap (jpeg2) data.
 *
 * Revision 1.2  2007/10/01 13:18:54  vapour
 * Started giving thought to the initial stub swf bitmap definition function.
 *
 * Revision 1.1  2007/10/01 12:14:17  vapour
 * Added initial stub function to create a bitmap layer in flash.
 *
 */

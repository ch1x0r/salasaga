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


// GTK includes
#include <gtk/gtk.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


GByteArray *flash_create_tag_bitmap(layer_image *layer_data, guint16 character_id)
{
	// Local variables
	gsize				buffer_size;
	guint8				*byte_pointer;			// Used to point at specific bytes we want to grab
	GError				*error = NULL;
	gchar				*jpeg_buffer;
	GByteArray			*output_buffer;
	guint16				record_header;
	guint16				tag_code;


	// Jpeg encode the image data
	gdk_pixbuf_save_to_buffer(layer_data->image_data,	// Pointer to GDK-pixbuf data
								&jpeg_buffer,			// Output buffer
								&buffer_size,			// Size of output buffer
								"jpeg",					// Format to save data in
								&error,					// Error indicator
								"quality",				// Key string
								"100",					// Value string
								NULL);					// NULL terminated list of options
	if (error)
	{
		// Something went wrong when creating the jpeg data, so display a warning
		display_warning(error->message);

		// Free resources allocated in this function
		g_error_free(error);

		// Return to the calling function
		return NULL;
	}

	// Initialise the output buffer
	output_buffer = g_byte_array_new();

	// Add the character id to the output buffer
	byte_pointer = (guint8 *) &character_id;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Add the image data to the output buffer
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) jpeg_buffer, buffer_size);

	// Embed the length of the output buffer plus character id in the record header
	tag_code = SWF_TAG_DEFINE_BITS_JPEG2 << 6;
	record_header = tag_code | (output_buffer->len + 2);

	// Prepend the tag type and length
	byte_pointer = (guint8 *) &record_header;
	output_buffer = g_byte_array_prepend(output_buffer, &byte_pointer[1], sizeof(guint8));
	output_buffer = g_byte_array_prepend(output_buffer, &byte_pointer[0], sizeof(guint8));

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.8  2007/10/11 13:47:40  vapour
 * Updated to pass character id to tag creation function.  Still needs work.
 *
 * Revision 1.7  2007/10/07 14:21:12  vapour
 * Fixed the checking of whether an error occured during the jpeg conversion.
 *
 * Revision 1.6  2007/10/07 06:26:23  vapour
 * Fixed a small bug.
 *
 * Revision 1.5  2007/10/06 14:52:37  vapour
 * Simplified the code a bit.  Code still untested.
 *
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

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


GByteArray *flash_create_tag_bitmap(layer *layer_data)
{
	// Local variables
	GByteArray			*output_array;
	guint16				record_header;
	guint16				tag_code;
	guint16				tag_header;
	guint16				tag_length;


	// Initialise variables
	output_array = g_byte_array_new();
	record_header = 0;

	// Not sure if we should use SWF_TAG_DEFINE_BITS_JPEG2 or SWF_TAG_DEFINE_BITS_LOSSLESS
	// Might start with SWF_TAG_DEFINE_BITS_JPEG2, as I think we already have jpeg encoding functions
	tag_code = SWF_TAG_DEFINE_BITS_JPEG2 << 6;

	// Fixme2: Needs to be written

	tag_length = 0;  // Fixme3: Needs to be figured out
	tag_header = tag_code | tag_length;

	// Prepend the tag type and length
	g_byte_array_prepend(output_array, (guint8 *) &tag_header, 2);

	// Return the fully formed dictionary shape
	return output_array;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/01 13:18:54  vapour
 * Started giving thought to the initial stub swf bitmap definition function.
 *
 * Revision 1.1  2007/10/01 12:14:17  vapour
 * Added initial stub function to create a bitmap layer in flash.
 *
 */

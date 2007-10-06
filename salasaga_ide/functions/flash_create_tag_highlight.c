/*
 * $Id$
 *
 * Flame Project: Function to create a Flash dictionary shape for a highlight layer
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


GByteArray *flash_create_tag_highlight(layer_highlight *layer_data)
{
	// Local variables
//	gsize				buffer_size;
//	GError				*error = NULL;
	GByteArray			*output_buffer;
	guint16				record_header;
	guint16				tag_code;
	guint16				tag_header;
	guint16				tag_length;
//	gboolean			return_code_bool;


	// Initialise variables
	output_buffer = g_byte_array_new();
	record_header = 0;


	// Fixme2: Needs to be written



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
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/10/04 11:50:28  vapour
 * Added initial (non working) stub functions for the creation of swf byte code for hightlight, mouse cursor, and text layers.
 *
 */

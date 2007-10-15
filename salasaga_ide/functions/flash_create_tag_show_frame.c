/*
 * $Id$
 *
 * Flame Project: Creates the swf tag to show the display list onscreen
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


GByteArray *flash_create_tag_show_frame()
{
	// Local variables
	guint8				*byte_pointer;			// Used to point at specific bytes we want to grab
	GByteArray			*output_buffer;
	guint16				record_header;


	// Initialise variables
	output_buffer = g_byte_array_new();

	// Create the tag (it has no data, so length automatically equals zero)
	record_header = SWF_TAG_SHOW_FRAME << 6;
	byte_pointer = (guint8 *) &record_header;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/15 06:29:35  vapour
 * Fixed a bug where it wasn't giving the correct tag type.
 * Updated to swap bytes properly.
 *
 * Revision 1.1  2007/10/07 06:37:06  vapour
 * Added further functions for swf code generation.
 *
 */

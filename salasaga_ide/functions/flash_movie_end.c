/*
 * $Id$
 *
 * Flame Project: 
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


GByteArray *flash_movie_end()
{
	// Local variables
	GByteArray			*output_buffer;
	guint16				record_header;


	// Initialise variables
	output_buffer = g_byte_array_new();

	// Create the tag (it has no data, so length automatically equals zero)
	record_header = SWF_TAG_MOVIE_END << 6;  // Doing the bitwise shift for consistency only
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &record_header, sizeof(guint16));  // Doesn't need to be byte swapped, as both bytes are 0 anyway

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/15 07:02:08  vapour
 * Added useful comments to explain some stuff.
 *
 * Revision 1.1  2007/10/07 06:37:06  vapour
 * Added further functions for swf code generation.
 *
 */

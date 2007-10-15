/*
 * $Id$
 *
 * Flame Project: Generates the swf tag for setting the background colour 
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


GByteArray *flash_create_tag_set_bg_colour(guint8 red_component, guint8 green_component, guint8 blue_component)
{
	// Local variables
	guint8				*byte_pointer;			// Used to point at specific bytes we want to grab
	GByteArray			*output_buffer;
	guint16				record_header;


	// Create the output stream
	output_buffer = g_byte_array_new();

	// Create the swf tag record header
	record_header = (SWF_TAG_SET_BACKGROUND_COLOUR << 6) | 3;  // Data length is fixed at 3 bytes
	byte_pointer = (guint8 *) &record_header;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));
	
	// Add the red, green, and blue components to the output stream
	output_buffer = g_byte_array_append(output_buffer, &red_component, sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &green_component, sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &blue_component, sizeof(guint8));

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/10/15 08:47:09  vapour
 * Added code to generate the swf tag for setting the background colour.
 *
 */

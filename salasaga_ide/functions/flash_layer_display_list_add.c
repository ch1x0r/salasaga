/*
 * $Id$
 *
 * Flame Project: Create the swf tag that adds a dictionary shape to the display list 
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


GByteArray *flash_layer_display_list_add(gint char_id, gint depth)
{
	// Local variables
	GByteArray			*output_buffer;
	guint16				record_header;
	guint16				tag_character_id;
	guint16				tag_code;
	guint16				tag_depth;
	guint8				tag_fields;
	guint8				tag_matrix;


	// Initialise variables
	output_buffer = g_byte_array_new();
	tag_character_id = (guint16) char_id;
	tag_depth = (guint16) depth;
	tag_matrix = 0;

	// Hard code the correct bit fields for this character (SWF_BIT_VALUE_PLACE_MOVE off, SWF_BIT_VALUE_PLACE_HAS_CHAR on, SWF_BIT_VALUE_PLACE_HAS_MATRIX on)
	tag_fields = SWF_BIT_VALUE_PLACE_HAS_CHAR | SWF_BIT_VALUE_PLACE_HAS_MATRIX;

	// Create the output buffer
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &tag_fields, sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &tag_depth, sizeof(guint16));  // Might need to byte swap this
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &tag_character_id, sizeof(guint16));  // Might need to byte swap this
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &tag_fields, sizeof(guint8));

	// Embed the length of the output buffer in the record header
	tag_code = SWF_TAG_PLACE_OBJECT_2 << 6;
	record_header = tag_code | output_buffer->len;

	// Prepend the tag type and length
	output_buffer = g_byte_array_prepend(output_buffer, (guint8 *) &record_header, 2);

	// Return the fully formed dictionary shape
	return output_buffer;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/10/07 06:37:06  vapour
 * Added further functions for swf code generation.
 *
 */

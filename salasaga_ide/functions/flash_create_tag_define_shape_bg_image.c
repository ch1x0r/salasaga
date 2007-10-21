/*
 * $Id$
 *
 * Flame Project: Creates the swf code to define a shape 
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


GByteArray *flash_create_tag_define_shape_bg_image(guint16 bitmap_id, guint16 character_id)
{
	// Local variables
	guint8				*byte_pointer;			// Used to point at specific bytes we want to grab
	GByteArray			*output_buffer;
	guint8				swf_rect_nbits = 0x80;  // Hard code number of bits to 16 (trying to be lazy)
	guint16				tag_code;
	guint8				working_byte;
	guint16				working_word;
	guint16				working_word2;
	guint16				x_max;
	guint16				y_max;


	// Initialise the output buffer
	output_buffer = g_byte_array_new();

	// Add the character id to the output buffer
	byte_pointer = (guint8 *) &character_id;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Define the bounds of the shape
	x_max = output_width * 20;
	y_max = output_height * 20;

	// Add the number of bits (16) for the rectangle structure values
	output_buffer = g_byte_array_append(output_buffer, (guint8 *) &swf_rect_nbits, sizeof(guint8));

	// Add the X minimum value
	working_byte = 0;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Add the X maximum value (first few bits - always 0 I think - are in the previous byte just added)
	working_byte = x_max >> 13;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = x_max << 3;
	working_byte = working_word >> 8;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = x_max << 11;
	working_byte = working_word >> 8;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Add the Y minimum value (first few bits - always 0 I think - are in the previous byte just added)
	working_byte = 0;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Add the Y maximum value
	working_byte = y_max >> 13;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = y_max << 3;
	working_byte = working_word >> 8;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = y_max << 11;
	working_byte = working_word >> 8;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// * shape with style info *

	// Fill in the number of fill styles (1)
	working_byte = 1;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Clipped fill style
	working_byte = 41;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Character ID of bitmap fill
	byte_pointer = (guint8 *) &bitmap_id;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Matrix for the bitmap fill
	working_byte = 0x20 | 0x05;  // Has scale = 0x20, number of bits = 5
	working_byte = working_byte << 2;  // Move them up two bits
	working_byte = working_byte | 2;  // Set 2nd lowest bit, the first part of the X scale amount
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_byte = 20;  // X Scale = 20
	working_byte = working_byte << 5;  // Move the X scale amount into position in the byte
	working_byte = working_byte + 20;  // Add the Y scale amount (20)
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_byte = 0;  //  No rotate nor translation bits
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	
	// Fill in the number of line styles (0)
	working_byte = 0;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Fill styles are 1 bit, no line styles
	working_byte = 0x10;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// * Create the straight edge shape components *

	// Start point is the centre, so move to the top left
	working_byte = 0x0C;  // Non-edge record, Move To, Fill Style 0
	working_byte = working_byte | 0x20;  // Number of move bits is 16;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = (x_max * -0.5);  // Half of maximum width, in negative
	working_word = working_word >> 3;
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));
	working_word = (x_max * -0.5);  // Isolate the bottom 3 bits, into the top of a word
	working_word = working_word << 13;
	working_word2 = (y_max * -0.5);  // Half of maximum height, in negative
	working_word2 = working_word2 >> 3;
	working_word = working_word | working_word2;  // Merge the two results then output
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));
	working_word = (y_max * -0.5);  // Isolate the bottom 3 bits
	working_word = working_word << 13;
	working_byte = working_word >> 8;
	working_byte = working_byte | 0x10;  // Fill style 0 is first fill style defined
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// Move horizontally to top right
	working_byte = 0x80;  // This is an edge record
	working_byte = working_byte | SWF_SHAPE_STRAIGHT_EDGE;  // This is a straight edge
	working_byte = working_byte | (14 << 2);  // Number of bits is 16
	working_byte = working_byte | SWF_SHAPE_HORIZONTAL_LINE;  // Horizontal line
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = x_max;  // Maximum width
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Move vertically down to bottom right
	working_byte = 0x80;  // This is an edge record
	working_byte = working_byte | SWF_SHAPE_STRAIGHT_EDGE;  // This is a straight edge
	working_byte = working_byte | (14 << 2);  // Number of bits is 16
	working_byte = working_byte | SWF_SHAPE_VERTICAL_LINE;  // Horizontal line
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = y_max;  // Maximum height
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Move horizontally to bottom left
	working_byte = 0x80;  // This is an edge record
	working_byte = working_byte | SWF_SHAPE_STRAIGHT_EDGE;  // This is a straight edge
	working_byte = working_byte | (14 << 2);  // Number of bits is 16
	working_byte = working_byte | SWF_SHAPE_HORIZONTAL_LINE;  // Horizontal line
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = -x_max;  // Maximum width, negative
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// Move vertically up to top left
	working_byte = 0x80;  // This is an edge record
	working_byte = working_byte | SWF_SHAPE_STRAIGHT_EDGE;  // This is a straight edge
	working_byte = working_byte | (14 << 2);  // Number of bits is 16
	working_byte = working_byte | SWF_SHAPE_VERTICAL_LINE;  // Horizontal line
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));
	working_word = -y_max;  // Maximum height, negative
	byte_pointer = (guint8 *) &working_word;
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[0], sizeof(guint8));
	output_buffer = g_byte_array_append(output_buffer, &byte_pointer[1], sizeof(guint8));

	// End of shape record
	working_byte = SWF_SHAPE_END_RECORD;
	output_buffer = g_byte_array_append(output_buffer, &working_byte, sizeof(guint8));

	// * Set the tag type and fill in the length *
	tag_code = (SWF_TAG_DEFINE_SHAPE << 6) | output_buffer->len;
	byte_pointer = (guint8 *) &tag_code;
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
 * Revision 1.3  2007/10/21 12:20:12  vapour
 * Added first cut of shape edge definition records.  Untested.
 *
 * Revision 1.2  2007/10/18 11:28:53  vapour
 * Updated variable definition and declaration for the flash_create_tag_define_shape_bg_image function.
 * Updated code calling it to also pass the bitmap id.
 * Added initial untested code to include fill style information.
 * Still needs work and testing.
 *
 * Revision 1.1  2007/10/17 11:04:08  vapour
 * Adding initial code to define shapes to place the background images on.  Still needs work.
 *
 */

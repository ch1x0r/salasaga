/*
 * $Id$
 *
 * Flame Project: Creates the swf header bytes 
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

GByteArray *flash_create_header(guint32 length_of_movie, guint16 number_of_frames, guint16 frame_rate)
{
	// Local variables
	guint8				*byte_pointer;			// Used to point at specific bytes we want to grab
	GByteArray			*swf_header;			// Pointer to the swf header construct
	guint8				swf_rect_nbits = 0x80;  // Hard code number of bits to 16 (trying to be lazy)
	guchar				swf_tag[] = "FWS";
	guint8				swf_version = 7;
	guint8				working_byte;
	guint16				working_word;
	guint16				x_max;
	guint16				y_max;


	// Initialise local variables
	swf_header = g_byte_array_new();

	// Output debugging info if requested
	if (debug_level)
	{
		printf("Length of movie is %#010x bytes\n", length_of_movie);
		printf("Total number of frames in movie is %u\n", number_of_frames);
	}

	// Add the FWS string to the swf header
	swf_header = g_byte_array_append(swf_header, swf_tag, sizeof(guchar) * 3);

	// Add the flash version number to the swf header
	swf_header = g_byte_array_append(swf_header, &swf_version, sizeof(guint8));

	// Add the file length to the swf header
	byte_pointer = (guint8 *) &length_of_movie;
	swf_header = g_byte_array_append(swf_header, &byte_pointer[0], sizeof(guint8));
	swf_header = g_byte_array_append(swf_header, &byte_pointer[1], sizeof(guint8));
	swf_header = g_byte_array_append(swf_header, &byte_pointer[2], sizeof(guint8));
	swf_header = g_byte_array_append(swf_header, &byte_pointer[3], sizeof(guint8));

	// Output debugging info if requested
	if (debug_level)
	{
		printf("Length of file, first byte: %#04x\n", byte_pointer[0]);
		printf("Length of file, second byte: %#04x\n", byte_pointer[1]);
		printf("Length of file, third byte: %#04x\n", byte_pointer[2]);
		printf("Length of file, fourth byte: %#04x\n", byte_pointer[3]);
	}

	// * Add the swf dimensions to the swf header *
	x_max = output_width * 20;
	y_max = output_height * 20;

	// Add the number of bits (16) for the rectangle structure values
	swf_header = g_byte_array_append(swf_header, (guint8 *) &swf_rect_nbits, sizeof(guint8));

	// Add the X minimum value
	working_byte = 0;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the X maximum value (first few bits - always 0 I think - are in the previous byte just added)
	working_byte = x_max >> 13;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = x_max << 3;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = x_max << 11;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the Y minimum value (first few bits - always 0 I think - are in the previous byte just added)
	working_byte = 0;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the Y maximum value
	working_byte = y_max >> 13;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = y_max << 3;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = y_max << 11;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the frame rate to the swf header
	byte_pointer = (guint8 *) &frame_rate;
	swf_header = g_byte_array_append(swf_header, &byte_pointer[1], sizeof(guint8));
	swf_header = g_byte_array_append(swf_header, &byte_pointer[0], sizeof(guint8));

	// Output debugging info if requested
	if (debug_level)
	{
		printf("Frame rate, decimal: %u\n", frame_rate);
		printf("Frame rate, hex: %#06x\n", frame_rate);
		printf("Frame rate, first byte: %#04x\n", byte_pointer[0]);
		printf("Frame rate, second byte: %#04x\n", byte_pointer[1]);
	}

	// Add the total frame count to the swf header
	byte_pointer = (guint8 *) &number_of_frames;
	swf_header = g_byte_array_append(swf_header, &byte_pointer[0], sizeof(guint8));
	swf_header = g_byte_array_append(swf_header, &byte_pointer[1], sizeof(guint8));

	// Output debugging info if requested
	if (debug_level)
	{
		printf("Number of frames, first byte: %#04x\n", byte_pointer[0]);
		printf("Number of frames, second byte: %#04x\n", byte_pointer[1]);
	}

	return swf_header;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.8  2007/10/15 08:14:46  vapour
 * Small tweak for comment and code clarity.
 *
 * Revision 1.7  2007/10/15 03:27:45  vapour
 * Fixed a bug stopping the dimensions of the movie being calculated correctly.
 *
 * Revision 1.6  2007/10/11 10:56:44  vapour
 * Frame rate is now written to the swf header correctly.
 *
 * Revision 1.5  2007/10/11 10:37:13  vapour
 * Number of frames is now calculated correctly.
 * Frame rate still needs work.
 *
 * Revision 1.4  2007/10/11 10:15:46  vapour
 * Correctly hard coded the number of dimension bits in the swf header to 16 bits.
 *
 * Revision 1.3  2007/10/11 10:06:14  vapour
 * Figured out how to point directly to the byte we want to write out, so the byte ordering of the length of file field in the swf header should be correct now.
 *
 * Revision 1.2  2007/10/10 14:58:41  vapour
 * Adjusted swf version in header tag to be version 7.
 * Started trying to get the byte ordering in the header correct.  Needs lots more work.
 *
 * Revision 1.1  2007/10/07 09:00:46  vapour
 * Moved the code for creating the swf header bytes into its own function.
 *
 */

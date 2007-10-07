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
	guint16				swf_frame_rate;
	GByteArray			*swf_header;			// Pointer to the swf header construct
	guint32				swf_length;
	guint8				swf_rect_nbits = 0xF0;  // Hard code number of bits to 16 (trying to be lazy)
	guchar				swf_tag[] = "FWS";
	guint16				swf_total_frames;
	guint8				swf_version = 1;
	guint8				working_byte;
	guint16				working_word;
	guint16				x_max;
	guint16				y_max;


	// Initialise local variables
	swf_frame_rate = frame_rate;
	swf_header = g_byte_array_new();
	swf_length = length_of_movie;
	swf_total_frames = number_of_frames;

	// Add the FWS string to the swf headertag, flash version number, and length of the file
	swf_header = g_byte_array_append(swf_header, swf_tag, sizeof(guchar) * 3);

	// Add the flash version number to the swf header
	swf_header = g_byte_array_append(swf_header, &swf_version, sizeof(guint8));

	// Add the file length to the swf header
	swf_header = g_byte_array_append(swf_header, (guint8 *) &swf_length, sizeof(guint32));

	// * Add the swf dimensions to the swf header *
	x_max = output_width * 20;
	y_max = output_height * 20;

	// Add the number of bits (16) for the rectangle structure values
	swf_header = g_byte_array_append(swf_header, (guint8 *) &swf_rect_nbits, sizeof(guint8));

	// Add the X minimum value
	working_byte = 0;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the X maximum value (first few bits - always 0 I think - are in the previous byte just added)
	working_byte = (guint8) x_max >> 13;
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
	working_byte = (guint8) y_max >> 13;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = y_max << 3;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));
	working_word = y_max << 11;
	working_byte = working_word >> 8;
	swf_header = g_byte_array_append(swf_header, &working_byte, sizeof(guint8));

	// Add the frame rate to the swf header
	swf_header = g_byte_array_append(swf_header, (guint8 *) &swf_frame_rate, sizeof(guint8));

	// Add the total frame count to the swf header
	swf_header = g_byte_array_append(swf_header, (guint8 *) &swf_total_frames, sizeof(guint16));

	return swf_header;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/10/07 09:00:46  vapour
 * Moved the code for creating the swf header bytes into its own function.
 *
 */

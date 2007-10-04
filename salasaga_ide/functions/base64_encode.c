/*
 * $Id$
 *
 * Flame Project: Function to create a Base64 encoded string from a block of data in memory
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

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


gboolean base64_encode(gpointer data, guint length, gchar **output_string)
{
	// Local variables
	gchar				*base64_dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	guint				buffer_length;
	guint				characters_out;			// Counter of how many characters have been output
	guint				counter;				// Counter of how many characters have been converted
	gchar				*input_buffer;
	gdouble				max_loop;				// The maximum number of interations to run
	guint				offset;					// Either 0, 1, or 2.  Used to calculate the end few output bytes
	gchar				*output_buffer;
	gint				output_counter;			// Counter used for positioning inside the output buffer
	gint				string_offset;			// Used as an offset pointer while parsing

	guchar				first_byte;
	guchar				fourth_byte;
	guchar				second_byte;
	guchar				third_byte;

	guchar				tmp_byte;


	// Initialise some things
	input_buffer = data;
	output_counter = 0;
	characters_out = 0;

	// Calculate the length of the Base64 buffer
	buffer_length = (guint) ((float) length * 1.5);  // Overestimate, to be on the safe side

	// Create the Base64 buffer
	output_buffer = g_new(gchar, buffer_length);

	// Calculate how many bytes are left hanging on the end of the input
	offset = length % 3;

	// Convert the data into Base64
	counter = 0;
	max_loop = length / 3;
	while (counter != max_loop)
	{
		string_offset = counter * 3;

		first_byte = input_buffer[string_offset];
		first_byte = first_byte >> 1;
		first_byte = first_byte >> 1;
		output_buffer[output_counter] = base64_dictionary[first_byte];
		output_counter++;

		second_byte = input_buffer[string_offset];
		second_byte = second_byte << 6;
		second_byte = second_byte >> 2;
		tmp_byte = input_buffer[string_offset + 1];
		tmp_byte = tmp_byte >> 4;
		second_byte = second_byte | tmp_byte;
		output_buffer[output_counter] = base64_dictionary[second_byte];
		output_counter++;

		third_byte = input_buffer[string_offset + 1];
		third_byte = third_byte << 4;
		third_byte = third_byte >> 2;
		tmp_byte = input_buffer[string_offset + 2];
		tmp_byte = tmp_byte >> 6;
		third_byte = third_byte | tmp_byte;
		output_buffer[output_counter] = base64_dictionary[third_byte];
		output_counter++;

		fourth_byte = input_buffer[string_offset + 2];
		fourth_byte = fourth_byte << 2;
		fourth_byte = fourth_byte >> 2;
		output_buffer[output_counter] = base64_dictionary[fourth_byte];
		output_counter++;

		characters_out = characters_out + 4;

		// Insert a newline into the output stream every 76 characters (as per spec)
		if (0 == (characters_out % 76))
		{
			output_buffer[output_counter] = '\n';
			output_counter++;
		}

		counter++;
	}

	// Process the last one or two bytes on the end of the input data (as per Base64 requirement)
	switch ((gint) length % 3)
	{
		case 0:
			// No remainder, nothing to do
			break;

		case 1:
			// We only need to process one byte, to make two characters
			first_byte = input_buffer[length - 1];
			first_byte = first_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[first_byte];
			output_counter++;

			second_byte = input_buffer[length - 1];
			second_byte = second_byte << 6;
			second_byte = second_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[second_byte];
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			break;

		case 2:
			// We only need to process two bytes, to make three characters
			first_byte = input_buffer[length - 2];
			first_byte = first_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[first_byte];
			output_counter++;

			second_byte = input_buffer[length - 2];
			second_byte = second_byte << 6;
			second_byte = second_byte >> 2;
			tmp_byte = input_buffer[length - 1];
			tmp_byte = tmp_byte >> 4;
			second_byte = second_byte | tmp_byte;
			output_buffer[output_counter] = base64_dictionary[second_byte];
			output_counter++;

			third_byte = input_buffer[length - 1];
			third_byte = third_byte << 4;
			third_byte = third_byte >> 2;
			output_buffer[output_counter] = base64_dictionary[third_byte];
			output_counter++;
			output_buffer[output_counter] = '=';
			output_counter++;
			break;

		default:
			display_warning("ED79: Unknown remainder amount when Base64 encoding (shouldn't happen)\n");
	}

	// Put a NULL at the end of the Base64 encoded string
	output_buffer[output_counter] = '\0';

	output_string[0] = output_buffer;
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2007/10/04 19:51:11  vapour
 * Began adjusting the includes in each function to only call the ones it needs, in order to dramatically speed up recompilation time for developers.
 *
 * Revision 1.3  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:29  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Flame Project: Function to decode a Base64 encoded string
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
#include "../gui-functions.h"


GString *base64_decode(GString *input_string, GString *output_string)
{
	// Local variables
	GString				*copied_string;			// A copy of the input string
	guint				counter;				// Counter of how many characters have been converted
	guchar				holding_byte;			// Holds the byte being worked on
	gdouble				max_loop;				// The maximum number of interations to run

	guchar				out_byte0;				// Used to hold the bytes being translated
	guchar				out_byte1;				// Used to hold the bytes being translated
	guchar				out_byte2;				// Used to hold the bytes being translated
	gint				string_offset;			// Used as an offset pointer while parsing

	GString				*tmp_gstring;			// Temporary GString


	// Initialise various things
	g_string_assign(output_string, "");
	tmp_gstring = g_string_new(NULL);

	// Make a copy of the input string so we can work on it in place
	copied_string = g_string_new(NULL);

	// Lookup each byte of the input string for it's numeric value
	for (counter = 0; counter < input_string->len - 1; counter++)
	{

// Notes:
//
//	base64_dictionary_offsets = "0 ABCDEFGHIJKLMNOPQRSTUVWXYZ"
// 								"26 abcdefghijklmnopqrstuvwxyz"
//								"52 0123456789"
//								"62 +"
//								"63 /"
//
// ASCII values
// + = 43
// / = 47
// 0 = 48
// 9 = 57
// = = 61
// A = 65
// Z = 90
// a = 97
// z = 122

		holding_byte = input_string->str[counter];

		if (48 <= holding_byte)
		{
			// * To get here, it must be in the ranges of 0-9, A-Z, or a-z *

			if (65 <= holding_byte)
			{
				// * To get here, it must be in the ranges of A-Z or a-z *

				if (97 <= holding_byte)
				{
					// * To get here, it must be in the range of a-z *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte - 71);  // a = ASCII 97.  97 - 71 = 26.  So a = 26, b = 27, etc
				} else
				{
					// * To get here, it must be in the range of A-Z *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte - 65);  // 65 Makes A = 0, B = 1, etc
				}
			} else
			{
				if (61 == holding_byte)
				{
					// * To get here it must be the character '=', which means "no value", so don't add to the output string *
				} else
				{
					// * To get here it must be in the range 0-9 *
					copied_string = g_string_append_c(copied_string, (guchar) holding_byte + 4);  // 0 = ASCII 48.  48 + 4 = 52.  So 0 = 52, 1 = 53, etc
				}
			}
		} else
		{
			// Must be either +, /, or \n
			switch (input_string->str[counter])
			{
				case '+':
					copied_string = g_string_append_c(copied_string, (guchar) 62);
					break;

				case '/':
					copied_string = g_string_append_c(copied_string, (guchar) 63);
					break;

				case '\n':
					break;

				default:
					// Should never get here
					g_string_printf(tmp_gstring, "ED64: Error in the Base64 decoding function, unrecognised input '%c'.", holding_byte);
					display_warning(tmp_gstring->str);
			}
		}
	}

	// * To get here, the copied string should have been converted to it's offset values
	counter = 0;
	max_loop = (copied_string->len / 4);
	while (counter != max_loop)
	{
		string_offset = counter * 4;

		out_byte0 = (guchar) (copied_string->str[string_offset] << 2) | (copied_string->str[string_offset + 1] >> 4);
		out_byte1 = (guchar) (copied_string->str[string_offset + 1] << 4) | (copied_string->str[string_offset + 2] >> 2);
		out_byte2 = (guchar) (copied_string->str[string_offset + 2] << 6) | (copied_string->str[string_offset + 3]);

		output_string = g_string_append_c(output_string, out_byte0);
		output_string = g_string_append_c(output_string, out_byte1);
		output_string = g_string_append_c(output_string, out_byte2);

		counter++;
	}

	switch ((gint) copied_string->len % 4)
	{
		case 0:
			// No remainder, nothing to do
			break;

		case 2:
			// We only need to process two bytes, producing one char
			out_byte0 = copied_string->str[copied_string->len - 3];
			out_byte0 = out_byte0 << 2;

			out_byte1 = copied_string->str[copied_string->len - 2];
			out_byte1 = out_byte1 >> 4;

			out_byte2 = out_byte0 | out_byte1;
			output_string = g_string_append_c(output_string, out_byte2);
			break;

		case 3:
			// We only need to process three bytes, producing two chars
			out_byte0 = (guchar) (copied_string->str[copied_string->len - 3] << 2) | (copied_string->str[copied_string->len - 2] >> 4);
			out_byte1 = (guchar) (copied_string->str[copied_string->len - 2] << 4) | (copied_string->str[copied_string->len - 1] >> 2);
			output_string = g_string_append_c(output_string, out_byte0);
			output_string = g_string_append_c(output_string, out_byte1);
			break;

		default:
			display_warning("ED73: Unknown Base64 decoding remainder (shouldn't happen)\n");
	}

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
	g_string_free(copied_string, TRUE);

	return output_string;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/09/28 12:05:06  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:36  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Salasaga: Function to decode a Base64 encoded string
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 * 
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"


GString *base64_decode(GString *input_string, GString *output_string)
{
	// Local variables
	GString				*copied_string;			// A copy of the input string
	guint				counter;				// Counter of how many characters have been converted
	guchar				holding_byte;			// Holds the byte being worked on
	gdouble				max_loop;				// The maximum number of interations to run
	GString				*message;					// Used to construct message strings

	guchar				out_byte0;				// Used to hold the bytes being translated
	guchar				out_byte1;				// Used to hold the bytes being translated
	guchar				out_byte2;				// Used to hold the bytes being translated
	gint				string_offset;			// Used as an offset pointer while parsing


	// Initialise various things
	g_string_assign(output_string, "");
	message = g_string_new(NULL);

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
					g_string_printf(message, "%s ED64: %s '%c'.", _("Error"), _("Error in the Base64 decoding function, unrecognized input"), holding_byte);
					display_warning(message->str);
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
			g_string_printf(message, "%s ED73: %s", _("Error"), _("Unknown Base64 decoding remainder.  This should not happen."));
			display_warning(message->str);
	}

	// Free the memory allocated in this function
	g_string_free(copied_string, TRUE);
	g_string_free(message, TRUE);

	return output_string;
}

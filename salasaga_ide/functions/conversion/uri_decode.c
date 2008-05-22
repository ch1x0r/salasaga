/*
 * $Id$
 *
 * Salasaga: Function to decode a URI encoded string
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
#include "../salasaga_types.h"
#include "../externs.h"


GString *uri_decode(GString *input_string, GString *output_string)
{
	// Local variables
	guint				input_counter;			// Counter for how many bytes have been decoded


	// Walk the input string looking for specific character strings, replacing them as needed
	for (input_counter = 0; input_counter <= input_string->len; input_counter++)
	{
		switch (input_string->str[input_counter])
		{
			case '%':
				// The input character is potentially the first of the "%2B" or "%2F" series
				// that we're watching for
				if ('2' == input_string->str[input_counter + 1])
				{
					switch (input_string->str[input_counter + 2])
					{
						case 'B':
							output_string = g_string_append_c(output_string, '+');
							input_counter = input_counter + 2;
							break;

						case 'F':
							output_string = g_string_append_c(output_string, '/');
							input_counter = input_counter + 2;
							break;

						default:
							// Turns out it's not one of the one's we're watching for,
							// so just copy it to the output string
							output_string = g_string_append_c(output_string, input_string->str[input_counter]);
							break;
					}					
				} else
				{
					// Turns out it's not one of the one's we're watching for,
					// so just copy it to the output string
					output_string = g_string_append_c(output_string, input_string->str[input_counter]);
				}
				break;
				
			default:
				// The input character isn't one of the one's we're watching for,
				// so just copy it to the output string
				output_string = g_string_append_c(output_string, input_string->str[input_counter]);
		}
	}

	return output_string;
}
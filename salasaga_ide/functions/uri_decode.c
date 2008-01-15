/*
 * $Id$
 *
 * Flame Project: Function to decode a URI encoded string
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:27  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

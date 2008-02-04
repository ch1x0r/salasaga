/*
 * $Id$
 *
 * Flame Project: Function to create a URI encoded string from a Base64 encoded string
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


gboolean uri_encode_base64(gpointer data, guint length, gchar **output_string)
{
	// Local variables
	guint				buffer_length;
	gchar				*input_buffer;
	gint				input_counter;			// Counter used for positioning inside the input buffer
	gchar				*output_buffer;
	gint				output_counter;			// Counter used for positioning inside the output buffer


	// Initialise some things
	input_buffer = data;
	output_counter = 0;

	// Calculate the length of the URI encoded buffer
	buffer_length = (guint) ((float) length * 3);  // Overestimate, to be on the safe side

	// Create the URI encoded buffer
	output_buffer = g_new(gchar, buffer_length);

	// Do the URI encoding into the new buffer
	for (input_counter = 0; input_counter < length; input_counter++)
	{
		switch (input_buffer[input_counter])
		{
			case '+':
				output_buffer[output_counter] = '%';
				output_buffer[output_counter + 1] = '2';
				output_buffer[output_counter + 2] = 'B';
				output_counter = output_counter + 3;
				break;

			case '/':
				output_buffer[output_counter] = '%';
				output_buffer[output_counter + 1] = '2';
				output_buffer[output_counter + 2] = 'F';
				output_counter = output_counter + 3;
				break;

			case '\n':
				break;

			default:
				output_buffer[output_counter] = input_buffer[input_counter];
				output_counter++;
				break;
		}
	}

	// Put a NULL at the end of the URI encoded string
	output_buffer[output_counter] = '\0';
	output_string[0] = output_buffer;
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.5  2008/02/04 17:20:59  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.4  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:57  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

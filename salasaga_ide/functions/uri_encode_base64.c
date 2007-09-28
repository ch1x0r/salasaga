/*
 * $Id$
 *
 * Flame Project: Function to create a URI encoded string from a Base64 encoded string
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
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:57  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

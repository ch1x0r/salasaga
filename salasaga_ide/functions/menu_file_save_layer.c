/*
 * $Id$
 *
 * Flame Project: Function that takes a layer structure pointer, and adds it's data to an in-memory XML document
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
#include "base64_encode.h"
#include "display_warning.h"


void menu_file_save_layer(gpointer element, gpointer user_data)
{
	// Local variables
	gchar				*base64_string;			// Pointer to an Base64 string
	GError				*error = NULL;			// Pointer to error return structure
	guint				finish_frame;			// The finish frame in which the object appears
	GString				*layer_name;			// Name of the layer
	xmlNodePtr			layer_node;				// Pointer to the new layer node
	layer				*layer_pointer;			// Points to the presently processing layer
	guint				layer_type;				// The type of layer
	gchar				*pixbuf_buffer;			// Gets given a pointer to a compressed jpeg image
	gsize				pixbuf_size;			// Gets given the size of a compressed jpeg image
	xmlNodePtr			slide_node;				// Pointer to the slide node
	guint				start_frame;			// The first frame in which the object appears
	GtkTextIter			text_end;				// The end position of the text buffer
	GtkTextIter			text_start;				// The start position of the text buffer

	gboolean			tmp_bool;				// Temporary boolean value
	GString				*tmp_gstring;			// Temporary GString
	GString				*tmp_gstring2;			// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);
	layer_pointer = element;
	slide_node = user_data;

	// Create some useful pointers
	start_frame = layer_pointer->start_frame;
	finish_frame = layer_pointer->finish_frame;
	layer_name	= layer_pointer->name;
	layer_type = layer_pointer->object_type;

	// Create the layer container
	layer_node = xmlNewChild(slide_node, NULL, (const xmlChar *) "layer", NULL);
	if (NULL == layer_node)
	{
		display_warning("ED24: Error creating the layer elements\n");
		return;
	}

	// Add the layer data to the layer container
	xmlNewChild(layer_node, NULL, (const xmlChar *) "name", (const xmlChar *) layer_name->str);
	g_string_printf(tmp_gstring, "%u", start_frame);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "start_frame", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", finish_frame);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "finish_frame", (const xmlChar *) tmp_gstring->str);
	if (0 != layer_pointer->external_link->len)
	{
		xmlNewChild(layer_node, NULL, (const xmlChar *) "external_link", (const xmlChar *) layer_pointer->external_link->str);
	}
	switch (layer_type)
	{
		case TYPE_GDK_PIXBUF:

			// * We save the image data in the project file, in the same way we save image     *
			// * data in exported svg.  This way a project file is self-contained and portable *

			// Convert the compressed image into png data
			tmp_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(((layer_image *) layer_pointer->object_data)->image_data),
						&pixbuf_buffer,  // Will come back filled out with location of png data
						&pixbuf_size,  // Will come back filled out with size of png data
						"png",
						&error,
						NULL);
			if (FALSE == tmp_bool)
			{
				// Something went wrong when encoding the image to jpeg format
				display_warning("ED62: Something went wrong when encoding a slide to png format");

				// Free the memory allocated in this function
				g_string_free(tmp_gstring, TRUE);
				g_error_free(error);
				return;
			}

			// Store the count of image data bytes in the file for read back verification
			g_string_printf(tmp_gstring2, "%d", pixbuf_size);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data_length", (const xmlChar *) tmp_gstring2->str);

			// Base64 encode the image data
			base64_encode(pixbuf_buffer, pixbuf_size, &base64_string);

			// Create a string to write to the output file
			g_string_printf(tmp_gstring, "%s", base64_string);

			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "image");
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_EMPTY:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "empty");
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "blue", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_MOUSE_CURSOR:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "mouse");
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			if (MOUSE_NONE == ((layer_mouse *) layer_pointer->object_data)->click)
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "none");
			} else
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "left_one");
			}
			break;

		case TYPE_TEXT:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "text");
			gtk_text_buffer_get_bounds(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "text_value", (const xmlChar *) gtk_text_buffer_get_text(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end, FALSE));
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->text_color.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%f", ((layer_text *) layer_pointer->object_data)->font_size);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "font_size", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_HIGHLIGHT:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "highlight");
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			break;
	}

	// Free the memory used in this function
	g_string_free(tmp_gstring, TRUE);
	g_string_free(tmp_gstring2, TRUE);

	return;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:52  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

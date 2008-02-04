/*
 * $Id$
 *
 * Flame Project: Function called when the y offset start field in the timeline widget is edited 
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

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"


void timeline_edited_y_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:
			((layer_image *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_HIGHLIGHT:
			((layer_highlight *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_MOUSE_CURSOR:
			((layer_mouse *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_TEXT:
			((layer_text *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		default:
			display_warning("Error ED60: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_Y_OFF_START, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/04 17:19:40  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/01/19 07:09:20  vapour
 * Tweaked an error message for clarity.
 *
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/10/06 11:37:24  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.2  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:06  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Flame Project: Detect when the user presses the mouse button on the drawing area 
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
#include "timeline_edited_x_offset_start.h"
#include "timeline_edited_y_offset_start.h"


gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint					mouse_x;
	gint					mouse_y;
	gint					present_x;
	gint					present_y;
	gint					width;
	gint					height;
	layer				*layer_data;

	slide				*current_slide_data;			// Alias to make things easier
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	gboolean				selection_hit;				// Status toggle
	gchar				*selected_row;				// Holds the number of the row that is selected

	gdouble				tmp_double;					// Temporary double
	GtkTreeViewColumn	*tmp_column;					// Temporary column
	GString				*tmp_gstring;				// Temporary GString
	GtkTreePath			*tmp_path;					// Temporary path


	// Only do this function if we have a backing store available
	if (NULL == backing_store)
	{
		return TRUE;
	}

	// If this release isn't for the end of a mouse drag operation, we ignore it
	if (FALSE == mouse_dragging)
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;
	tmp_gstring = g_string_new(NULL);
	selection_hit = FALSE;

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Check for primary mouse button release
	if (1 == event->button)
	{
		// Determine which layer is selected in the timeline
		tmp_path = gtk_tree_path_new();
		tmp_column = gtk_tree_view_column_new();
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
		selected_row = gtk_tree_path_to_string(tmp_path);

		// Get its present X and Y offsets
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		layer_data = g_list_nth_data(current_slide_data->layers, atoi(selected_row));
		switch (layer_data->object_type)
		{
			case TYPE_EMPTY:
				// We can't drag an empty layer, so reset things and return
				mouse_dragging = FALSE;
				stored_x = -1;
				stored_y = -1;
				return TRUE;

			case TYPE_HIGHLIGHT:
				present_x = ((layer_highlight *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_highlight *) layer_data->object_data)->y_offset_start;
				width = ((layer_highlight *) layer_data->object_data)->width;
				height = ((layer_highlight *) layer_data->object_data)->height;
				break;

			case TYPE_GDK_PIXBUF:
				// If this is the background layer, then we ignore it
				if (0 == g_ascii_strncasecmp(layer_data->name->str, "Background", 10))
				{
					mouse_dragging = FALSE;
					stored_x = -1;
					stored_y = -1;
					return TRUE;
				}

				// No it's not, so process it
				present_x = ((layer_image *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_image *) layer_data->object_data)->y_offset_start;
				width = ((layer_image *) layer_data->object_data)->width;
				height = ((layer_image *) layer_data->object_data)->height;
				break;

			case TYPE_MOUSE_CURSOR:

				// No it's not, so process it
				present_x = ((layer_mouse *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_mouse *) layer_data->object_data)->y_offset_start;
				width = ((layer_mouse *) layer_data->object_data)->width;
				height = ((layer_mouse *) layer_data->object_data)->height;
				break;

			case TYPE_TEXT:
				present_x = ((layer_text *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_text *) layer_data->object_data)->y_offset_start;
				width = ((layer_text *) layer_data->object_data)->rendered_width;
				height = ((layer_text *) layer_data->object_data)->rendered_height;
				break;

			default:
				display_warning("Error ED32: Unknown layer type\n");
				return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
		}

		// Work out and set the new X offset for the layer object
		// fixme3: This needs to be a lot more accurate
		tmp_double = mouse_x - stored_x;
		tmp_double = (tmp_double * zoom) / 100;
		tmp_double = present_x + tmp_double;
		if (0 >= tmp_double)
		{
			tmp_double = 0;
		}
		if (tmp_double + width >= project_width)
		{
			tmp_double = project_width - width;
		}
		g_string_printf(tmp_gstring, "%.0f", tmp_double);
		timeline_edited_x_offset_start(NULL, selected_row, tmp_gstring->str, NULL);

		// Work out and set the new Y offset for the layer object
		// fixme3: This needs to be a lot more accurate
		tmp_double = mouse_y - stored_y;
		tmp_double = (tmp_double * zoom) / 100;
		tmp_double = present_y + tmp_double;
		if (0 >= tmp_double)
		{
			tmp_double = 0;
		}
		if (tmp_double + height >= project_height)
		{
			tmp_double = project_height - height;
		}
		g_string_printf(tmp_gstring, "%.0f", tmp_double);
		timeline_edited_y_offset_start(NULL, selected_row, tmp_gstring->str, NULL);

		// Reset the mouse drag switch and related info
		mouse_dragging = FALSE;
		stored_x = -1;
		stored_y = -1;

		// Free the allocated memory
		g_free(selected_row);
	}

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/04 17:22:04  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/01/19 07:10:09  vapour
 * Tweaked an error message for clarity.
 *
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/10/06 11:36:39  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.2  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

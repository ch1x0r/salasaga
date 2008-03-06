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
#include "detect_collisions.h"
#include "layer_edit.h"


gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	guint				num_layers;
	guint				count_int;

	GList				*collision_list = NULL;
	slide				*current_slide_data;	// Alias to make things easier
	GtkWidget			*list_widget;			// Alias to the timeline widget to make things easier
	guint				num_collisions;
	gboolean			selection_hit;			// Status toggle
	guint				selected_row;			// Holds the number of the row that is selected

	guint				tmp_int;				// Temporary integer
	GtkTreeViewColumn	*tmp_column;			// Temporary column
	GString				*tmp_gstring;			// Temporary GString
	GtkTreePath			*tmp_path;				// Temporary path


	// Only do this function if we have a front store available
	if (NULL == front_store)
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;
	selection_hit = FALSE;

	// Check for primary mouse button click
	if (1 != event->button)
	{
		// Not a primary mouse click, so return
		return TRUE;
	}

	// Reset the mouse drag toggle
	mouse_dragging = FALSE;

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();

		return TRUE;
	}

	// Check if this was a triple mouse click.  If it was, ignore it
	if (GDK_3BUTTON_PRESS == event->type)
	{
		return TRUE;
	}

	// If we're presently creating a new highlight layer, store the mouse coordinates
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Save the mouse coordinates
		stored_x = event->x;
		stored_y = event->y;

		// Reset the invalidation area
		invalidation_end_x = event->x;
		invalidation_end_y = event->y;
		invalidation_start_x = event->x - 1;
		invalidation_start_y = event->y - 1;

		return TRUE;
	}

	// * Do collision detection here to determine if the user has clicked on a layer's object *
	tmp_gstring = g_string_new(NULL);
	collision_list = detect_collisions(collision_list, event->x, event->y);
	if (NULL == collision_list)
	{
		// If there was no collision, then select the background layer
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		num_layers = g_list_length(current_slide_data->layers);
		g_string_printf(tmp_gstring, "%d", num_layers - 1);
		tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

		// Reset the stored mouse coordinates
		stored_x = -1;
		stored_y = -1;

		// Free the memory allocated during the collision detection
		g_string_free(tmp_gstring, TRUE);
		g_list_free(collision_list);
		collision_list = NULL;

		return TRUE;
	}

	// * To get here there must have been at least one collision *

	// Save the mouse coordinates
	stored_x = event->x;
	stored_y = event->y;

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));

	// Is the presently selected layer in the collision list?
	collision_list = g_list_first(collision_list);
	num_collisions = g_list_length(collision_list);
	for (count_int = 0; count_int < num_collisions; count_int++)
	{
		collision_list = g_list_first(collision_list);
		collision_list = g_list_nth(collision_list, count_int);

		current_slide_data->layers = g_list_first(current_slide_data->layers);

		tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
		if (tmp_int == selected_row)
		{
			// * Yes, the presently selected row is in the collision list *
			selection_hit = TRUE;

			// If there are further collisions in the collision list, we just move the timeline row selection
			// to the next collision row, otherwise we wrap to the collision row
			if (count_int == num_collisions - 1)
			{
				// We're at the bottom of the collision hit list, so we wrap back to the start
				collision_list = g_list_first(collision_list);
				current_slide_data->layers = g_list_first(current_slide_data->layers);
				tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
				g_string_printf(tmp_gstring, "%d", tmp_int);
				tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
				gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

				// Free the memory allocated during the collision detection
				g_string_free(tmp_gstring, TRUE);
				g_list_free(collision_list);
				collision_list = NULL;

				return TRUE;
			} else
			{
				// There are further collision hits, so we just move the selection to the next one
				collision_list = g_list_next(collision_list);
				current_slide_data->layers = g_list_first(current_slide_data->layers);
				tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
				g_string_printf(tmp_gstring, "%d", tmp_int);
				tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
				gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

				// Draw a bounding box around the selected object
				// fixme4: Better to do this later, driven from changes in the timelines selected row
//				draw_bounding_box(widget, ((boundary_box *) collision_list->data)->region_ptr);

				// Free the memory allocated during the collision detection
				g_string_free(tmp_gstring, TRUE);
				g_list_free(collision_list);
				collision_list = NULL;

				return TRUE;
			}
		}
	}

	// * To get here, the presently selected layer wasn't in the collision list *

	// The presently selected row is not in the collision list, so move the selection row to the first collision
	collision_list = g_list_first(collision_list);
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
	g_string_printf(tmp_gstring, "%d", tmp_int);
	tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

	// Free the memory allocated during the collision detection
	g_string_free(tmp_gstring, TRUE);
	g_list_free(collision_list);
	collision_list = NULL;

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.11  2008/03/06 01:09:15  vapour
 * Fixed a bug in setting the initial values for the invalidation area.
 *
 * Revision 1.10  2008/03/05 13:47:51  vapour
 * Tweaked the starting position for the double buffering invalidation area.
 *
 * Revision 1.9  2008/03/05 13:23:43  vapour
 * Now resets the double buffering invalidation area.
 *
 * Revision 1.8  2008/03/05 12:49:46  vapour
 * Renamed the old backing store variable and associated comments to front store.
 *
 * Revision 1.7  2008/03/05 10:20:43  vapour
 * Reversed the previous commit, and included code to save the mouse coordinates when we are creating a new highlight layer.
 *
 * Revision 1.6  2008/03/05 09:21:20  vapour
 * Added initial code to recognise button release events for the creation of a new image layer.
 *
 * Revision 1.5  2008/02/04 17:21:32  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.4  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/10/06 11:36:39  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.2  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

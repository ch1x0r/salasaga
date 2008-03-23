/*
 * $Id$
 *
 * Salasaga: Function called when the user presses the mouse button on the drawing area 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "calculate_object_boundaries.h"
#include "detect_collisions.h"
#include "draw_handle_box.h"
#include "layer_edit.h"


gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GList				*collision_list = NULL;
	guint				count_int;
	slide				*current_slide_data;		// Alias to make things easier
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	guint				num_collisions;
	guint				num_layers;
	guint				selected_row;				// Holds the number of the row that is selected

	guint				tmp_int;					// Temporary integer
	GtkTreeViewColumn	*tmp_column;				// Temporary column
	GString				*tmp_gstring;				// Temporary GString
	GtkTreePath			*tmp_path;					// Temporary path


	// Only do this function if we have a front store available
	if (NULL == front_store)
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;

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
	calculate_object_boundaries();
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

		// Clear any existing handle box
		gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
				GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);

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
			// Return if the presently selected row is in the collision list, as we don't want to change our selected layer
			return TRUE;
		}
	}

	// * To get here, the presently selected layer wasn't in the collision list *

	// The presently selected row is not in the collision list, so move the selection row to the first collision
	collision_list = g_list_first(collision_list);
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	selected_row = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
	g_string_printf(tmp_gstring, "%d", selected_row);
	tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

	// Draw a handle box around the new selected object
	draw_handle_box();

	// Free the memory allocated during the collision detection
	g_string_free(tmp_gstring, TRUE);
	g_list_free(collision_list);
	collision_list = NULL;

	return TRUE;
}

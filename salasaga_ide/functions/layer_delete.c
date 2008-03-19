/*
 * $Id$
 *
 * Salasaga: Function which deletes the layer the user presently has selected 
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
#include "display_warning.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"


void layer_delete(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkListStore			*list_pointer;				//
	GtkWidget			*list_widget;				// Points to the timeline widget
	guint				num_layers;				// Number of layers
	guint				selected_row;				// Holds the number of the row that is selected

	gboolean			tmp_bool;				// Temporary boolean
	GtkTreeViewColumn		*tmp_column;				// Temporary column
	GtkTreePath			*tmp_path;				// Temporary path
	layer				*tmp_layer;				// Temporary layer


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;
	list_pointer = ((slide *) current_slide->data)->layer_store;
	list_widget = ((slide *) current_slide->data)->timeline_widget;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));

	// If the background layer is selected, don't delete it
	if (1 == (num_layers - selected_row))
	{
		// Give the user a warning, then return
		gdk_beep();
		display_warning("Error ED38: The background layer can not be deleted\n");
		return;
	}

	// Remove the layer from the Timeline widget
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);
	tmp_bool = gtk_list_store_remove(list_pointer, tmp_layer->row_iter);

	// Remove the layer from the layer structure
	layer_pointer = g_list_remove(layer_pointer, tmp_layer);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Make the row above in the Timeline widget selected
	if (0 != selected_row)
	{
		selected_row = selected_row - 1;
	}
	tmp_path = gtk_tree_path_new_from_indices(selected_row, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(list_widget), tmp_path, NULL, FALSE);

	// Redraw the workspace area
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Free the storage allocated by this function
	gtk_tree_path_free(tmp_path);

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer deleted");
	gdk_flush();

	return;
}

/*
 * $Id$
 *
 * Salasaga: Copies the presently selected layer into the copy buffer
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "layer_duplicate.h"
#include "layer_free.h"


void layer_copy(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	guint				selected_layer;				// Holds the number of the layer that is selected
	layer				*this_layer;				// Pointer to the presently selected layer
	GtkTreePath			*tmp_path;					// Temporary path


	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;

	// Determine which layer the user has selected in the timeline
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, NULL);
	selected_layer = atoi(gtk_tree_path_to_string(tmp_path));
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	this_layer = g_list_nth_data(current_slide_data->layers, selected_layer);

	// If there's presently a layer in the copy buffer we destroy it
	if (NULL != copy_layer)
	{
		layer_free(this_layer);
		copy_layer = NULL;
	}

	// Create a new layer
	copy_layer = layer_duplicate(this_layer);
	if (NULL == copy_layer)
	{
		// Something went wrong duplicating the existing layer.  Not much we can do
		return;
	}

	return;
}

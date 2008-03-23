/*
 * $Id$
 *
 * Salasaga: Copies the contents of the copy buffer into the selected slide
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "draw_workspace.h"
#include "layer_duplicate.h"
#include "regenerate_film_strip_thumbnails.h"
#include "regenerate_timeline_duration_images.h"


void layer_paste(void)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkTreeIter			*new_iter;					// New iter
	layer				*new_layer;					// Newly created layer
	GtkTreePath			*new_path;					// New GtkPath
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	slide				*slide_data;				// Pointer to the data for the current slide


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Create a new layer
	new_layer = layer_duplicate(copy_layer);
	if (NULL == new_layer)
	{
		// Something went wrong duplicating the copy buffer layer, so exit
		return;
	}

	// Simplify pointers
	slide_data = current_slide->data;
	layer_pointer = slide_data->layers;
	finish_frame = new_layer->finish_frame;

	// Insert it into the present slide
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, new_layer);

	// If the finish frame for the copied layer is longer than the present slide duration, we need to increase the slide duration to match
	if (finish_frame > slide_data->duration)
	{
		// Update slide duration data
		slide_data->duration = finish_frame;
	}

	// Add the new layer to slide list store
	new_iter = g_new0(GtkTreeIter, 1);
	new_layer->row_iter = new_iter;
	gtk_list_store_prepend(slide_data->layer_store, new_iter);
	gtk_list_store_set(slide_data->layer_store, new_iter,
						TIMELINE_NAME, new_layer->name->str,
						TIMELINE_VISIBILITY, new_layer->visible,
						TIMELINE_DURATION, NULL,
						TIMELINE_X_OFF_START, new_layer->x_offset_start,
						TIMELINE_Y_OFF_START, new_layer->y_offset_start,
						TIMELINE_X_OFF_FINISH, new_layer->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, new_layer->y_offset_finish,
						-1);

	// Regenerate the timeline duration images
	regenerate_timeline_duration_images(slide_data);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Select the new layer in the timeline widget
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), new_path, NULL, FALSE);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer pasted from buffer");
	gdk_flush();
	return;
}

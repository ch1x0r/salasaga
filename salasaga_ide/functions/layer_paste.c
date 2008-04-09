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
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "film_strip_create_thumbnail.h"
#include "layer_duplicate.h"
#include "widgets/time_line.h"


void layer_paste(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkTreeIter			*new_iter;					// New iter
	layer				*new_layer;					// Newly created layer
	slide				*slide_data;				// Pointer to the data for the current slide


	// If no project is loaded then don't run this function
	if (FALSE == project_active)
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

	// Insert it into the present slide
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, new_layer);

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if ((new_layer->start_time + new_layer->duration + new_layer->transition_in_duration + new_layer->transition_out_duration) > slide_data->duration)
	{
		slide_data->duration = new_layer->start_time + new_layer->duration + new_layer->transition_in_duration + new_layer->transition_out_duration;
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

	// Increase the layer counter for the slide
	slide_data->num_layers++;

	// Regenerate the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Select the new layer in the timeline widget
	time_line_set_selected_layer_num(slide_data->timeline_widget, 0);

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer pasted from buffer");
	gdk_flush();
	return;
}

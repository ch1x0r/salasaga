/*
 * $Id$
 *
 * Salasaga: Copies the presently selected layer into the copy buffer
 * 
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 * 
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../widgets/time_line.h"
#include "layer_duplicate.h"
#include "layer_free.h"


void layer_copy(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	guint				selected_layer;				// Holds the number of the layer that is selected
	layer				*this_layer;				// Pointer to the presently selected layer


	// Initialise some things
	current_slide_data = current_slide->data;

	// Determine which layer the user has selected in the timeline
	selected_layer = time_line_get_selected_layer_num(current_slide_data->timeline_widget);
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	this_layer = g_list_nth_data(current_slide_data->layers, selected_layer);

	// If there is already a layer in the copy buffer, then we free it
	if (NULL != copy_layer)
	{
		layer_free(copy_layer);
	}

	// Create a new layer
	copy_layer = layer_duplicate(this_layer);
	if (NULL == copy_layer)
	{
		// Something went wrong duplicating the existing layer.  Not much we can do
		gdk_beep();
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Copy failed"));
		gdk_flush();
		return;
	}

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer copied to buffer"));
	gdk_flush();
	return;
}

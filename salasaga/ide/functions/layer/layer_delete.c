/*
 * $Id$
 *
 * Salasaga: Function which deletes the layer the user presently has selected
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../draw_timeline.h"
#include "../widget_focus.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_duplicate.h"
#include "../undo_redo/undo_functions.h"
#include "../widgets/time_line/time_line_get_selected_layer_num.h"
#include "../working_area/draw_workspace.h"
#include "layer_free.h"


void layer_delete(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	guint				num_layers;					// Number of layers
	guint				selected_row;				// Holds the number of the row that is selected
	layer				*tmp_layer;					// Temporary layer
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// If no project is loaded then don't run this function
	if (FALSE == get_project_active())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Change the focus of the window to be this widget
	set_delete_focus(FOCUS_LAYER);

	// Determine the number of layers present in this slide
	num_layers = ((slide *) current_slide->data)->num_layers;

	// Determine which layer the user has selected in the timeline
	selected_row = time_line_get_selected_layer_num(((slide *) current_slide->data)->timeline_widget);

	// If the background layer is selected, don't delete it
	if (1 == (num_layers - selected_row))
	{
		// Give the user a warning beep, then return
		gdk_beep();
		return;
	}

	// Set up pointers
	layer_pointer = g_list_first(layer_pointer);
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);

	// Create and store the undo history item for this layer
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_new = NULL;  // NULL means not set
	undo_item_data->layer_data_old = layer_duplicate(tmp_layer);
	undo_item_data->position_new = -1;  // -1 means not set
	undo_item_data->position_old = 0;
	undo_item_data->slide_data = current_slide->data;
	undo_history_add_item(UNDO_DELETE_LAYER, undo_item_data, TRUE);

	// Remove the layer from the layer structure
	layer_pointer = g_list_remove(layer_pointer, tmp_layer);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Free the memory allocated to the layer
	layer_free(tmp_layer);

	// Decrement the number of layers counter
	((slide *) current_slide->data)->num_layers--;

	// Redraw the timeline area
	draw_timeline();

	// Redraw the workspace area
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail((slide *) current_slide->data);

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Layer deleted"));
	gdk_flush();

	return;
}

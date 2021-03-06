/*
 * $Id$
 *
 * Salasaga: Displays a dialog box asking for the values required to make a new mouse layer
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_duplicate.h"
#include "../preference/application_preferences.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_set_selected_layer_num.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_workspace.h"


void layer_new_mouse_inner(guint release_x, guint release_y)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	slide				*slide_data;				// Pointer to the data for the current slide
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	layer				*tmp_layer;					// Temporary layer
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Change the cursor back to normal
	gdk_window_set_cursor(get_main_drawing_area()->window, NULL);

	// * Create a new mouse layer in memory using reasonable defaults *

	// Simplify pointing to the current slide structure in memory
	slide_data = get_current_slide_data();

	// Create the mouse pointer layer data
	tmp_mouse_ob = g_new(layer_mouse, 1);
	tmp_mouse_ob->width = 22;
	tmp_mouse_ob->height = 32;
	tmp_mouse_ob->click = MOUSE_NONE;

	// Constuct the new mouse pointer layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_MOUSE_CURSOR;
	tmp_layer->object_data = (GObject *) tmp_mouse_ob;
	tmp_layer->start_time = 0.0;
	tmp_layer->duration = get_default_layer_duration();
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new(_("Mouse Pointer"));
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new(_("_self"));
	tmp_layer->transition_in_type = TRANS_LAYER_NONE;
	tmp_layer->transition_in_duration = 0.0;
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Create and store the undo history item for this layer
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_new = layer_duplicate(tmp_layer);
	undo_item_data->layer_data_old = NULL;  // NULL means not set
	undo_item_data->position_new = 0;
	undo_item_data->position_old = -1;  // -1 means not set
	undo_item_data->slide_data = get_current_slide_data();
	undo_history_add_item(UNDO_INSERT_LAYER, undo_item_data, TRUE);

	// Add the new layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);
	slide_data->num_layers++;

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if (tmp_layer->duration > slide_data->duration)
	{
		// Change the slide duration
		slide_data->duration = tmp_layer->duration;

		// Change the background layer duration
		tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
		tmp_layer->duration = slide_data->duration;
	}

	// Regenerate the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Select the new layer in the timeline widget
	time_line_set_selected_layer_num(slide_data->timeline_widget, 0);

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Mouse layer added"));
	gdk_flush();
}

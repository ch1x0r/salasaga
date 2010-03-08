/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Move Layer Down toolbar button
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
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../other/widget_focus.h"
#include "../time_line/draw_timeline.h"
#include "../time_line/time_line_get_selected_layer_num.h"
#include "../time_line/time_line_set_selected_layer_num.h"
#include "../working_area/draw_workspace.h"


void layer_move_down(void)
{
	// Local variables
	GList				*below_layer;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				num_layers;					// The number of layers in the select slide
	GList				*our_layer;					// The selected layer
	gint				selected_row;				// Holds the row that is selected


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise various things
	layer_pointer = get_current_slide_layers_pointer();

	// Change the focus of the window to be this widget
	set_delete_focus(FOCUS_LAYER);

	// Determine which layer the user has selected in the timeline
	num_layers = get_current_slide_num_layers();
	selected_row = time_line_get_selected_layer_num(get_current_slide_timeline_widget());
	if (num_layers - 2 <= selected_row)
	{
		// We're already at the bottom of the list or the background layer is selected, so return
		gdk_beep();
		return;
	}

	// Get details of the layers we're moving around
	layer_pointer = g_list_first(layer_pointer);
	our_layer = g_list_nth(layer_pointer, selected_row);
	below_layer = g_list_nth(layer_pointer, selected_row + 1);

	// Move the row down one in the layer list
	layer_pointer = g_list_remove_link(layer_pointer, below_layer);
	layer_pointer = g_list_insert_before(layer_pointer, our_layer, below_layer->data);
	set_current_slide_layers_pointer(layer_pointer);

	// Move the row down one in the timeline widget
	time_line_set_selected_layer_num(get_current_slide_timeline_widget(), selected_row + 1);

	// Redraw the timeline area
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(get_current_slide_data());

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Layer moved down"));
	gdk_flush();
}

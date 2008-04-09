/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Move Layer Down toolbar button 
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
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "film_strip_create_thumbnail.h"
#include "widgets/time_line.h"


void layer_move_down(void)
{
	// Local variables
	GList				*below_layer;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				num_layers;					// The number of layers in the select slide
	GList				*our_layer;					// The selected layer
	gint				selected_row;				// Holds the row that is selected
	GList				*tmp_glist;					// Temporary GList


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise various things
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Determine which layer the user has selected in the timeline
	num_layers = ((slide *) current_slide->data)->num_layers;
	selected_row = time_line_get_selected_layer_num(((slide *) current_slide->data)->timeline_widget);
	if (num_layers - 2 <= selected_row)
	{
		// We're already at the bottom of the list or the background layer is selected, so return
		gdk_beep();
		display_warning("Error ED40: This layer is already at the bottom\n");
		return;
	}

	// Get details of the layers we're moving around
	layer_pointer = g_list_first(layer_pointer);
	our_layer = g_list_nth(layer_pointer, selected_row);
	below_layer = g_list_nth(layer_pointer, selected_row + 1);

	// Move the row down one in the timeline widget
	time_line_set_selected_layer_num(((slide *) current_slide->data)->timeline_widget, selected_row + 1);

	// Move the row down one in the layer list
	tmp_glist = g_list_remove_link(layer_pointer, below_layer);
	layer_pointer = g_list_insert_before(layer_pointer, our_layer, below_layer->data);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Redraw the timeline area
	gtk_widget_destroy(GTK_WIDGET(((slide *) current_slide->data)->timeline_widget));
	((slide *) current_slide->data)->timeline_widget = NULL;
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail((slide *) current_slide->data);

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer moved down");
	gdk_flush();
}

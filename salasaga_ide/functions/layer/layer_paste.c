/*
 * $Id$
 *
 * Salasaga: Copies the contents of the copy buffer into the selected slide
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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
#include "../../externs.h"
#include "../draw_timeline.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../widgets/time_line.h"
#include "../working_area/draw_workspace.h"
#include "layer_duplicate.h"


void layer_paste(void)
{
	// Local variables
	gfloat				end_time;					// Used to calculate the end time in seconds of a layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	layer				*new_layer;					// Newly created layer
	slide				*slide_data;				// Pointer to the data for the current slide
	layer				*tmp_layer;					// Temporary layer


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

	// Increase the layer counter for the slide
	slide_data->num_layers++;

	// Work out the end time in seconds of the layer
	end_time = new_layer->start_time + new_layer->duration;
	if (TRANS_LAYER_NONE != new_layer->transition_in_type)
		end_time += new_layer->transition_in_duration;
	if (TRANS_LAYER_NONE != new_layer->transition_out_type)
		end_time += new_layer->transition_out_duration;

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if (end_time > slide_data->duration)
	{
		// Change the slide duration
		slide_data->duration = end_time;

		// Change the background layer duration
		layer_pointer = g_list_first(layer_pointer);
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
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, _(" Layer pasted from buffer"));
	gdk_flush();
	return;
}

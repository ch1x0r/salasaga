/*
 * bot_right_button_press_event.c
 *
 *  Created on: Jun 9, 2010
 *      Author: althaf
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

// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

#include <math.h>
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../layer/layer_duplicate.h"
#include "../layer/layer_edit.h"
#include "../other/widget_focus.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_handle_box.h"
#include "../working_area/draw_layer_start_and_end_points.h"
#include "../working_area/draw_workspace.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "time_line.h"
#include "time_line_get_cursor_position.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_set_cursor_position.h"
#include "time_line_set_selected_layer_num.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_layer_name.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_invalidate_layer_area.h"
#include "time_line_internal_redraw_bg_area.h"
#include "time_line_internal_redraw_layer_bg.h"
#include "time_line_internal_draw_cursor.h"
#include "time_line_internal_initialise_bg_image.h"
#include "draw_timeline.h"


void bot_right_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_border;
	gint				new_row;					// Used to determine the row clicked upon
	gint				pps;						// Holds the number of pixels per second used when drawing
	TimeLinePrivate		*priv;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in

	priv = data;

	if (NULL == widget)
	{
		return;
	}

	// Set the delete focus to be layers
	set_delete_focus(FOCUS_LAYER);

	// Change the focus of the window to be this widget
	gtk_widget_grab_focus(GTK_WIDGET(widget));

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}


	this_slide_data = get_current_slide_data();
	layer_pointer = this_slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	left_border = time_line_get_left_border_width(priv);
	pps = time_line_get_pixels_per_second();


	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();
		return;
	}

	// Figure out which row the user has selected in the timeline area
	new_row = floor(event->y  / priv->row_height);

	// Ensure the user clicked on a valid row
	if (0 > new_row)
	{
		// Too low, the user didn't click on a valid row
		return;
	}
	if (this_slide_data->num_layers <= new_row)
	{
		// Too high, the user didn't click on a valid row
		return;
	}

	// The user clicked on a valid row, so update the selection
	time_line_set_selected_layer_num(GTK_WIDGET(priv->main_table->parent), new_row);
	this_layer_data = g_list_nth_data(layer_pointer, new_row);

	// Create an undo history item and store the existing layer data in it
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_old = layer_duplicate(this_layer_data);
	undo_item_data->position_old = new_row;
	undo_item_data->slide_data = this_slide_data;
	time_line_set_undo_item(undo_item_data);

	// Calculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Store the guide line positions so we know where to refresh
	priv->guide_line_start = (this_layer_data->start_time * pps);
	priv->guide_line_end = (end_time * pps);

	// Draw guide lines
	time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_start);
	time_line_internal_draw_guide_line(GTK_WIDGET(priv->main_table->parent), priv->guide_line_end);


	draw_handle_box();

	// If it's not the background layer that is selected then update things in the working area
	if (this_slide_data->num_layers > (new_row + 1))
	{
		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();
	} else
	{
		// Background layer was clicked, so clear any start and end points shown in the working area
		gdk_draw_drawable(GDK_DRAWABLE(get_main_drawing_area()->window), GDK_GC(get_main_drawing_area()->style->fg_gc[GTK_WIDGET_STATE(get_main_drawing_area())]),
				GDK_PIXMAP(get_front_store()), 0, 0, 0, 0, -1, -1);
	}


}

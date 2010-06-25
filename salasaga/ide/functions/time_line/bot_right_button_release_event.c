/*
 * bot_right_button_release_event.c
 *  Created on: Jun 9, 2010    Author: althaf
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

// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

#include <math.h>
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../layer/layer_edit.h"
#include "../layer/layer_duplicate.h"
#include "../other/widget_focus.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_handle_box.h"
#include "../working_area/draw_layer_start_and_end_points.h"
#include "../working_area/draw_workspace.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "time_line.h"
#include "time_line_set_cursor_position.h"
#include "time_line_set_selected_layer_num.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_name.h"
#include "time_line_internal_invalidate_layer_area.h"
#include "time_line_internal_redraw_bg_area.h"
#include "time_line_internal_redraw_layer_bg.h"
#include "time_line_internal_draw_cursor.h"
#include "top_right_button_release_event.h"
#include "draw_timeline.h"

void bot_right_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	layer				*background_layer_data;		// Data for the background layer
	GdkModifierType		button_state;				// Mouse button states
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GtkAllocation		area;						// Area covered by an individual guide line
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	TimeLinePrivate		*priv;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in

	priv = data;
	if (NULL == widget)
	{
		return;
	}

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}
	if (TRUE == priv->cursor_drag_active)
	{
		// Note that the cursor drag has finished
		priv->cursor_drag_active = FALSE;
		top_right_button_release_event(widget,event,data);
		return;
	}
	if(TRUE == priv->left_resize_active){
				priv->left_resize_active = FALSE;
				return;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	area.x = priv->guide_line_start;
	area.y = 0;
	area.height = GTK_WIDGET(widget)->allocation.height;
	area.width = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
	area.x = priv->guide_line_end;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);


	if (RESIZE_NONE != priv->resize_type)
		{
			// Note that the resize has finished
			priv->resize_type = RESIZE_NONE;

			// Mark that there are unsaved changes
			set_changes_made(TRUE);

			// Remove the resize guideline from the widget
			area.x = priv->guide_line_resize;
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
			priv->guide_line_resize = 0;

			// Create pointers to things we're working with
			this_slide_data = get_current_slide_data();
			layer_pointer = this_slide_data->layers;
			layer_pointer = g_list_first(layer_pointer);
			this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);

			// Calculate the end time of the layer (in seconds)
			end_time = this_layer_data->start_time + this_layer_data->duration;
			if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
				end_time += this_layer_data->transition_in_duration;
			if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
				end_time += this_layer_data->transition_out_duration;

			// Check if the new end time is longer than the slide duration
			if (end_time > priv->stored_slide_duration)
			{
				// The new slide duration is longer than the old one, so update the slide and background layer to match
				this_slide_data->duration = priv->stored_slide_duration = end_time;
				end_row = this_slide_data->num_layers - 1;
				background_layer_data = g_list_nth_data(layer_pointer, end_row);
				background_layer_data->duration = priv->stored_slide_duration;

				// Refresh the time line display of the background layer
				time_line_internal_redraw_layer_bg(priv, end_row);
				time_line_internal_draw_layer_name(priv, end_row);
				time_line_internal_draw_layer_duration(priv, end_row);
				time_line_internal_invalidate_layer_area(GTK_WIDGET(priv->main_table->parent), end_row);
			}

			// Use the status bar to communicate the resize has completed
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Resize completed"));
			gdk_flush();

			// Store the undo item created in the button click event handler function
			undo_item_data = time_line_get_undo_item();
			undo_item_data->position_new = priv->selected_layer_num;
			undo_item_data->layer_data_new = layer_duplicate(this_layer_data);
			undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);
			draw_timeline();
		}


	if (TRUE == priv->drag_active)
	{
		// Note that the drag has finished
		priv->drag_active = FALSE;
		// Mark that there are unsaved changes
		set_changes_made(TRUE);
		// Create pointers to things we're working with
		this_slide_data = get_current_slide_data();
		layer_pointer = this_slide_data->layers;
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);
		// Calculate the end time of the layer (in seconds)
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;
		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
		// The new slide duration is longer than the old one, so update the slide and background layer to match
		this_slide_data->duration = priv->stored_slide_duration = end_time;
		end_row = this_slide_data->num_layers - 1;
		background_layer_data = g_list_nth_data(layer_pointer, end_row);
		background_layer_data->duration = priv->stored_slide_duration;
		// Refresh the time line display of the background layer
		time_line_internal_redraw_layer_bg(priv, end_row);
		time_line_internal_draw_layer_name(priv, end_row);
		time_line_internal_draw_layer_duration(priv, end_row);
		time_line_internal_invalidate_layer_area(GTK_WIDGET(priv->main_table->parent), end_row);
		}

		// Use the status bar to communicate the drag has completed
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Drag completed"));

		gdk_flush();

		// Store the undo item created in the button click event handler function
		undo_item_data = time_line_get_undo_item();
		undo_item_data->position_new = priv->selected_layer_num;
		undo_item_data->layer_data_new = layer_duplicate(this_layer_data);
		undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);
		// For Updating the scoll bars, just redraw the same timeline / only if it is a drag event
		draw_timeline();
		}

		// Recreate the film strip thumbnail
		film_strip_create_thumbnail(get_current_slide_data());

		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();
		//draw_workspace();

}

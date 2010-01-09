/*
 * $Id$
 *
 * Salasaga: Callback function for when the user presses the mouse button on the time line widget
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "../../widget_focus.h"
#include "../../layer/layer_edit.h"
#include "../../working_area/draw_handle_box.h"
#include "../../working_area/draw_layer_start_and_end_points.h"
#include "../../working_area/draw_workspace.h"
#include "time_line.h"
#include "time_line_get_cursor_position.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_guide_line.h"
#include "time_line_set_cursor_position.h"
#include "time_line_set_selected_layer_num.h"


void time_line_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GtkAllocation		area;						// Rectangular area
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				left_border;
	gint				new_row;					// Used to determine the row clicked upon
	gint				pps;						// Holds the number of pixels per second used when drawing
	TimeLinePrivate		*priv;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	TimeLine			*this_time_line;
	gfloat				tl_cursor_pos;				// Holds the position of the cursor in the time line (in seconds)
	GList				*tmp_glist = NULL;			// Is given a list of child widgets, if any exist


	// Safety check
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

	// It's probably the child of the called widget that we need to get data from
	if (FALSE == IS_TIME_LINE(widget))
	{
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(widget));
		if (NULL == tmp_glist)
			return;
		if (FALSE == IS_TIME_LINE(tmp_glist->data))
		{
			g_list_free(tmp_glist);
			return;
		}

		// The child is the TimeLine widget
		this_time_line = TIME_LINE(tmp_glist->data);
	} else
	{
		// This is a time line widget
		this_time_line = TIME_LINE(widget);
	}

	// Initialisation
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	this_slide_data = ((slide *) current_slide->data);
	layer_pointer = this_slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	left_border = time_line_get_left_border_width(priv);
	pps = time_line_get_pixels_per_second();
	tl_cursor_pos = time_line_get_cursor_position(GTK_WIDGET(this_time_line));

	// Check if this button press is in time line cursor area
	if ((ADJUSTMENTS_Y <= event->y) && ((ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= event->y) && (left_border < event->x))
	{
		// * Direct click in the cursor area *

		// Remove the old cursor line
		area.x = left_border + (tl_cursor_pos * pps) - (CURSOR_HEAD_WIDTH / 2);
		area.y = 0;
		area.height = GTK_WIDGET(this_time_line)->allocation.height;
		area.width = CURSOR_HEAD_WIDTH;
		gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);

		// Reposition the cursor to where the mouse button down occurred
		tl_cursor_pos = round(event->x - left_border) / pps;
		time_line_set_cursor_position(GTK_WIDGET(this_time_line), tl_cursor_pos);
		priv->cursor_drag_active = FALSE;

		// Draw the new cursor line
		area.x = left_border + (tl_cursor_pos * pps) - (CURSOR_HEAD_WIDTH / 2);
		area.y = 0;
		area.height = GTK_WIDGET(this_time_line)->allocation.height;
		area.width = CURSOR_HEAD_WIDTH;
		gdk_window_invalidate_rect(GTK_WIDGET(this_time_line)->window, &area, TRUE);

		// Update the workspace area
		draw_workspace();
		g_list_free(tmp_glist);
		return;
	}

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();
		g_list_free(tmp_glist);
		return;
	}

	// Figure out which row the user has selected in the timeline area
	new_row = floor((event->y - priv->top_border_height) / priv->row_height);

	// Ensure the user clicked on a valid row
	if (0 > new_row)
	{
		// Too low, the user didn't click on a valid row
		g_list_free(tmp_glist);
		return;
	}
	if (this_slide_data->num_layers <= new_row)
	{
		// Too high, the user didn't click on a valid row
		g_list_free(tmp_glist);
		return;
	}

	// The user clicked on a valid row, so update the selection
	time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

	// Calculate the present end time of the layer (in seconds)
	this_layer_data = g_list_nth_data(layer_pointer, new_row);
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Store the guide line positions so we know where to refresh
	priv->guide_line_start = left_border + (this_layer_data->start_time * pps);
	priv->guide_line_end = left_border + (end_time * pps) - 1;

	// Draw guide lines
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_start);
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_end);

	// Draw a handle box around the newly selected row in the time line area
	draw_handle_box();

	// If it's not the background layer that is selected then update things in the working area
	if (this_slide_data->num_layers > (new_row + 1))
	{
		// Draw the start and end points for the layer
		draw_layer_start_and_end_points();
	} else
	{
		// Background layer was clicked, so clear any start and end points shown in the working area
		gdk_draw_drawable(GDK_DRAWABLE(main_drawing_area->window), GDK_GC(main_drawing_area->style->fg_gc[GTK_WIDGET_STATE(main_drawing_area)]),
				GDK_PIXMAP(front_store), 0, 0, 0, 0, -1, -1);
	}

	// Free the memory allocated in this function
	g_list_free(tmp_glist);
}

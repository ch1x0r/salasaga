/*
 * top_right_button_press_event.c
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
#include "../working_area/draw_workspace.h"
#include "time_line.h"
#include "time_line_get_cursor_position.h"
#include "time_line_set_cursor_position.h"
#include "time_line_internal_draw_cursor.h"

#include "draw_timeline.h"


void top_right_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GtkAllocation		area;						// Rectangular area
	gint				pps;						// Holds the number of pixels per second used when drawing
	TimeLinePrivate		*priv;
	gfloat				tl_cursor_pos;				// Holds the position of the cursor in the time line (in seconds)

	// Change the focus of the window to be this widget
	gtk_widget_grab_focus(GTK_WIDGET(widget));

	priv = data;
	tl_cursor_pos = priv->cursor_position;
	pps = time_line_get_pixels_per_second();

	area.x =  (tl_cursor_pos * pps) - (CURSOR_HEAD_WIDTH / 2);
	area.y = 0;
	area.height = GTK_WIDGET(widget)->allocation.height;
	area.width = CURSOR_HEAD_WIDTH;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

	area.height = GTK_WIDGET(priv->bot_right_evb)->allocation.height;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

	// Reposition the cursor to where the mouse button down occurred
	tl_cursor_pos = event->x/pps;
	priv->cursor_position = tl_cursor_pos;
	priv->cursor_drag_active = FALSE;

	time_line_internal_draw_cursor(priv->main_table->parent,event->x);

	// Draw the new cursor line
	area.x = (tl_cursor_pos * pps) - (CURSOR_HEAD_WIDTH / 2);
	area.y = 0;
	area.height = GTK_WIDGET(widget)->allocation.height;
	area.width = CURSOR_HEAD_WIDTH;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
	area.height = GTK_WIDGET(priv->bot_right_evb)->allocation.height;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

	draw_timeline();
	// Update the workspace area
	//draw_workspace();
	return;

}

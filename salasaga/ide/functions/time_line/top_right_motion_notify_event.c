/*
 * top_right_motion_notify_event.c
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
#include "time_line.h"
#include "time_line_internal_draw_cursor.h"
#include "draw_timeline.h"


void top_right_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	TimeLinePrivate		*priv;
	GtkAllocation		area;						// Rectangular area
	guint 				temp_int;
	gint 				distance_moved;
	gfloat				time_moved;
	gint				pps;						// Holds the number of pixels per second used when drawing
	priv = data;
	pps = time_line_get_pixels_per_second();

	if(TRUE == priv->left_resize_active){
		top_left_motion_notify_event(priv->top_left_evb,event,data);
		return;
	}
	if(FALSE == priv->cursor_drag_active)
	{
		priv->cursor_drag_active = TRUE;
		priv->stored_x = event->x;
	}
	else
	{
		temp_int = CLAMP(event->x, 0, GTK_WIDGET(widget)->allocation.width);
		distance_moved = priv->stored_x - event->x;
		time_moved = ((gfloat) distance_moved) / pps;

		area.x = (priv->cursor_position * pps) - (CURSOR_HEAD_WIDTH);
		area.y = 0;
		area.height = GTK_WIDGET(widget)->allocation.height;
		area.width = CURSOR_HEAD_WIDTH * 2;
		gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

		// Update the cursor position
		priv->cursor_position = priv->cursor_position - time_moved;

		// Safety check
		if (0 > priv->cursor_position)
			priv->cursor_position=0;
		time_line_internal_draw_cursor(priv->main_table->parent,priv->cursor_position * pps);

		area.x = (priv->cursor_position * pps) - (CURSOR_HEAD_WIDTH );
		gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
				priv->stored_x = event->x;

		draw_timeline();
	}

}

/*
 * top_left_button_press_event.c
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

#define _ISOC99_SOURCE

#include <math.h>
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"


void top_left_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	TimeLinePrivate		*priv;
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	GdkModifierType		button_state;				// Mouse button states
	priv = data;

	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);
		if(event->x > (priv->left_border_width-5))
		{
			priv->left_resize_active = TRUE;
			priv->stored_x = mouse_x;
		}

}

/*
 * top_right_button_release_event.c
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
#include "../working_area/draw_layer_start_and_end_points.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "time_line.h"
#include "draw_timeline.h"
void top_right_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	TimeLinePrivate		*priv;
	priv = data;
	if (NULL == widget)
	{
		return;
	}
	if (TRUE == priv->cursor_drag_active)
	{
		// Note that the cursor drag has finished
		priv->cursor_drag_active = FALSE;
	}
	else{
		draw_timeline();
	}
	if(TRUE == priv->left_resize_active){
			priv->left_resize_active = FALSE;
	}
	film_strip_create_thumbnail(get_current_slide_data());

	// Draw the start and end points for the layer
	draw_layer_start_and_end_points();

}

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

void top_left_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	TimeLinePrivate		*priv;
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	GdkModifierType		button_state;				// Mouse button states
	GString 			*message;
	priv = data;

	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);
		if(event->x > (priv->left_border_width-5))
		{
//			message = g_string_new(NULL);
//			g_string_printf(message," %f ----  %d ",event->x,mouse_x);
//			display_warning(message->str);
			priv->left_resize_active = TRUE;
			priv->stored_x = mouse_x;
		}

}

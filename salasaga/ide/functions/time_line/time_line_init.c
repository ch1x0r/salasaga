/*
 * $Id$
 *
 * Salasaga: Instance initialiser for the time line widget
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

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "time_line.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_make_widgets.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_cursor_position.h"
#include "time_line_internal_draw_cursor.h"
#include "../working_area/draw_workspace.h"
#include "draw_timeline.h"
#include "realize_allocate_table.h"
void time_line_init(TimeLine *time_line)
{
	TimeLinePrivate		*priv;
	time_line_set_pixels_per_second(60);
	priv = TIME_LINE_GET_PRIVATE(time_line);
	priv->cached_bg_valid = FALSE;
	priv->cursor_drag_active = FALSE;
	priv->display_buffer_bot_right = NULL;
	priv->drag_active = FALSE;
	priv->resize_type = RESIZE_NONE;
	priv->selected_layer_num = 0;
	priv->stored_x = 0;
	priv->stored_y = 0;
	priv->guide_line_start = 0;
	priv->guide_line_end = 0;

	// Store the slide duration
	priv->stored_slide_duration = get_current_slide_duration();

	// fixme3: These may be better as widget properties
	priv->cursor_position = 0.00;
	priv->left_border_width = 120;
	priv->row_height = 20;
	priv->top_border_height = 15;
	// make all the widgets necessary
	time_line_internal_make_widgets(priv,WIDGET_MINIMUM_WIDTH,WIDGET_MINIMUM_HEIGHT);

	// add the table to the vbox / time line.
	gtk_box_pack_start(GTK_BOX(time_line), GTK_WIDGET(priv->main_table), TRUE, TRUE, 0);

	// allocate the size  / for initializing the size
		realize_allocate_table(get_time_line_container(),priv);

	// Set a periodic time out, so we rate limit the calls to the motion notify (mouse drag) handler
	priv->mouse_x = -1;
	priv->mouse_y = -1;
	priv->timeout_id = g_timeout_add(250,  // Timeout interval (1/1000ths of a second)
							(GSourceFunc) draw_workspace,  // Function to call
							 NULL); //time_line);  // Pass the time line widget to the function
}

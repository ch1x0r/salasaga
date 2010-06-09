/*
 * expose_event_bot_right.c
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

gboolean expose_event_bot_right(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;
	GString				*message;
	TimeLinePrivate		*priv;
	gint				new_cursor_pos;
	guint				width;
	guint				height;
	priv = (TimeLinePrivate *)user;

	if(priv->bot_right_evb->window == NULL || priv->display_buffer_bot_right == NULL){
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED358: %s", _("Error"), _("Null data in window"));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return TRUE;
	}
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->bot_right_evb->window));
	}

	width = ((get_current_slide_duration() +1)  * time_line_get_pixels_per_second());
	height = (get_current_slide_num_layers() *priv->row_height)+22;
	if(width<priv->main_table->allocation.width)
		 width = priv->main_table->allocation.width;
	if(height<priv->main_table->allocation.height)
			 height = priv->main_table->allocation.height;

	new_cursor_pos = round(priv->cursor_position * time_line_get_pixels_per_second());
	time_line_internal_draw_cursor(priv->main_table->parent, new_cursor_pos);

	gdk_draw_drawable(GDK_DRAWABLE(priv->bot_right_evb->window), GDK_GC(this_gc),
	GDK_PIXMAP(priv->display_buffer_bot_right),0,0,0,0,width, height);

	return TRUE;
}

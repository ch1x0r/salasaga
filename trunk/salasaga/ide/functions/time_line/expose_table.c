/*
 * expose_table.c
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
#include "time_line.h"

#include "time_line_internal_draw_cursor.h"

gboolean expose_table(GtkWidget *widget,GdkEventExpose *event,gpointer user_data){
	TimeLinePrivate		*priv;
	gint				new_cursor_pos;
	priv = (TimeLinePrivate *)user_data;

	new_cursor_pos = round(priv->cursor_position * time_line_get_pixels_per_second());
	time_line_internal_draw_cursor(priv->main_table->parent, new_cursor_pos);


	return FALSE;
}

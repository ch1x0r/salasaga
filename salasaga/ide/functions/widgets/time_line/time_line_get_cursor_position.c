/*
 * $Id$
 *
 * Salasaga: Return the time line cursor position
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "time_line.h"


gfloat time_line_get_cursor_position(GtkWidget *widget)
{
	// Local variables
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Safety check
	g_return_val_if_fail(widget != NULL, -1);
	g_return_val_if_fail(IS_TIME_LINE(widget), -1);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	return priv->cursor_position;
}

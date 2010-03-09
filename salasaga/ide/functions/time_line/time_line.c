/*
 * $Id$
 *
 * Salasaga: Functions to access variables private to the time line widget
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


// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"


// Library wide variables
static gint					pixels_per_second;		// Number of pixels used to display each second
static undo_history_data	*undo_item_data = NULL;	// Memory structure undo history items are created in


gint time_line_get_pixels_per_second()
{
	return pixels_per_second;
}

undo_history_data *time_line_get_undo_item()
{
	return undo_item_data;
}

void time_line_set_pixels_per_second(gint pps)
{
	pixels_per_second = pps;

	return;
}

void time_line_set_undo_item(undo_history_data *item_data)
{
	undo_item_data = item_data;

	return;
}

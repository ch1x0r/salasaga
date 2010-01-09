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
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "time_line.h"


// Library wide variables
gint					pixels_per_second;			// Number of pixels used to display each second


gint time_line_get_pixels_per_second()
{
	return pixels_per_second;
}

void time_line_set_pixels_per_second(gint pps)
{
	pixels_per_second = pps;

	return;
}

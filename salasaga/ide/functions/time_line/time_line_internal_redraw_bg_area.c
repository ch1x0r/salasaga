/*
 * $Id$
 *
 * Salasaga: Refresh an area of the display buffer from the cached background image
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
#include "time_line.h"
#include "time_line_internal_initialise_bg_image.h"


gboolean time_line_internal_redraw_bg_area(TimeLinePrivate *priv, gint x1, gint y1, gint width, gint height)
{
	static GdkGC		*display_buffer_gc = NULL;


	// Initialisation
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Ensure the background image we're about to use is valid
	if (TRUE != priv->cached_bg_valid)
	{
		// It's not, so recreate the timeline background image
		time_line_internal_initialise_bg_image(priv, width, height);
	}

	// Refresh the display buffer for the desired area
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc),
			GDK_PIXMAP(priv->cached_bg_image), x1, y1, x1, y1, width, height);

	return TRUE;
}

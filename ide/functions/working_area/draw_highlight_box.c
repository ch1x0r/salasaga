/*
 * $Id$
 *
 * Salasaga: Function to draw a highlight box on a GDK pixbuf 
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"


void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color, guint alpha)
{
	// Local variables
	GdkPixbuf			*highlight_pixbuf;			// GDK Pixbuf used for highlighting
	GString				*message;					// Used to construct message strings


	// Create a horizontal line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, 2);
	if (NULL == highlight_pixbuf)
	{
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED52: %s", _("Error"), _("Not enough memory for pixbuf allocation."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the front store - top line
	gdk_pixbuf_composite(highlight_pixbuf,	// Source pixbuf
		tmp_pixbuf,							// Destination pixbuf
		x_offset,							// X offset
		y_offset,							// Y offset
		width,								// Width
		2,									// Height
		0, 0,								// Source offsets
		1, 1,								// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,					// Scaling type
		alpha);								// Alpha

	// Composite the line onto the front store - bottom line
	gdk_pixbuf_composite(highlight_pixbuf,	// Source pixbuf
		tmp_pixbuf,							// Destination pixbuf
		x_offset,							// X offset
		y_offset + height - 1,				// Y offset
		width,								// Width
		2,									// Height
		0, 0,								// Source offsets
		1, 1,								// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,					// Scaling type
		alpha);								// Alpha

	// Free the temporary pixbuf
	g_object_unref(GDK_PIXBUF(highlight_pixbuf));

	// Create a vertical line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 2, height);
	if (NULL == highlight_pixbuf)
	{
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED53: %s", _("Error"), _("Not enough memory for pixbuf allocation."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the front store - left side
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
		tmp_pixbuf,									// Destination pixbuf
		x_offset,									// X offset
		y_offset,									// Y offset
		2,											// Width
		height,										// Height
		0, 0,										// Source offsets
		1, 1,										// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,							// Scaling type
		alpha);										// Alpha

	// Composite the line onto the front store - right side
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
		tmp_pixbuf,									// Destination pixbuf
		x_offset + width - 1,						// X offset
		y_offset,									// Y offset
		2,											// Width
		height,										// Height
		0, 0,										// Source offsets
		1, 1,										// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,							// Scaling type
		alpha);										// Alpha

	// Free the temporary pixbuf
	g_object_unref(GDK_PIXBUF(highlight_pixbuf));

	// Create the inner highlight box
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width - 2, height - 2);
	gdk_pixbuf_fill(highlight_pixbuf, fill_color);

	// Composite the inner highlight box onto the front store
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
			     tmp_pixbuf,						// Destination pixbuf
			     x_offset + 1,						// X offset
			     y_offset + 1,						// Y offset
			     width - 2,							// Width
			     height - 2,						// Height
			     0, 0,								// Source offsets
			     1, 1,								// Scale factor (1 == no scale)
			     GDK_INTERP_NEAREST,				// Scaling type
			     alpha);							// Alpha

	// Free the temporary pixbuf
	g_object_unref(GDK_PIXBUF(highlight_pixbuf));
	return;
}

/*
 * $Id$
 *
 * Flame Project: Function to draw a highlight box on a GDK pixbuf 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"


void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color)
{
	// Local variables
	GdkPixbuf			*highlight_pixbuf;			// GDK Pixbuf used for highlighting


	// Create a horizontal line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, 2);
	if (NULL == highlight_pixbuf)
	{
		display_warning("Error ED52: Not enough memory for pixbuf allocation");
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the backing store - top line
	gdk_pixbuf_composite(highlight_pixbuf,					// Source pixbuf
		tmp_pixbuf,							// Destination pixbuf
		x_offset,							// X offset
		y_offset,							// Y offset
		width,								// Width
		2,								// Height
		0, 0,								// Source offsets
		1, 1,								// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);								// Alpha

	// Composite the line onto the backing store - bottom line
	gdk_pixbuf_composite(highlight_pixbuf,					// Source pixbuf
		tmp_pixbuf,							// Destination pixbuf
		x_offset,							// X offset
		y_offset + height - 1,						// Y offset
		width,								// Width
		2,								// Height
		0, 0,								// Source offsets
		1, 1,								// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);								// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);

	// Create a vertical line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 2, height);
	if (NULL == highlight_pixbuf)
	{
		display_warning("Error ED53: Not enough memory for pixbuf allocation");
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the backing store - left side
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
		tmp_pixbuf,									// Destination pixbuf
		x_offset,									// X offset
		y_offset,									// Y offset
		2,											// Width
		height,										// Height
		0, 0,										// Source offsets
		1, 1,										// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,							// Scaling type
		255);										// Alpha

	// Composite the line onto the backing store - right side
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
		tmp_pixbuf,									// Destination pixbuf
		x_offset + width - 1,						// X offset
		y_offset,									// Y offset
		2,											// Width
		height,										// Height
		0, 0,										// Source offsets
		1, 1,										// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,							// Scaling type
		255);										// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);

	// Create the inner highlight box
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width - 2, height - 2);
	gdk_pixbuf_fill(highlight_pixbuf, fill_color);

	// Composite the inner highlight box onto the backing store
	gdk_pixbuf_composite(highlight_pixbuf,			// Source pixbuf
			     tmp_pixbuf,						// Destination pixbuf
			     x_offset + 1,						// X offset
			     y_offset + 1,						// Y offset
			     width - 2,							// Width
			     height - 2,						// Height
			     0, 0,								// Source offsets
			     1, 1,								// Scale factor (1 == no scale)
			     GDK_INTERP_NEAREST,				// Scaling type
			     255);								// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);
	return;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/02/04 16:46:45  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/01/19 06:35:58  vapour
 * Tweaked some error messages for clarity.
 *
 * Revision 1.4  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2008/01/13 10:33:10  vapour
 * Realigned tabs to suit my monitor.
 *
 * Revision 1.2  2007/10/04 19:51:10  vapour
 * Began adjusting the includes in each function to only call the ones it needs, in order to dramatically speed up recompilation time for developers.
 *
 * Revision 1.1  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Salasaga: Functions called when the main window is iconified or not
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


// Variables specific to this source file
gboolean	window_state_iconified = FALSE;


// Function that keeps track of whether the main window is iconified or not
gint window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	// Double check if this was triggered by a window state event
	if (GDK_WINDOW_STATE == event->type)
	{
		// Check if the iconify state changed
		if (GDK_WINDOW_STATE_ICONIFIED & event->changed_mask)
		{
			// Was the window iconified?
			if (GDK_WINDOW_STATE_ICONIFIED & event->new_window_state)
			{
				window_state_iconified = TRUE;
			} else
			{
				window_state_iconified = FALSE;
			}
		}

		// Check if the withdrawn state changed
		if (GDK_WINDOW_STATE_WITHDRAWN & event->changed_mask)
		{
			// Was the window iconified?
			if (GDK_WINDOW_STATE_ICONIFIED & event->new_window_state)
			{
				window_state_iconified = TRUE;
			} else
			{
				window_state_iconified = FALSE;
			}
		}
	}
	return FALSE;
}


// Function that returns a boolean, indicating whether the main window is iconified or not
gboolean is_window_iconified(void)
{
	// If the main window is iconified, then return TRUE, else return FALSE
	return window_state_iconified;
}

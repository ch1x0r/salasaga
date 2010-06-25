/*
 * $Id$
 *
 * Salasaga: Function that is called when the user clicks the status bar icon
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../call_back/track_window_state.h"


gint status_icon_activate(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Check if the window is iconified or not
	if (TRUE == is_window_iconified())
	{
		// Make the window visible to the user, and bring it to the front
		gtk_window_present(GTK_WINDOW(get_main_window()));

		// Display the application in the task bar
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(get_main_window()), FALSE);

	} else
	{
		// Iconify the window
		gtk_window_iconify(GTK_WINDOW(get_main_window()));

		// Hide the application from the task bar
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(get_main_window()), TRUE);
	}

	return FALSE;
}

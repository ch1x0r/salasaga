/*
 * $Id$
 *
 * Salasaga: Function that is called when the user clicks the status bar icon
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
#include "../quit_event.h"


gint status_icon_activate(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Restore the window if it's minimised
	gtk_window_deiconify(GTK_WINDOW(main_window));

	// Add the application back to the task bar
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(main_window), FALSE);

	return FALSE;
}

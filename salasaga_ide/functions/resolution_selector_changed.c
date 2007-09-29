/*
 * $Id$
 *
 * Flame Project: Function called when the user chooses a new output resolution 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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


// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	GString		*tmp_string;
	gchar		**strings;

	// Get the new output resolution
	tmp_string = g_string_new(NULL);
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(resolution_selector)));

	// Parse and store the new project output size
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	strings = g_strsplit(tmp_string->str, "x", 2);
	output_width = atoi(strings[0]);
	output_height = atoi(strings[1]);

	// Free the memory allocated in this function
	g_strfreev(strings);
	g_string_free(tmp_string, TRUE);

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

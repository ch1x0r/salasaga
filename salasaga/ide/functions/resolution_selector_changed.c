/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses a new output resolution
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	gchar		*tmp_gchar;
	gchar		**strings;
	GString		*tmp_string;

	// Get the new output resolution
	tmp_string = g_string_new(NULL);
	tmp_gchar = gtk_combo_box_get_active_text(GTK_COMBO_BOX(resolution_selector));
	g_string_printf(tmp_string, "%s", tmp_gchar);
	g_free(tmp_gchar);

	// Parse and store the new project output size
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 7);
	strings = g_strsplit(tmp_string->str, "x", 2);
	output_width = atoi(strings[0]);
	output_height = atoi(strings[1]);

	// Free the memory allocated in this function
	g_strfreev(strings);
	g_string_free(tmp_string, TRUE);

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}

/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses a new output resolution
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../preference/project_preferences.h"


gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	gchar		**main_strings;
	gchar		*tmp_gchar;
	gchar		**resolution_strings;

	// Get the new output resolution
	tmp_gchar = gtk_combo_box_get_active_text(GTK_COMBO_BOX(resolution_selector));

	// Parse and store the new project output size
	main_strings = g_strsplit(tmp_gchar, " ", 2);
	resolution_strings = g_strsplit(main_strings[0], "x", 2);
	set_output_width(atoi(resolution_strings[0]));
	set_output_height(atoi(resolution_strings[1]));

	// Free the memory allocated in this function
	g_strfreev(main_strings);
	g_strfreev(resolution_strings);
	g_free(tmp_gchar);

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}

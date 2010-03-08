/*
 * $Id$
 *
 * Salasaga: Function to create the status icon
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "status_icon_activate.h"
#include "status_icon_popup.h"


void create_status_icon()
{
	// Local variables
	GString				*status_icon_path;					// Path to the status icon image
	gulong				status_icon_signal_activate;		// Holds the signal handle for the status bar icon activation callback function
	gulong				status_icon_signal_popup;			// Holds the signal handle for the status bar icon popup callback function

	// Initialisation
	status_icon_path = g_string_new(NULL);

	// Create the status bar icon
	g_string_printf(status_icon_path, "%s%c%s", STATUS_ICON_DIR, G_DIR_SEPARATOR, "salasaga-icon.png");
	set_status_icon(gtk_status_icon_new_from_file(status_icon_path->str));
	gtk_status_icon_set_tooltip_text(get_status_icon(), _("Salasaga - Not yet capturing"));
	gtk_status_icon_set_visible(get_status_icon(), TRUE);
	g_string_free(status_icon_path, TRUE);

	// Connect the signal handlers
	status_icon_signal_activate = g_signal_connect(G_OBJECT(get_status_icon()), "activate", G_CALLBACK(status_icon_activate), (gpointer) NULL);
	status_icon_signal_popup = g_signal_connect(G_OBJECT(get_status_icon()), "popup-menu", G_CALLBACK(status_icon_popup), (gpointer) NULL);
}

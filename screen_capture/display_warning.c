/*
 * $Id$
 *
 * Salasaga: Display a warning message to the user
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


gint display_warning(gchar *warning_string)
{
	// Local variables
	GtkWidget			*tmp_dialog;				// Temporary dialog box

	// Display the warning dialog
	tmp_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", warning_string);
	gtk_dialog_run(GTK_DIALOG(tmp_dialog));
	gtk_widget_destroy(tmp_dialog);

	// Send the warning to stdout as well
	g_warning("%s", warning_string);

	return TRUE;
}

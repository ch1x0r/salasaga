/*
 * $Id$
 *
 * Flame Project: Display a warning message to the user 
 * 
 * Copyright (C) 2008 Justin Clift <justin@postgresql.org>
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


gint display_warning(gchar *warning_string)
{
	// Local variables
	GtkWidget			*tmp_dialog;				// Temporary dialog box

	// Display the warning dialog
	tmp_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, warning_string);
	gtk_dialog_run(GTK_DIALOG(tmp_dialog));
	gtk_widget_destroy(tmp_dialog);

	// Send the warning to stdout as well
	g_warning(warning_string);

	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2008/03/05 04:30:02  vapour
 * Added files required for validation of input and display of warning messages.
 *
 */

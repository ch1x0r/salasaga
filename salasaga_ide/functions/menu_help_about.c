/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Help -> About from the top menu 
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


void menu_help_about(void)
{
	// Local variables
	GtkAboutDialog		*about_dialog;
	const gchar			*authors[] = {"Justin Clift", NULL};
	GString				*tmp_gstring;
	gint				tmp_int;

	// Add the build date
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s. Built on: %s", APP_VERSION, __DATE__);

	// Create an About dialog
	about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
	gtk_about_dialog_set_name(about_dialog, APP_NAME);
	gtk_about_dialog_set_version(about_dialog, tmp_gstring->str);
	gtk_about_dialog_set_copyright(about_dialog, "(C) 2005-2007 Justin Clift <justin@postgresql.org>");
	gtk_about_dialog_set_license(about_dialog, "LGPL");
	gtk_about_dialog_set_license(about_dialog, NULL);
	gtk_about_dialog_set_authors(about_dialog, authors);

	// Display the dialog box
	tmp_int = gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy(GTK_WIDGET(about_dialog));

	// Free the resources allocated in this function
	g_string_free(tmp_gstring, TRUE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/02/04 17:05:24  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.3  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/01 11:07:33  vapour
 * Updated to show the build date here only.
 *
 * Revision 1.1  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

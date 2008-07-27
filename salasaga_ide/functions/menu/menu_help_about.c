/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Help -> About from the top menu 
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

// Salasaga includes
#include "../../config.h"
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "menu_help_website.h"


void menu_help_about(void)
{
	// Local variables
	GtkAboutDialog		*about_dialog;
	const gchar			*authors[] = {"Justin Clift <justin@salasaga.org>", NULL};
	GString				*tmp_gstring;
	gint				tmp_int;


	// Create an About dialog
	tmp_gstring = g_string_new(NULL);
	about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
	gtk_about_dialog_set_name(about_dialog, APP_NAME);
	gtk_about_dialog_set_version(about_dialog, APP_VERSION);
	g_string_printf(tmp_gstring, "%s (C) 2005-2008 Justin Clift\n<justin@salasaga.org>", _("Copyright"));
	gtk_about_dialog_set_copyright(about_dialog, tmp_gstring->str);
	gtk_about_dialog_set_license(about_dialog,
			_("This program is free software: you can redistribute it and/or modify\n"
			"it under the terms of the GNU Lesser General Public License as\n"
			"published by the Free Software Foundation, either version 3 of\n"
			"the License, or (at your option) any later version.\n"
			"\n"
			"This program is distributed in the hope that it will be useful,\n"
			"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
			"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
			"GNU Lesser General Public License for more details.\n"
			"\n"
			"You should have received a copy of the GNU Lesser General Public\n"
			"License along with this program.  If not, see\n"
			"http://www.gnu.org/licenses/."));
	gtk_about_dialog_set_authors(about_dialog, authors);
	gtk_about_dialog_set_url_hook((gpointer) menu_help_website, NULL, NULL);
	gtk_about_dialog_set_website(about_dialog, "www.salasaga.org");

	// Add the build date
	g_string_printf(tmp_gstring, _("Built on: %s"), __DATE__);
	gtk_about_dialog_set_comments(about_dialog, tmp_gstring->str);

	// Display the dialog box
	tmp_int = gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy(GTK_WIDGET(about_dialog));

	// Free the resources allocated in this function
	g_string_free(tmp_gstring, TRUE);
}

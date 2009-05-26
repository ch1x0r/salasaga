/*
 * $Id$
 *
 * Salasaga: Function that is called when the user right clicks the status bar icon
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


gint status_icon_popup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	GtkWidget			*status_icon_menu;					// Widget holding the status bar icon's menu
	GtkWidget			*status_icon_menu_item_minimise;	// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_restore;		// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_screenshot;	// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_quit;		// An item in the status bar icon menu


	// Create the right click pop-up menu for the status bar icon
	status_icon_menu = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(status_icon_menu), "Salasaga");

	// Create the menu items
	status_icon_menu_item_minimise = gtk_menu_item_new_with_label("Minimise to status bar");
	status_icon_menu_item_restore = gtk_menu_item_new_with_label("Restore");
	status_icon_menu_item_screenshot = gtk_menu_item_new_with_label("Take Screenshot");
	status_icon_menu_item_quit = gtk_menu_item_new_with_label("Quit");

	// Add the menu items to the menu
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_minimise));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_restore));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_screenshot));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_quit));

	// Ensure all of the menu items are displayed
	gtk_widget_show_all(GTK_WIDGET(status_icon_menu));

	// Connect the quit menu item to the quit signal
	g_signal_connect(G_OBJECT(status_icon_menu_item_quit), "activate", G_CALLBACK(quit_event), (gpointer) NULL);

	// Make the menu pop up
	gtk_menu_popup(GTK_MENU(status_icon_menu), NULL, NULL, gtk_status_icon_position_menu, NULL, 0, gtk_get_current_event_time() - 1000);

	return FALSE;
}

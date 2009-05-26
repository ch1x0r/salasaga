/*
 * $Id$
 *
 * Salasaga: Function to create the status icon
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


void create_status_icon()
{
	// Local variables
	GtkWidget			*status_icon_menu;					// Widget holding the status bar icon's menu
	GtkWidget			*status_icon_menu_item_display;		// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_screenshot;	// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_quit;		// An item in the status bar icon menu
	GString				*status_icon_path;					// Path to the status icon image


	// Initialise stuff
	status_icon_path = g_string_new(NULL);

	// Create the status bar icon
	g_string_printf(status_icon_path, "%s%c%s", STATUS_ICON_DIR, G_DIR_SEPARATOR, "salasaga-icon.png");
	status_icon = gtk_status_icon_new_from_file(status_icon_path->str);
	gtk_status_icon_set_tooltip_text(status_icon, "Salasaga - Not yet capturing");
	gtk_status_icon_set_visible(status_icon, TRUE);
	g_string_free(status_icon_path, TRUE);

	// Create the right click pop-up menu for the status bar icon
	status_icon_menu = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(status_icon_menu), "Salasaga");

	// Create the menu items
	status_icon_menu_item_display = gtk_menu_item_new_with_mnemonic("Display");
	status_icon_menu_item_screenshot = gtk_menu_item_new_with_mnemonic("Take Screenshot");
	status_icon_menu_item_quit = gtk_menu_item_new_with_mnemonic("Quit");

	// Add the menu items to the menu
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_display));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_screenshot));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_quit));

	// Make the menu pop up
//	gtk_menu_popup(GTK_MENU(status_icon_menu), NULL, NULL, NULL, NULL, 0, GDK_CURRENT_TIME);

	// Connect the status bar icon menu to the status bar icon
//	gtk_status_icon_position_menu(GTK_MENU(status_icon_menu), 0, 0, FALSE, status_icon);
}

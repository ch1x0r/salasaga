/*
 * $Id$
 *
 * Salasaga: Function that is called when the user right clicks the status bar icon
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
#include "../call_back/quit_event.h"
#include "../call_back/track_window_state.h"
#include "status_icon_iconify.h"
#include "status_icon_restore.h"
#include "status_icon_take_screenshot.h"


gint status_icon_popup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	GtkWidget			*status_icon_menu;					// Widget holding the status bar icon's menu
	GtkWidget			*status_icon_menu_item_iconify;		// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_screenshot;	// An item in the status bar icon menu
	GtkWidget			*status_icon_menu_item_quit;		// An item in the status bar icon menu


	// Create the right click pop-up menu for the status bar icon
	status_icon_menu = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(status_icon_menu), "Salasaga");

	// Add the Restore window/Minimise window menu item(s)
	if (TRUE == is_window_iconified())
	{
		// Create a menu option to restore the window
		status_icon_menu_item_iconify = gtk_menu_item_new_with_label(_("Restore window"));
		gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_iconify));
		g_signal_connect(G_OBJECT(status_icon_menu_item_iconify), "activate", G_CALLBACK(status_icon_restore), (gpointer) NULL);
	} else
	{
		// Create a menu option to iconify the window
		status_icon_menu_item_iconify = gtk_menu_item_new_with_label(_("Minimize window"));
		gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_iconify));
		g_signal_connect(G_OBJECT(status_icon_menu_item_iconify), "activate", G_CALLBACK(status_icon_iconify), (gpointer) NULL);
	}

	// Add the Take Screenshot menu item if screenshots are enabled
	if (TRUE == get_screenshots_enabled())
	{
		status_icon_menu_item_screenshot = gtk_menu_item_new_with_label(_("Take Screenshot"));
		gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_screenshot));
		g_signal_connect(G_OBJECT(status_icon_menu_item_screenshot), "activate", G_CALLBACK(status_icon_take_screenshot), (gpointer) NULL);
	}

	// Add the Quit menu item
	status_icon_menu_item_quit = gtk_menu_item_new_with_label(_("Quit"));
	gtk_menu_shell_append(GTK_MENU_SHELL(status_icon_menu), GTK_WIDGET(status_icon_menu_item_quit));
	g_signal_connect(G_OBJECT(status_icon_menu_item_quit), "activate", G_CALLBACK(quit_event), (gpointer) NULL);

	// Ensure all of the menu items are displayed
	gtk_widget_show_all(GTK_WIDGET(status_icon_menu));

	// Make the menu appear
	gtk_menu_popup(GTK_MENU(status_icon_menu), NULL, NULL, gtk_status_icon_position_menu, GTK_STATUS_ICON(widget), 0, gtk_get_current_event_time());

	return FALSE;
}

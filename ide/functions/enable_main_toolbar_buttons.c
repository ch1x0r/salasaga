/*
 * $Id$
 *
 * Salasaga: Enables the main toolbar buttons that can only be used when a project is loaded
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
#include "../salasaga_types.h"
#include "../externs.h"
#include "menu/menu_export_swf.h"
#include "project_adjust_dimensions.h"


void enable_main_toolbar_buttons(void)
{
	// Enable the Adjust Dimensions icon
	if (NULL != main_toolbar_icons_gray[DIMENSIONS])
	{
		g_object_ref(main_toolbar_icons_gray[DIMENSIONS]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[DIMENSIONS]), main_toolbar_icons[DIMENSIONS]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[DIMENSIONS]), main_toolbar_tooltips, _("Adjust the dimensions of the project"), "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[DIMENSIONS]));
	}

	// Enable the Export Flash icon
	if (NULL != main_toolbar_icons_gray[EXPORT_FLASH])
	{
		g_object_ref(main_toolbar_icons_gray[EXPORT_FLASH]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_FLASH]), main_toolbar_icons[EXPORT_FLASH]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, _("Export as a Flash animation"), "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_FLASH]));
	}

	// Set the event handlers for the main toolbar buttons
	if (0 == main_toolbar_signals[DIMENSIONS])
	{
		main_toolbar_signals[DIMENSIONS] = g_signal_connect(G_OBJECT(main_toolbar_items[DIMENSIONS]), "clicked", G_CALLBACK(project_adjust_dimensions), (gpointer) NULL);
		main_toolbar_signals[EXPORT_FLASH] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), "clicked", G_CALLBACK(menu_export_swf), (gpointer) NULL);
	}
}

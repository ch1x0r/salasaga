/*
 * $Id$
 *
 * Flame Project: Disables the main toolbar buttons that can only be used when a project is loaded 
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


void disable_main_toolbar_buttons(void)
{
	// Disable the Crop All icon
	if (NULL != main_toolbar_icons[CROP_ALL])
	{
		g_object_ref(main_toolbar_icons[CROP_ALL]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[CROP_ALL]), main_toolbar_icons_gray[CROP_ALL]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CROP_ALL]), main_toolbar_tooltips, "Crop disabled: No project loaded", "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[CROP_ALL]));
	}

	// Disable the Export Flash icon
	if (NULL != main_toolbar_icons[EXPORT_FLASH])
	{
		g_object_ref(main_toolbar_icons[EXPORT_FLASH]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_FLASH]), main_toolbar_icons_gray[EXPORT_FLASH]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export to Flash disabled: No project loaded", "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_FLASH]));
	}

	// Disconnect the main toolbar signal handlers
	if (0 != main_toolbar_signals[CROP_ALL])
	{
		g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[CROP_ALL]), main_toolbar_signals[CROP_ALL]);
		g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), main_toolbar_signals[EXPORT_FLASH]);

		// Flag the signal handlers as unused (used by enable_layer_toolbar_buttons())
		main_toolbar_signals[CROP_ALL] = 0;
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/02/04 16:44:45  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.3  2008/02/03 05:43:01  vapour
 * Removing svg export functionality.
 *
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

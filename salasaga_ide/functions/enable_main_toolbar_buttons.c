/*
 * $Id$
 *
 * Flame Project: Enables the main toolbar buttons that can only be used when a project is loaded 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void enable_main_toolbar_buttons(void)
{
	// Enable the Crop All icon
	if (NULL != main_toolbar_icons_gray[CROP_ALL])
	{
		g_object_ref(main_toolbar_icons_gray[CROP_ALL]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[CROP_ALL]), main_toolbar_icons[CROP_ALL]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CROP_ALL]), main_toolbar_tooltips, "Crop all slides in the project", "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[CROP_ALL]));
	}

	// Enable the Export Flash icon
	if (NULL != main_toolbar_icons_gray[EXPORT_FLASH])
	{
		g_object_ref(main_toolbar_icons_gray[EXPORT_FLASH]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_FLASH]), main_toolbar_icons[EXPORT_FLASH]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export as a Flash animation", "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_FLASH]));
	}

	// Enable the Export SVG icon
	if (NULL != main_toolbar_icons_gray[EXPORT_SVG])
	{
		g_object_ref(main_toolbar_icons_gray[EXPORT_SVG]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_SVG]), main_toolbar_icons[EXPORT_SVG]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_SVG]), main_toolbar_tooltips, "Export as an SVG animation", "Private");
		gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_SVG]));
	}

	// Set the event handlers for the main toolbar buttons
	if (0 == main_toolbar_signals[CROP_ALL])
	{
		main_toolbar_signals[CROP_ALL] = g_signal_connect(G_OBJECT(main_toolbar_items[CROP_ALL]), "clicked", G_CALLBACK(project_crop), (gpointer) NULL);
		main_toolbar_signals[EXPORT_FLASH] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), "clicked", G_CALLBACK(menu_export_flash_animation), (gpointer) NULL);
		main_toolbar_signals[EXPORT_SVG] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_SVG]), "clicked", G_CALLBACK(menu_export_svg_animation), (gpointer) NULL);
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:11  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

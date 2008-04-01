/*
 * $Id$
 *
 * Salasaga: Function to create the main application toolbar 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "menu_export_flash_animation.h"
#include "menu_file_new.h"
#include "menu_file_open.h"
#include "menu_file_save.h"
#include "menu_screenshots_capture.h"
#include "menu_screenshots_import.h"
#include "project_crop.h"
#include "quit_event.h"


GtkWidget *create_toolbar(GtkWidget *inner_toolbar)
{
	// Purpose: To create the toolbar
	//
	// Inputs: Takes a pointer to an empty GtkWidget
	//
	// Returns: A pointer to the toolbar GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *toolbar;
	//		toolbar = create_toolbar(toolbar);
	//

	// Local variables
	GdkPixbuf			*tmp_gdk_pixbuf;		// Temporary GDK Pixbuf
	GString				*tmp_gstring;			// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// Create the toolbar widget
	inner_toolbar = gtk_toolbar_new();

	// Create the tooltips structure
	main_toolbar_tooltips = gtk_tooltips_new();
	gtk_tooltips_enable(GTK_TOOLTIPS(main_toolbar_tooltips));

	// Create the New button
	main_toolbar_icons[NEW] = gtk_image_new_from_stock(GTK_STOCK_NEW, icon_height);
	main_toolbar_items[NEW] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[NEW]), "New");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[NEW]), main_toolbar_tooltips, "Begin a new project", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[NEW], NEW);
	main_toolbar_signals[NEW] = g_signal_connect(G_OBJECT(main_toolbar_items[NEW]), "clicked", G_CALLBACK(menu_file_new), (gpointer) NULL);

	// Create the Open button
	main_toolbar_icons[OPEN] = gtk_image_new_from_stock(GTK_STOCK_OPEN, icon_height);
	main_toolbar_items[OPEN] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[OPEN]), "Open");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[OPEN]), main_toolbar_tooltips, "Open an existing project", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[OPEN], OPEN);
	main_toolbar_signals[OPEN] = g_signal_connect(G_OBJECT(main_toolbar_items[OPEN]), "clicked", G_CALLBACK(menu_file_open), (gpointer) NULL);

	// Create the Save button
	main_toolbar_icons[SAVE] = gtk_image_new_from_stock(GTK_STOCK_SAVE, icon_height);
	main_toolbar_items[SAVE] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[SAVE]), "Save");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[SAVE]), main_toolbar_tooltips, "Save the project", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[SAVE], SAVE);
	main_toolbar_signals[SAVE] = g_signal_connect(G_OBJECT(main_toolbar_items[SAVE]), "clicked", G_CALLBACK(menu_file_save), (gpointer) NULL);

	// Create the Quit button
	main_toolbar_icons[QUIT] = gtk_image_new_from_stock(GTK_STOCK_QUIT, icon_height);
	main_toolbar_items[QUIT] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[QUIT]), "Quit");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[SAVE]), main_toolbar_tooltips, "Quit the application", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[QUIT], QUIT);
	main_toolbar_signals[QUIT] = g_signal_connect(G_OBJECT(main_toolbar_items[QUIT]), "clicked", G_CALLBACK(quit_event), (gpointer) NULL);

	// Add a spacer to the toolbar
	main_toolbar_items[SEPARATOR_1] = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[SEPARATOR_1], SEPARATOR_1);

	// Create the Capture button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "capture", icon_extension->str);
	if (debug_level) printf("Capture icon: '%s'\n", tmp_gstring->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons[CAPTURE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}
	main_toolbar_items[CAPTURE] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[CAPTURE]), "Capture");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CAPTURE]), main_toolbar_tooltips, "Capture screenshots", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[CAPTURE], CAPTURE);
	main_toolbar_signals[CAPTURE] = g_signal_connect(G_OBJECT(main_toolbar_items[CAPTURE]), "clicked", G_CALLBACK(menu_screenshots_capture), (gpointer) NULL);

	// Create the Import button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "import", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons[IMPORT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}
	main_toolbar_items[IMPORT] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[IMPORT]), "Import");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[IMPORT]), main_toolbar_tooltips, "Import screenshots", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[IMPORT], IMPORT);
	main_toolbar_signals[IMPORT] = g_signal_connect(G_OBJECT(main_toolbar_items[IMPORT]), "clicked", G_CALLBACK(menu_screenshots_import), (gpointer) NULL);

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons[CROP_ALL] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}
	main_toolbar_items[CROP_ALL] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[CROP_ALL]), "Crop all");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CROP_ALL]), main_toolbar_tooltips, "Crop all slides in the project", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[CROP_ALL], CROP_ALL);
	main_toolbar_signals[CROP_ALL] = g_signal_connect(G_OBJECT(main_toolbar_items[CROP_ALL]), "clicked", G_CALLBACK(project_crop), (gpointer) NULL);

	// Add a spacer to the toolbar
	main_toolbar_items[SEPARATOR_2] = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[SEPARATOR_2], SEPARATOR_2);

	// Create the Export Flash button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_flash", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons[EXPORT_FLASH] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}
	main_toolbar_items[EXPORT_FLASH] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[EXPORT_FLASH]), "Flash");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export as a Flash animation", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[EXPORT_FLASH], EXPORT_FLASH);
	main_toolbar_signals[EXPORT_FLASH] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), "clicked", G_CALLBACK(menu_export_flash_animation), (gpointer) NULL);

	// * Create the "grayed out" icons for buttons *

	// Create the grayed out Capture icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "capture_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons_gray[CAPTURE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}

	// Create the grayed out Crop All icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons_gray[CROP_ALL] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}

	// Create the grayed out Export Flash icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_flash_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		main_toolbar_icons_gray[EXPORT_FLASH] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
		gtk_object_destroy(GTK_OBJECT(tmp_gdk_pixbuf));
	}

	// Free memory allocated in this function
	g_string_free(tmp_gstring, TRUE);

	return inner_toolbar;	
}

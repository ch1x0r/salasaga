/*
 * $Id$
 *
 * Salasaga: Function to create the main application toolbar
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../call_back/quit_event.h"
#include "../menu/menu_export_swf.h"
#include "../menu/menu_file_new.h"
#include "../menu/menu_file_open.h"
#include "../menu/menu_file_save.h"
#include "../menu/menu_file_save_as.h"
#include "../menu/menu_screenshots_capture.h"
#include "../menu/menu_screenshots_import.h"
#include "../other/project_adjust_dimensions.h"
#include "../preference/application_preferences.h"
#include "shared_toolbar_functions.h"


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
	set_main_toolbar_tooltips(gtk_tooltips_new());
	gtk_tooltips_enable(GTK_TOOLTIPS(get_main_toolbar_tooltips()));

	// Create the New button
	set_main_toolbar_icon(NEW, gtk_image_new_from_stock(GTK_STOCK_NEW, get_icon_height()));
	set_main_toolbar_item(NEW, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(NEW)), _("New")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(NEW)), get_main_toolbar_tooltips(), _("Begin a new project"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(NEW), NEW);
	set_main_toolbar_signal(NEW, g_signal_connect(G_OBJECT(get_main_toolbar_item(NEW)), "clicked", G_CALLBACK(menu_file_new), (gpointer) NULL));

	// Create the Open button
	set_main_toolbar_icon(OPEN, gtk_image_new_from_stock(GTK_STOCK_OPEN, get_icon_height()));
	set_main_toolbar_item(OPEN, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(OPEN)), _("Open")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(OPEN)), get_main_toolbar_tooltips(), _("Open an existing project"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(OPEN), OPEN);
	set_main_toolbar_signal(OPEN, g_signal_connect(G_OBJECT(get_main_toolbar_item(OPEN)), "clicked", G_CALLBACK(menu_file_open), (gpointer) NULL));

	// Create the Save button
	set_main_toolbar_icon(SAVE, gtk_image_new_from_stock(GTK_STOCK_SAVE, get_icon_height()));
	set_main_toolbar_item(SAVE, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(SAVE)), _("Save")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(SAVE)), get_main_toolbar_tooltips(), _("Save the project"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(SAVE), SAVE);
	set_main_toolbar_signal(SAVE, g_signal_connect(G_OBJECT(get_main_toolbar_item(SAVE)), "clicked", G_CALLBACK(menu_file_save), (gpointer) NULL));

	// Create the Save As button
	set_main_toolbar_icon(SAVE_AS, gtk_image_new_from_stock(GTK_STOCK_SAVE_AS, get_icon_height()));
	set_main_toolbar_item(SAVE_AS, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(SAVE_AS)), _("Save As")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(SAVE_AS)), get_main_toolbar_tooltips(), _("Save the project with a different file name"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(SAVE_AS), SAVE_AS);
	set_main_toolbar_signal(SAVE_AS, g_signal_connect(G_OBJECT(get_main_toolbar_item(SAVE_AS)), "clicked", G_CALLBACK(menu_file_save_as), (gpointer) NULL));

	// Create the Quit button
	set_main_toolbar_icon(QUIT, gtk_image_new_from_stock(GTK_STOCK_QUIT, get_icon_height()));
	set_main_toolbar_item(QUIT, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(QUIT)), _("Quit")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(QUIT)), get_main_toolbar_tooltips(), _("Quit the application"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(QUIT), QUIT);
	set_main_toolbar_signal(QUIT, g_signal_connect(G_OBJECT(get_main_toolbar_item(QUIT)), "clicked", G_CALLBACK(quit_event), (gpointer) NULL));

	// Add a spacer to the toolbar
	set_main_toolbar_item(SEPARATOR_1, gtk_separator_tool_item_new());
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(SEPARATOR_1), SEPARATOR_1);

	// Create the Capture button
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "capture", get_icon_extension());
	if (get_debug_level()) printf("%s: '%s'\n", _("Path to toolbar 'Capture Screenshots' icon"), tmp_gstring->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon(CAPTURE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_main_toolbar_item(CAPTURE, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(CAPTURE)), _("Capture")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(CAPTURE)), get_main_toolbar_tooltips(), _("Capture screenshots"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(CAPTURE), CAPTURE);
	set_main_toolbar_signal(CAPTURE, g_signal_connect(G_OBJECT(get_main_toolbar_item(CAPTURE)), "clicked", G_CALLBACK(menu_screenshots_capture), (gpointer) NULL));

	// Create the Import button
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "import", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon(IMPORT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_main_toolbar_item(IMPORT, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(IMPORT)), _("Import")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(IMPORT)), get_main_toolbar_tooltips(), _("Import screenshots"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(IMPORT), IMPORT);
	set_main_toolbar_signal(IMPORT, g_signal_connect(G_OBJECT(get_main_toolbar_item(IMPORT)), "clicked", G_CALLBACK(menu_screenshots_import), (gpointer) NULL));

	// Create the Adjust Dimensions button
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "crop", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon(DIMENSIONS, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_main_toolbar_item(DIMENSIONS, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(DIMENSIONS)), _("Dimensions")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(DIMENSIONS)), get_main_toolbar_tooltips(), _("Adjust the dimensions of the project"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(DIMENSIONS), DIMENSIONS);
	set_main_toolbar_signal(DIMENSIONS, g_signal_connect(G_OBJECT(get_main_toolbar_item(DIMENSIONS)), "clicked", G_CALLBACK(project_adjust_dimensions), (gpointer) NULL));

	// Add a spacer to the toolbar
	set_main_toolbar_item(SEPARATOR_2, gtk_separator_tool_item_new());
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(SEPARATOR_2), SEPARATOR_2);

	// Create the Export Flash button
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "export_flash", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon(EXPORT_FLASH, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_main_toolbar_item(EXPORT_FLASH, gtk_tool_button_new(GTK_WIDGET(get_main_toolbar_icon(EXPORT_FLASH)), _("Flash")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(EXPORT_FLASH)), get_main_toolbar_tooltips(), _("Export as a Flash animation"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), get_main_toolbar_item(EXPORT_FLASH), EXPORT_FLASH);
	set_main_toolbar_signal(EXPORT_FLASH, g_signal_connect(G_OBJECT(get_main_toolbar_item(EXPORT_FLASH)), "clicked", G_CALLBACK(menu_export_swf), (gpointer) NULL));

	// * Create the "grayed out" icons for buttons *

	// Create the grayed out Capture icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "capture_grayed", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon_gray(CAPTURE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Dimensions icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "crop_grayed", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon_gray(DIMENSIONS, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Export Flash icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "export_flash_grayed", get_icon_extension());
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_main_toolbar_icon_gray(EXPORT_FLASH, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Free memory allocated in this function
	g_string_free(tmp_gstring, TRUE);

	return inner_toolbar;
}

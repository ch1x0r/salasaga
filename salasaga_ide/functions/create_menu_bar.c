/*
 * $Id$
 *
 * Flame Project: Function to create the top menu bar 
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
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <locale.h>

// GLib includes
#include <glib/gstdio.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void create_menu_bar()
{
	// Purpose: To create the main application menu bar
	//
	// Inputs: None
	//
	// Returns: A pointer to a Gtk menu bar Widget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *menu_bar;
	//		menu_bar = create_menu_bar(menu_bar);
	//

	// Local variables
	GtkAccelGroup			*accel_group;				// Holds the accelerator (shortcut) keys
	static GtkItemFactoryEntry	menu_items[] =
	{
		{"/_File",			NULL,			NULL,				0,	"<Branch>"},
		{"/File/_New",			"<control>N",		menu_file_new,			0,	"<Item>"},
		{"/File/_Open",			"<control>O",		menu_file_open,			0,	"<Item>"},
		{"/File/_Save",			"<control>S",		menu_file_save,			0,	"<Item>"},
		{"/File/Sep",			NULL,			NULL,				0,	"<Separator>"},
		{"/File/_Quit",			"<control>Q",		save_preferences_and_exit,	0,	"<Item>"},

		{"/_Edit",			NULL,			NULL,				0,	"<Branch>"},
		{"/Edit/_Preferences",		NULL,			menu_edit_preferences,		0,	"<Item>"},

		{"/_Screenshots",		NULL,			NULL,				0,	"<Branch>"},
		{"/Screenshots/_Capture",	NULL,			menu_screenshots_capture,	0,	"<Item>"},
		{"/Screenshots/_Import",	NULL,			menu_screenshots_import,	0,	"<Item>"},

		{"/Sl_ide",			NULL,			NULL,				0,	"<Branch>"},
		{"/Slide/_Insert",		NULL,			slide_insert,			0,	"<Item>"},
		{"/Slide/_Delete",		NULL,			slide_delete,			0,	"<Item>"},
		{"/Slide/Move _up",		NULL,			slide_move_up,			0,	"<Item>"},
		{"/Slide/Move dow_n",		NULL,			slide_move_down,		0,	"<Item>"},
		{"/Slide/Move to _top",		NULL,			slide_move_top,			0,	"<Item>"},
		{"/Slide/Move to _bottom",	NULL,			slide_move_bottom,		0,	"<Item>"},
		{"/Slide/_Edit slide name",	NULL,			slide_name_set,			0,	"<Item>"},
		{"/Slide/_Reset slide name",	NULL,			slide_name_reset,		0,	"<Item>"},

		{"/_Layer",			NULL,			NULL,				0,	"<Branch>"},
		{"/Layer/_Edit",		NULL,			layer_edit,			0,	"<Item>"},
		{"/Layer/_Delete",		NULL,			NULL,				0,	"<Item>"},
		{"/Layer/Add _Text",		NULL,			layer_new_text,			0,	"<Item>"},
		{"/Layer/Add _Highlight",	NULL,			layer_new_highlight,		0,	"<Item>"},
		{"/Layer/Add _Image",		NULL,			layer_new_image,		0,	"<Item>"},

		{"/E_xport",			NULL,			NULL,				0,	"<Branch>"},
		{"/Export/_Flash Animation",	"<control><shift>F",	menu_export_flash_animation,	0,	"<Item>"},
		{"/Export/_SVG Animation",	"<control><shift>S",	menu_export_svg_animation,	0,	"<Item>"},

		{"/_Help",			NULL,			NULL,				0,	"<LastBranch>"},
		{"/_Help/_About",		NULL,			menu_help_about,		0,	"<Item>"	},
		{"/_Help/Sep",			NULL,			NULL,				0,	"<Separator>"},
		{"/_Help/_Register",		NULL,			menu_help_register,		0,	"<Item>"	},
		{"/_Help/_Survey",		NULL,			menu_help_survey,		0,	"<Item>"	},
		{"/_Help/_Website",		NULL,			menu_help_website,		0,	"<Item>"	}
	};  // The menu structure
	static gint			num_items = sizeof(menu_items) / sizeof(menu_items[0]);	// The number of menu items


	// Create an accelerator group
	accel_group = gtk_accel_group_new();

	// Attach the accelerator group to the main window
	gtk_window_add_accel_group(GTK_WINDOW(main_window),accel_group);

	// Create the ItemFactory menu bar
	menu_bar = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

	// Create the menu items
	gtk_item_factory_create_items(menu_bar, num_items, menu_items, NULL);

	// Grey out the project menu items
	menu_enable("/Slide", FALSE);
	menu_enable("/Layer", FALSE);
	menu_enable("/Export", FALSE);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/29 02:42:25  vapour
 * Broke flame-edit.c into its component functions.
 *
 */

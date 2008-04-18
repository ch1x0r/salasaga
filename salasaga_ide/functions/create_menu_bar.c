/*
 * $Id$
 *
 * Salasaga: Function to create the top menu bar 
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
#include "layer_copy.h"
#include "layer_delete.h"
#include "layer_edit.h"
#include "layer_new_highlight.h"
#include "layer_new_image.h"
#include "layer_new_mouse.h"
#include "layer_new_text.h"
#include "layer_paste.h"
#include "menu_edit_preferences.h"
#include "menu_enable.h"
#include "menu_export_flash_animation.h"
#include "menu_export_layer.h"
#include "menu_export_slide.h"
#include "menu_file_close.h"
#include "menu_file_new.h"
#include "menu_file_open.h"
#include "menu_file_save.h"
#include "menu_file_save_as.h"
#include "menu_help_about.h"
#include "menu_help_forum.h"
#include "menu_help_register.h"
#include "menu_help_support.h"
#include "menu_help_survey.h"
#include "menu_help_website.h"
#include "menu_help_wiki.h"
#include "menu_project_properties.h"
#include "menu_screenshots_capture.h"
#include "menu_screenshots_import.h"
#include "save_preferences_and_exit.h"
#include "slide_insert.h"
#include "slide_delete.h"
#include "slide_move_bottom.h"
#include "slide_move_down.h"
#include "slide_move_top.h"
#include "slide_move_up.h"
#include "slide_name_reset.h"
#include "slide_properties.h"
#include "callbacks/menu_screenshots_capture_full_screen.h"


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
	GtkAccelGroup			*accel_group;			// Holds the accelerator (shortcut) keys
	static GtkItemFactoryEntry	menu_items[] =
	{
		{"/_File",						NULL,					NULL,							0,	"<Branch>"},
		{"/File/_New Project",			"<control>N",			menu_file_new,					0,	"<Item>"},
		{"/File/_Open Project",			"<control>O",			menu_file_open,					0,	"<Item>"},
		{"/File/_Save Project",			"<control>S",			menu_file_save,					0,	"<Item>"},
		{"/File/Save _As Project",		"<control><shift>S",	menu_file_save_as,				0,	"<Item>"},
		{"/File/Close Project",			"<control>W",			menu_file_close,				0,	"<Item>"},
		{"/File/Sep",					NULL,					NULL,							0,	"<Separator>"},
		{"/File/_Quit",					"<control>Q",			save_preferences_and_exit,		0,	"<Item>"},

		{"/_Edit",						NULL,					NULL,							0,	"<Branch>"},
		{"/Edit/_Preferences",			NULL,					menu_edit_preferences,			0,	"<Item>"},

		{"/_Screenshots",				NULL,					NULL,							0,	"<Branch>"},
		{"/Screenshots/Capture _window",		NULL,			menu_screenshots_capture,		0,	"<Item>"},
		{"/Screenshots/Capture _full screen",	NULL,			menu_screenshots_capture_full_screen,	0,	"<Item>"},
		{"/Screenshots/_Import",		NULL,					menu_screenshots_import,		0,	"<Item>"},

		{"/_Project",					NULL,					NULL,							0,	"<Branch>"},
		{"/Project/P_roperties",		NULL,					menu_project_properties,		0,	"<Item>"},

		{"/Sl_ide",						NULL,					NULL,							0,	"<Branch>"},
		{"/Slide/_Insert",				NULL,					slide_insert,					0,	"<Item>"},
		{"/Slide/_Delete",				NULL,					slide_delete,					0,	"<Item>"},
		{"/Slide/Move _up",				NULL,					slide_move_up,					0,	"<Item>"},
		{"/Slide/Move dow_n",			NULL,					slide_move_down,				0,	"<Item>"},
		{"/Slide/Move to _top",			NULL,					slide_move_top,					0,	"<Item>"},
		{"/Slide/Move to _bottom",		NULL,					slide_move_bottom,				0,	"<Item>"},
		{"/Slide/_Reset slide name",	NULL,					slide_name_reset,				0,	"<Item>"},
		{"/Slide/_Properties",			NULL,					slide_properties,				0,	"<Item>"},

		{"/_Layer",						NULL,					NULL,							0,	"<Branch>"},
//		{"/Layer/C_ut",					NULL,					layer_cut,						0,	"<Item>"},
		{"/Layer/_Copy",				NULL,					layer_copy,						0,	"<Item>"},
		{"/Layer/_Paste",				NULL,					layer_paste,					0,	"<Item>"},
		{"/Layer/_Edit",				NULL,					layer_edit,						0,	"<Item>"},
		{"/Layer/_Delete",				NULL,					layer_delete,					0,	"<Item>"},
		{"/Layer/Sep",					NULL,					NULL,							0,	"<Separator>"},
		{"/Layer/Add _Text",			NULL,					layer_new_text,					0,	"<Item>"},
		{"/Layer/Add _Highlight",		NULL,					layer_new_highlight,			0,	"<Item>"},
		{"/Layer/Add _Image",			NULL,					layer_new_image,				0,	"<Item>"},
		{"/Layer/Add _Mouse",			NULL,					layer_new_mouse,				0,	"<Item>"},

		{"/E_xport",					NULL,					NULL,							0,	"<Branch>"},
		{"/Export/_Flash Animation",	"<control><shift>F",	menu_export_flash_animation,	0,	"<Item>"},
		{"/Export/_Slide as Image",		NULL,					menu_export_slide,				0,	"<Item>"},
		{"/Export/_Image Layer",		NULL,					menu_export_layer,				0,	"<Item>"},

		{"/_Help",						NULL,					NULL,							0,	"<LastBranch>"},
		{"/_Help/_About",				NULL,					menu_help_about,				0,	"<Item>"	},
		{"/_Help/_Main Website",		NULL,					menu_help_website,				0,	"<Item>"	},
		{"/_Help/Sep",					NULL,					NULL,							0,	"<Separator>"},
//		{"/_Help/_Survey",				NULL,					menu_help_survey,				0,	"<Item>"	},
		{"/_Help/_Forum",				NULL,					menu_help_forum,				0,	"<Item>"	},
		{"/_Help/_Register",			NULL,					menu_help_register,				0,	"<Item>"	},
		{"/_Help/_Support",				NULL,					menu_help_support,				0,	"<Item>"	},
		{"/_Help/_Wiki",				NULL,					menu_help_wiki,					0,	"<Item>"	}
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

	// Grey out the menu items
	menu_enable("/Project", FALSE);
	menu_enable("/Slide", FALSE);
	menu_enable("/Layer", FALSE);
	menu_enable("/Export", FALSE);

	// If screenshots are disabled, then grey out the screenshot menu item
	if (FALSE == screenshots_enabled)
		menu_enable("/Screenshots/Capture", FALSE);
}

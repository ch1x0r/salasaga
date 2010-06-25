/*
 * $Id$
 *
 * Salasaga: Function to create the top menu bar
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
#include "../export/html/export_html_wrapper.h"
#include "../layer/image_crop.h"
#include "../layer/image_resize.h"
#include "../layer/layer_copy.h"
#include "../layer/layer_delete.h"
#include "../layer/layer_edit.h"
#include "../layer/layer_new_highlight.h"
#include "../layer/layer_new_image.h"
#include "../layer/layer_new_mouse.h"
#include "../layer/layer_new_text.h"
#include "../layer/layer_paste.h"
#include "../menu/menu_edit_preferences.h"
#include "../menu/menu_edit_redo.h"
#include "../menu/menu_edit_undo.h"
#include "../menu/menu_enable.h"
#include "../menu/menu_export_swf.h"
#include "../menu/menu_export_layer.h"
#include "../menu/menu_export_slide.h"
#include "../menu/menu_file_close.h"
#include "../menu/menu_file_new.h"
#include "../menu/menu_file_open.h"
#include "../menu/menu_file_save.h"
#include "../menu/menu_file_save_as.h"
#include "../menu/menu_help_about.h"
#include "../menu/menu_help_forum.h"
#include "../menu/menu_help_register.h"
#include "../menu/menu_help_support.h"
#include "../menu/menu_help_survey.h"
#include "../menu/menu_help_website.h"
#include "../menu/menu_help_wiki.h"
#include "../menu/menu_project_properties.h"
#include "../menu/menu_screenshots_capture.h"
#include "../menu/menu_screenshots_capture_full_screen.h"
#include "../menu/menu_screenshots_import.h"
#include "../other/project_adjust_dimensions.h"
#include "../preference/save_preferences_and_exit.h"
#include "../slide/slide_insert.h"
#include "../slide/slide_delete.h"
#include "../slide/slide_duration.h"
#include "../slide/slide_move_bottom.h"
#include "../slide/slide_move_down.h"
#include "../slide/slide_move_top.h"
#include "../slide/slide_move_up.h"


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
	GtkItemFactoryEntry	menu_items[] =
	{
		{_("/_File"),					NULL,					NULL,							0,	"<Branch>"},
		{_("/File/_New Project"),		_("<control>N"),		menu_file_new,					0,	"<Item>"},
		{_("/File/_Open Project"),		_("<control>O"),		menu_file_open,					0,	"<Item>"},
		{_("/File/_Save Project"),		_("<control>S"),		menu_file_save,					0,	"<Item>"},
		{_("/File/Save _As Project"),	_("<control><shift>S"),	menu_file_save_as,				0,	"<Item>"},
		{_("/File/_Close Project"),		_("<control>W"),		menu_file_close,				0,	"<Item>"},
		{_("/File/Sep"),				NULL,					NULL,							0,	"<Separator>"},
		{_("/File/_Quit"),				_("<control>Q"),		save_preferences_and_exit,		0,	"<Item>"},

		{_("/_Edit"),					NULL,					NULL,							0,	"<Branch>"},
		{_("/Edit/_Undo"),				_("<control>Z"),		menu_edit_undo,					0,	"<Item>"},
		{_("/Edit/_Redo"),				_("<control>Y"),		menu_edit_redo,					0,	"<Item>"},
		{_("/Edit/Sep"),				NULL,					NULL,							0,	"<Separator>"},
		{_("/Edit/_Preferences"),		NULL,					menu_edit_preferences,			0,	"<Item>"},

		{_("/_Screenshots"),			NULL,					NULL,							0,	"<Branch>"},
		{_("/Screenshots/Capture _window"),	NULL,				menu_screenshots_capture,		0,	"<Item>"},
		{_("/Screenshots/Capture _full screen"),	NULL,		menu_screenshots_capture_full_screen,	0,	"<Item>"},
		{_("/Screenshots/_Import"),		NULL,					menu_screenshots_import,		0,	"<Item>"},

		{_("/_Project"),				NULL,					NULL,							0,	"<Branch>"},
		{_("/Project/P_roperties"),		NULL,					menu_project_properties,		0,	"<Item>"},
		{_("/Project/Sep"),				NULL,					NULL,							0,	"<Separator>"},
		{_("/Project/_Adjust dimensions"),	NULL,				project_adjust_dimensions,		0,	"<Item>"},

		{_("/Sl_ide"),					NULL,					NULL,							0,	"<Branch>"},
		{_("/Slide/_Insert"),			NULL,					slide_insert,					0,	"<Item>"},
		{_("/Slide/_Delete"),			NULL,					slide_delete,					0,	"<Item>"},
		{_("/Slide/Du_ration"),			NULL,					slide_duration,					0,	"<Item>"},
		{_("/Slide/Move _up"),			NULL,					slide_move_up,					0,	"<Item>"},
		{_("/Slide/Move dow_n"),		NULL,					slide_move_down,				0,	"<Item>"},
		{_("/Slide/Move to _top"),		NULL,					slide_move_top,					0,	"<Item>"},
		{_("/Slide/Move to _bottom"),	NULL,					slide_move_bottom,				0,	"<Item>"},

		{_("/_Layer"),					NULL,					NULL,							0,	"<Branch>"},
//		{"/Layer/C_ut",					NULL,					layer_cut,						0,	"<Item>"},
		{_("/Layer/_Copy"),				NULL,					layer_copy,						0,	"<Item>"},
		{_("/Layer/_Paste"),			NULL,					layer_paste,					0,	"<Item>"},
		{_("/Layer/_Edit"),				NULL,					layer_edit,						0,	"<Item>"},
		{_("/Layer/_Delete"),			NULL,					layer_delete,					0,	"<Item>"},
		{_("/Layer/Sep"),				NULL,					NULL,							0,	"<Separator>"},
		{_("/Layer/Add _Text"),			NULL,					layer_new_text,					0,	"<Item>"},
		{_("/Layer/Add _Highlight"),	NULL,					layer_new_highlight,			0,	"<Item>"},
		{_("/Layer/Add _Image"),		NULL,					layer_new_image,				0,	"<Item>"},
		{_("/Layer/Add _Mouse"),		NULL,					layer_new_mouse,				0,	"<Item>"},
		{_("/Layer/Sep"),				NULL,					NULL,							0,	"<Separator>"},
		{_("/Layer/Image c_rop"),		NULL,					image_crop,						0,	"<Item>"},
		{_("/Layer/Image resi_ze"),		NULL,					image_resize,					0,	"<Item>"},

		{_("/E_xport"),					NULL,					NULL,							0,	"<Branch>"},
		{_("/Export/_Flash Animation"),	_("<control><shift>F"),	menu_export_swf,				0,	"<Item>"},
		{_("/Export/_Html Wrapper"),	NULL,					export_html_wrapper,			0,	"<Item>"},
		{_("/Export/_Slide as Image"),	NULL,					menu_export_slide,				0,	"<Item>"},
		{_("/Export/_Image Layer"),		NULL,					menu_export_layer,				0,	"<Item>"},

		{_("/_Help"),					NULL,					NULL,							0,	"<Branch>"},
		{_("/_Help/_About"),			NULL,					menu_help_about,				0,	"<Item>"},
		{_("/_Help/_Main Website"),		NULL,					menu_help_website,				0,	"<Item>"},
		{_("/_Help/Sep"),				NULL,					NULL,							0,	"<Separator>"},
//		{"/_Help/_Survey",				NULL,					menu_help_survey,				0,	"<Item>"},
		{_("/_Help/_Forum"),			NULL,					menu_help_forum,				0,	"<Item>"},
		{_("/_Help/_Register"),			NULL,					menu_help_register,				0,	"<Item>"},
		{_("/_Help/_Support"),			NULL,					menu_help_support,				0,	"<Item>"},
		{_("/_Help/_Wiki"),				NULL,					menu_help_wiki,					0,	"<Item>"}
	};  // The menu structure
	static gint			num_items = sizeof(menu_items) / sizeof(menu_items[0]);	// The number of menu items


	// Create an accelerator group
	accel_group = gtk_accel_group_new();

	// Attach the accelerator group to the main window
	gtk_window_add_accel_group(GTK_WINDOW(get_main_window()), accel_group);

	// Create the ItemFactory menu bar
	set_menu_bar(gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group));

	// Create the menu items
	gtk_item_factory_create_items(get_menu_bar(), num_items, menu_items, NULL);

	// Grey out the menu items
	menu_enable(_("/Project"), FALSE);
	menu_enable(_("/Slide"), FALSE);
	menu_enable(_("/Layer"), FALSE);
	menu_enable(_("/Export"), FALSE);
	menu_enable(_("/Edit/Undo"), FALSE);
	menu_enable(_("/Edit/Redo"), FALSE);

	// If screenshots are disabled, then grey out the screenshot menu items
	if (FALSE == get_screenshots_enabled())
	{
		menu_enable(_("/Screenshots/Capture _window"), FALSE);
		menu_enable(_("/Screenshots/Capture _full screen"), FALSE);
	}
}

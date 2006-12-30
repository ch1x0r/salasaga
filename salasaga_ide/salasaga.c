/*
 * $Id$
 *
 * Flame Project: Editing GUI
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"


// Global variables
GdkPixmap			*backing_store;			// Pixel buffer most drawing operations are done on
GList				*boundary_list = NULL;	// Stores a linked list of layer object boundaries
guint				capture_height;			// Height for screen captures
guint				capture_width;			// Width for screen captures
guint				capture_x;				// X offset for screen captures
guint				capture_y;				// Y offset for screen captures
GList				*current_slide = NULL;	// Pointer to the presently selected slide
gfloat				export_time_counter;	// Used when exporting, holds the number of seconds thus far
GString				*file_name = NULL;		// Holds the file name the project is saved as
GtkWidget			*film_strip;			// The film strip area
GtkScrolledWindow	*film_strip_container;	// Container for the film strip
guint				frames_per_second;		// Number of frames per second
GString				*icon_extension;		// Used to determine if SVG images can be loaded
GString				*icon_path;				// Used to determine if SVG images can be loaded
GtkWidget			*main_drawing_area;		// Widget for the drawing area
GtkWidget			*main_window;			// Widget for the main window
GtkItemFactory		*menu_bar = NULL;		// Widget for the menu bar
GtkTable			*message_bar;			// Widget for message bar
gboolean			mouse_dragging = FALSE;	// Is the mouse being dragged?
GdkPixbuf			*mouse_ptr_pixbuf;		// Temporary GDK Pixbuf
GIOChannel			*output_file;			// The output file handle
gulong				resolution_callback;	// Holds the id of the resolution selector callback
GtkComboBox			*resolution_selector;	// Widget for the resolution selector
GtkWidget			*right_side;			// Widget for the right side area
GList				*slides = NULL;			// Linked list holding the slide info
GtkWidget			*status_bar;			// Widget for the status bar
guint				statusbar_context;		// Context id for the status bar messages
gint				stored_x;				// X co-ordinate of the mouse last click
gint				stored_y;				// Y co-ordinate of the mouse last click
GtkWidget			*time_line_container;	// Scrolled window widget, to add scroll bars to the time line widget
GtkWidget			*time_line_vbox;		// VBox widget holding all of the time line elements
GtkWidget			*working;				// Widget for the working area
guint				working_width;			// Width of the display portion of the working area in pixels
guint				working_height;			// Height of the display portion of the working area in pixels
guint				zoom;					// Percentage zoom to use in the drawing area
GtkComboBox			*zoom_selector;			// Widget for the zoom selector
const gchar* const*	shared_dir_array;		// Base directory for shared files on this system

// Main toolbar items
GtkTooltips			*main_toolbar_tooltips;	// Tooltips structure
GtkWidget			*main_toolbar_icons[MAIN_TB_COUNT];  // Array of toolbar icons
GtkWidget			*main_toolbar_icons_gray[MAIN_TB_COUNT];  // Array of toolbar icons (the grayed out ones)
GtkToolItem			*main_toolbar_items[MAIN_TB_COUNT];  // Array of toolbar items
gulong				main_toolbar_signals[MAIN_TB_COUNT];  // Array of toolbar signals

// Layer toolbar items
GtkTooltips			*layer_toolbar_tooltips;	// Tooltips structure
GtkWidget			*layer_toolbar_icons[MAIN_TB_COUNT];  // Array of toolbar icons
GtkWidget			*layer_toolbar_icons_gray[MAIN_TB_COUNT];  // Array of toolbar icons (the grayed out ones)
GtkToolItem			*layer_toolbar_items[MAIN_TB_COUNT];  // Array of toolbar items
gulong				layer_toolbar_signals[MAIN_TB_COUNT];  // Array of toolbar signals

// Application default preferences
GdkColor			default_bg_colour;		// Default background color for slides
GString				*default_output_folder;	// Application default save path for exporting animations
guint				default_output_height;	// Application default for how high to create project output
guint				default_output_quality;	// Application default quality level [0-9] to save output png files with
guint				default_output_width;	// Application default for how wide to create project output
GString				*default_project_folder;// Application default save path for project folders
guint				default_slide_length;	// Default length of all new slides, in frames
guint				icon_height = 30;		// Height in pixels for the toolbar icons (they're scalable SVG's)
guint				preview_width = 300;	// Width in pixel for the film strip preview (might turn into a structure later)
guint				scaling_quality;		// Default image scaling quality used
GString				*screenshots_folder;	// Application default for where to store screenshots

// Project preferences
GString				*output_folder;			// Where to export animated SVG files too
guint				output_height;			// How high to create project output
guint				output_quality;			// The quality level [0-9] to save output png files with
guint				output_width;			// How wide to create project output
GString				*project_folder;		// The path to the project folder
guint				project_height;			// The height of the project in pixels
GString				*project_name;			// The name of the project
guint				project_width;			// The width of the project in pixels
guint				slide_length;			// Length of all new slides, in frames

// Default output resolutions
ResolutionStructure	res_array[] =
{
	{ 1600, 1200 },
	{ 1280, 1024 },
	{ 1024, 768 },
	{ 800, 600 },
	{ 640, 480 },
	{ 352, 288 },
	{ 320, 240 },
	{ 176, 144 },
	{ 160, 120 },
	{ 128, 96 }
};
gint					num_res_items = sizeof(res_array) / sizeof(res_array[0]);	// The number of resolution items


// Callback to exit the application
gint quit_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	save_preferences_and_exit();
	return FALSE;
}

// Function to create the top menu bar
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
	GtkAccelGroup		*accel_group;		// Holds the accelerator (shortcut) keys
	static GtkItemFactoryEntry	menu_items[] =
	{
		{"/_File",					NULL,					NULL,							0,	"<Branch>"},
		{"/File/_New",				"<control>N",			menu_file_new,					0,	"<Item>"},
		{"/File/_Open",				"<control>O",			menu_file_open,					0,	"<Item>"},
		{"/File/_Save",				"<control>S",			menu_file_save,					0,	"<Item>"},
		{"/File/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/File/_Quit",				"<control>Q",			save_preferences_and_exit,		0,	"<Item>"},

		{"/_Edit",					NULL,					NULL,							0,	"<Branch>"},
		{"/Edit/_Preferences",		NULL,					menu_edit_preferences,			0,	"<Item>"},

		{"/_Screenshots",			NULL,					NULL,							0,	"<Branch>"},
		{"/Screenshots/_Capture",	NULL,					menu_screenshots_capture,		0,	"<Item>"},
		{"/Screenshots/_Import",	NULL,					menu_screenshots_import,		0,	"<Item>"},

		{"/Sl_ide",					NULL,					NULL,							0,	"<Branch>"},
		{"/Slide/_Insert",			NULL,					slide_insert,					0,	"<Item>"},
		{"/Slide/De_lete",			NULL,					slide_delete,					0,	"<Item>"},
		{"/Slide/Move _up",			NULL,					slide_move_up,					0,	"<Item>"},
		{"/Slide/Move _down",		NULL,					slide_move_down,				0,	"<Item>"},
		{"/Slide/_Edit slide name",	NULL,					slide_name_set,					0,	"<Item>"},
		{"/Slide/_Reset slide name",NULL,					slide_name_reset,				0,	"<Item>"},

		{"/_Layer",					NULL,					NULL,							0,	"<Branch>"},
		{"/Layer/_Edit",			NULL,					layer_edit,						0,	"<Item>"},
		{"/Layer/_Delete",			NULL,					NULL,							0,	"<Item>"},
		{"/Layer/Add _Text",		NULL,					layer_new_text,					0,	"<Item>"},
		{"/Layer/Add _Highlight",	NULL,					layer_new_highlight,			0,	"<Item>"},
		{"/Layer/Add _Image",		NULL,					layer_new_image,				0,	"<Item>"},

		{"/E_xport",				NULL,					NULL,							0,	"<Branch>"},
		{"/Export/_Flash Animation","<control><shift>F",	menu_export_flash_animation,	0,	"<Item>"},
		{"/Export/_SVG Animation",	"<control><shift>S",	menu_export_svg_animation,		0,	"<Item>"},

		{"/_Help",					NULL,					NULL,							0,	"<LastBranch>"},
		{"/_Help/_About",			NULL,					menu_help_about,				0,	"<Item>"	},
		{"/_Help/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/_Help/_Register",		NULL,					menu_help_register,				0,	"<Item>"	},
		{"/_Help/_Survey",			NULL,					menu_help_survey,				0,	"<Item>"	},
		{"/_Help/_Website",			NULL,					menu_help_website,				0,	"<Item>"	}
	};  // The menu structure
	static gint					num_items = sizeof(menu_items) / sizeof(menu_items[0]);	// The number of menu items


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


// Function to create the main application toolbar
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
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons[CAPTURE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	main_toolbar_items[CAPTURE] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[CAPTURE]), "Capture");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CAPTURE]), main_toolbar_tooltips, "Capture screenshots", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[CAPTURE], CAPTURE);
	main_toolbar_signals[CAPTURE] = g_signal_connect(G_OBJECT(main_toolbar_items[CAPTURE]), "clicked", G_CALLBACK(menu_screenshots_capture), (gpointer) NULL);

	// Create the Import button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "import", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons[IMPORT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	main_toolbar_items[IMPORT] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[IMPORT]), "Import");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[IMPORT]), main_toolbar_tooltips, "Import screenshots", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[IMPORT], IMPORT);
	main_toolbar_signals[IMPORT] = g_signal_connect(G_OBJECT(main_toolbar_items[IMPORT]), "clicked", G_CALLBACK(menu_screenshots_import), (gpointer) NULL);

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons[CROP_ALL] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
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
	main_toolbar_icons[EXPORT_FLASH] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	main_toolbar_items[EXPORT_FLASH] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[EXPORT_FLASH]), "Flash");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export as a Flash animation", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[EXPORT_FLASH], EXPORT_FLASH);
	main_toolbar_signals[EXPORT_FLASH] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), "clicked", G_CALLBACK(menu_export_flash_animation), (gpointer) NULL);

	// Create the Export SVG button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_svg", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons[EXPORT_SVG] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	main_toolbar_items[EXPORT_SVG] = gtk_tool_button_new(GTK_WIDGET(main_toolbar_icons[EXPORT_SVG]), "SVG");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_SVG]), main_toolbar_tooltips, "Export as an SVG animation", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(inner_toolbar), main_toolbar_items[EXPORT_SVG], EXPORT_SVG);
	main_toolbar_signals[EXPORT_SVG] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_SVG]), "clicked", G_CALLBACK(menu_export_svg_animation), (gpointer) NULL);

	// * Create the "grayed out" icons for buttons *

	// Create the grayed out Crop All icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons_gray[CROP_ALL] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Export Flash icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_flash_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons_gray[EXPORT_FLASH] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Export SVG icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_svg_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	main_toolbar_icons_gray[EXPORT_SVG] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Free memory allocated in this function
	g_string_free(tmp_gstring, TRUE);

	return inner_toolbar;	
}


// Function to create the film strip area
void create_film_strip()
{
	// Purpose: To create the left film strip area
	//
	// Inputs: None
	//
	// Returns: None
	//
	// Important variables:
	//							film_strip
	//							film_strip_container
	//
	// Example:
	//
	//		GtkWidget 			*film_strip;
	//		GtkScrolledWindow	*film_strip_container;
	//
	//		create_film_strip();
	//

	// Create the film strip top widget
	film_strip_container = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(film_strip_container), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	// * Create a VBox for storing the film strip thumbnails *
	film_strip = gtk_vbox_new(FALSE, 2);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(film_strip_container), film_strip);
}


// Function to create the time line area
GtkWidget *create_time_line(void)
{
	// Purpose: To create the time line area
	//
	// Inputs: none
	//
	// Returns: A pointer to the time line GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		create_time_line();
	//

	// Local variables
	GtkWidget			*time_line_toolbar;		// Widget for holding the time line toolbar
	GtkWidget			*time_line_scrolled_window;				// Widget for holding the scrolled window

	GdkPixbuf			*tmp_gdk_pixbuf;		// Temporary GDK Pixbuf
	GString				*tmp_gstring;			// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// Create the VBox the time line elements are packed into
	time_line_vbox = gtk_vbox_new(FALSE, 0);

	// Create the scrolled window that the time line widgets are swapped into and out of
	time_line_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(time_line_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(time_line_vbox), GTK_WIDGET(time_line_scrolled_window), TRUE, TRUE, 0);
	time_line_container = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(time_line_scrolled_window), GTK_WIDGET(time_line_container));

	// Create the time line toolbar
	time_line_toolbar = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(time_line_vbox), GTK_WIDGET(time_line_toolbar), FALSE, FALSE, 0);

	// Create the tooltips structure
	layer_toolbar_tooltips = gtk_tooltips_new();
	gtk_tooltips_enable(GTK_TOOLTIPS(layer_toolbar_tooltips));

	// * Create the layer toolbar icons *

	// Create the Edit Layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "edit", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_EDIT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_EDIT] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_EDIT]), "Edit");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_tooltips, "Edit layer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_EDIT], LAYER_EDIT);
	layer_toolbar_signals[LAYER_EDIT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_EDIT]), "clicked", G_CALLBACK(layer_edit), (gpointer) NULL);

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_CROP] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_CROP] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_CROP]), "Crop");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_CROP]), layer_toolbar_tooltips, "Crop image", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_CROP], LAYER_CROP);
	layer_toolbar_signals[LAYER_CROP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_CROP]), "clicked", G_CALLBACK(image_crop), (gpointer) NULL);

	// Create the Delete layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "delete", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_DELETE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_DELETE] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_DELETE]), "Delete");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_tooltips, "Delete selected layer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_DELETE], LAYER_DELETE);
	layer_toolbar_signals[LAYER_DELETE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DELETE]), "clicked", G_CALLBACK(layer_delete), (gpointer) NULL);

	// Create the Move Layer Down button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "down_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_DOWN] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_DOWN] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_DOWN]), "Down");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_tooltips, "Move layer down", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_DOWN], LAYER_DOWN);
	layer_toolbar_signals[LAYER_DOWN] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DOWN]), "clicked", G_CALLBACK(layer_move_down), (gpointer) NULL);

	// Create the Move Layer Up button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "up_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_UP] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_UP] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_UP]), "Up");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_UP]), layer_toolbar_tooltips, "Move layer up", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_UP], LAYER_UP);
	layer_toolbar_signals[LAYER_UP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_UP]), "clicked", G_CALLBACK(layer_move_up), (gpointer) NULL);

	// Add a spacer to the toolbar
	layer_toolbar_items[LAYER_SEPARATOR_1] = gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_SEPARATOR_1], LAYER_SEPARATOR_1);

	// Create the add mouse pointer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_mouse", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_MOUSE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_MOUSE] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_MOUSE]), "Mouse");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_tooltips, "Add a mouse pointer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_MOUSE], LAYER_MOUSE);
	layer_toolbar_signals[LAYER_MOUSE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_MOUSE]), "clicked", G_CALLBACK(layer_new_mouse), (gpointer) NULL);

	// Create the add text layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_text", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_TEXT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_TEXT] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_TEXT]), "Text");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_tooltips, "Add a text layer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_TEXT], LAYER_TEXT);
	layer_toolbar_signals[LAYER_TEXT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_TEXT]), "clicked", G_CALLBACK(layer_new_text), (gpointer) NULL);

	// Create the add highlight layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_highlight", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_HIGHLIGHT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_HIGHLIGHT] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_HIGHLIGHT]), "Highlight");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_tooltips, "Add a highlight layer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_HIGHLIGHT], LAYER_HIGHLIGHT);
	layer_toolbar_signals[LAYER_HIGHLIGHT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_HIGHLIGHT]), "clicked", G_CALLBACK(layer_new_highlight), (gpointer) NULL);

	// Create the add image layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_image", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons[LAYER_IMAGE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	layer_toolbar_items[LAYER_IMAGE] = gtk_tool_button_new(GTK_WIDGET(layer_toolbar_icons[LAYER_IMAGE]), "Image");
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_tooltips, "Add an image layer", "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), layer_toolbar_items[LAYER_IMAGE], LAYER_IMAGE);
	layer_toolbar_signals[LAYER_IMAGE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_IMAGE]), "clicked", G_CALLBACK(layer_new_image), (gpointer) NULL);

	// * Create the "grayed out" icons for buttons *

	// Create the grayed out Edit Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "edit_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_EDIT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Crop Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_CROP] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Delete Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "delete_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_DELETE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Move Layer Down icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "down_arrow_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_DOWN] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Move Layer Up icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "up_arrow_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_UP] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Add Mouse Pointer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_mouse_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_MOUSE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Add Text Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_text_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_TEXT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Add Highlight Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_highlight_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_HIGHLIGHT] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Create the grayed out Add Image Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_image_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	layer_toolbar_icons_gray[LAYER_IMAGE] = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);

	// Free the memory allocated during this function
	g_string_free(tmp_gstring, TRUE);

	// Return the handle of the time line container
	return time_line_vbox;
}


// Function to create the working area
GtkWidget *create_working_area(GtkWidget *working_frame)
{
	// Purpose: To create the working area
	//
	// Inputs: Takes a pointer to an empty GtkWidget
	//
	// Returns: A pointer to the working area GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *working;
	//		working = create_working_area(working);
	//

	// Create the top working area widget
	// fixme3: How about we remove the scroll bars and use Alt+mouse to drag/move instead?
	working_frame = gtk_scrolled_window_new(NULL, NULL);

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(working_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// Create an aspect frame inside the working area's scrolled window
	working = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1, TRUE);
	gtk_frame_set_shadow_type(GTK_FRAME(working), GTK_SHADOW_NONE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(working_frame), GTK_WIDGET(working));

	// Create the drawing area inside the aspect frame
	main_drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(working), GTK_WIDGET(main_drawing_area));

	// Hook up the signal handlers to the working area
	g_signal_connect(main_drawing_area, "button_press_event", G_CALLBACK(working_area_button_press_event), NULL);
	g_signal_connect(main_drawing_area, "button_release_event", G_CALLBACK(working_area_button_release_event), NULL);
	g_signal_connect(main_drawing_area, "expose_event", G_CALLBACK(working_area_expose_event), NULL);
	g_signal_connect(main_drawing_area, "motion_notify_event", G_CALLBACK(working_area_motion_notify_event), NULL);

	// Ensure we get the signals we want
	gtk_widget_set_events(main_drawing_area, gtk_widget_get_events(main_drawing_area)
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_BUTTON1_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK);

	// Display the working area
	gtk_widget_show_all(GTK_WIDGET(working_frame));

	return working_frame;
}


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	gint				format_counter;			// Used to determine if SVG images can be loaded
	GdkPixbufFormat		*format_data;			// Used to determine if SVG images can be loaded
	GValue				*handle_size;			// The size of the handle in the main area
	GtkWidget			*main_area;				// Widget for the onscreen display
	gint				num_formats;			// Used to determine if SVG images can be loaded
	GtkWidget			*outer_box;				// Widget for the onscreen display
	GtkLabel			*resolution_label;		// Widget for the resolution selector label
	gboolean			should_maximise = FALSE;// Briefly keeps track of whether the window should be maximised
	GSList				*supported_formats;		// Used to determine if SVG images can be loaded
	GtkWidget			*toolbar = NULL;		// Widget for the toolbar
	GdkScreen			*which_screen;			// Gets given the screen the monitor is on
	gchar				wintitle[40];			// Stores the window title
	GtkLabel			*zoom_label;			// Widget for the zoom selector label

	GString				*tmp_gstring;			// Temporary GString
	GtkWidget			*tmp_widget = NULL;		// Temporary widget

#ifndef _WIN32
	// GConf related variables (not for windows)
	GString				*command_key;			// Used to work out paths into the GConf structure
	GError				*error = NULL;			// Pointer to error return structure
	gboolean			key_already_set = FALSE;// Used to work out which metacity run command is unassigned
	GConfEngine			*gconf_engine;			// GConf engine
	gchar				*gconf_value;			//
	guint				unused_num = 0;			// Used to work out which metacity run command is unassigned
	gboolean			tmp_boolean;			// Temporary boolean

	guint				tmp_guint;				// Temporary guint
	guint				tmp_int;				// Temporary guint
#endif


	// Initialise various things
	default_output_folder = g_string_new(NULL);
	default_project_folder = g_string_new(NULL);
	output_folder = g_string_new(NULL);
	project_folder = g_string_new(NULL);
	project_name = g_string_new(NULL);
	screenshots_folder = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);
	default_bg_colour.red = 0;
	default_bg_colour.green = 0;
	default_bg_colour.blue = 0;
	frames_per_second = 12;  // Half of 24 fps (film)
	icon_path = g_string_new(NULL);
	icon_extension = g_string_new("png");  // Fallback to png format if SVG isn't supported

	// Initialise GTK
	gtk_set_locale();
	gtk_init(&argc, &argv);

	// Determine the base directory for shared files on this system
	shared_dir_array = g_get_system_data_dirs();

	// Redirect log output so it doesn't pop open a console window
	g_set_print_handler(logger_simple);
	g_set_printerr_handler(logger_simple);
	g_log_set_handler(NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("Gdk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("Gtk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("GModule", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("GLib-GObject", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	g_log_set_handler("GThread", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);

	// * Work out if SVG images can be loaded *
#ifdef _WIN32
	// Hard code a different path for windows
	icon_path = g_string_assign(icon_path, "icons");
#else
	g_string_printf(icon_path, "%s%s", shared_dir_array[0], "flame/icons/72x72");
#endif

	supported_formats = gdk_pixbuf_get_formats();
	num_formats = g_slist_length(supported_formats);
	for (format_counter = 0; format_counter < num_formats; format_counter++)
	{
		format_data = g_slist_nth_data(supported_formats, format_counter);
		if (0 == g_ascii_strncasecmp(gdk_pixbuf_format_get_name(format_data), "svg", 3))
		{
			// SVG is supported
			icon_extension = g_string_assign(icon_extension, "svg");

#ifdef _WIN32
			// Hard code a different path for windows
			icon_path = g_string_assign(icon_path, "icons");
#else
			g_string_printf(icon_path, "%s%s", shared_dir_array[0], "flame/icons/scalable");
#endif

		}
	}

	// Load initial mouse pointer graphic
	g_string_printf(tmp_gstring, "%s%c%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", icon_extension->str);
	mouse_ptr_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);

	// Start up the GUI part of things
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

#ifndef _WIN32  // Non-windows check
	// Initialise sound
	gnome_sound_init(NULL);
#endif

// fixme4: Workaround for now as GConf on windows doesn't seem optimal
//         May be better to abstract this stuff into a function that switches backend transparently (GConf/Win Registry)
#ifndef _WIN32  // Non-windows check

	// Check if we have a saved configuration in GConf
	gconf_engine = gconf_engine_get_default();
	tmp_boolean = gconf_engine_dir_exists(gconf_engine, "/apps/flame", &error);
	if (TRUE == tmp_boolean)
	{
		// Load the GConf configuration
		g_string_printf(default_project_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/project_folder", NULL));
		g_string_printf(screenshots_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/screenshots_folder", NULL));
		g_string_printf(default_output_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/output_folder", NULL));
		g_string_printf(project_name, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/project_name", NULL));
		project_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_width", NULL);
		project_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_height", NULL);
		default_output_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_width", NULL);
		default_output_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_height", NULL);
		default_slide_length = slide_length = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/slide_length", NULL);
		default_output_quality = output_quality = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_quality", NULL);
		frames_per_second = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/frames_per_second", NULL);

		// Determine the stored scaling quality preference
		scaling_quality = GDK_INTERP_BILINEAR;  // A backup, just in case someone sets the GConf to an invalid value
		g_string_printf(tmp_gstring, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/scaling_quality", NULL));
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "N", 1))
		{
			// Scaling quality should be GDK_INTERP_NEAREST
			scaling_quality = GDK_INTERP_NEAREST;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "T", 1))
		{
			// Scaling quality should be GDK_INTERP_TILES
			scaling_quality = GDK_INTERP_TILES;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "B", 1))
		{
			// Scaling quality should be GDK_INTERP_BILINEAR
			scaling_quality = GDK_INTERP_BILINEAR;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "H", 1))
		{
			// Scaling quality should be GDK_INTERP_HYPER
			scaling_quality = GDK_INTERP_HYPER;
		}
		g_string_free(tmp_gstring, TRUE);

		// Check if the application should start maximised or not
		should_maximise = gconf_engine_get_bool(gconf_engine, "/apps/flame/defaults/window_maximised", NULL);
	} else
	{
#else

	// * Registry related code (windows only) *

	// Check if we have a saved configuration in the windows registry
	HKEY				hkey;
	guchar				buffer_data[1000];
	LPSTR				buffer_ptr = &buffer_data[0];
	glong				buffer_size;
	gboolean			missing_keys = FALSE;
	glong				return_code;


	// Check if the Flame Project registry keys exist
	if (!RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject", 0, KEY_QUERY_VALUE, &hkey))
	{
		// They do, so load the default values
		RegCloseKey(hkey);

		// Retrieve the value for the project folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(default_project_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "projects");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(default_project_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the screenshots folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(screenshots_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "screenshots");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "screenshots_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(screenshots_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default output folder
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(default_output_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "output");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "default_output_folder", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(default_output_folder, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the project name
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			g_string_printf(project_name, "%s", "New Project");
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_name", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				g_string_printf(project_name, "%s", buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the project width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));
			project_width = gdk_screen_get_width(which_screen);
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				project_width = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the project height
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));
			project_height = gdk_screen_get_height(which_screen);
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "project_height", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				project_height = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the output width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_output_width = 640;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "output_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_output_width = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the output height
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_output_height = 480;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "output_height", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_output_height = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default slide length
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_slide_length = slide_length = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "slide_length", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_slide_length = slide_length = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default output quality
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_output_quality = output_quality = 9;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "output_quality", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_output_quality = output_quality = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default scaling quality
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			scaling_quality =  GDK_INTERP_TILES;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "scaling_quality", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				scaling_quality = GDK_INTERP_TILES;  // A backup, just in case someone sets an invalid value
				if (0 == g_ascii_strncasecmp(buffer_ptr, "N", 1))
				{
					// Scaling quality should be GDK_INTERP_NEAREST
					scaling_quality = GDK_INTERP_NEAREST;
				}
				if (0 == g_ascii_strncasecmp(buffer_ptr, "T", 1))
				{
					// Scaling quality should be GDK_INTERP_TILES
					scaling_quality = GDK_INTERP_TILES;
				}
				if (0 == g_ascii_strncasecmp(buffer_ptr, "B", 1))
				{
					// Scaling quality should be GDK_INTERP_BILINEAR
					scaling_quality = GDK_INTERP_BILINEAR;
				}
				if (0 == g_ascii_strncasecmp(buffer_ptr, "H", 1))
				{
					// Scaling quality should be GDK_INTERP_HYPER
					scaling_quality = GDK_INTERP_HYPER;
				}
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the default slide length
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			default_slide_length = slide_length = 60;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "slide_length", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				default_slide_length = slide_length = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// Retrieve the value for the initial window maximisation
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "window_maximised", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				should_maximise = atoi(buffer_ptr);
			}

			// Close the registry key
			RegCloseKey(hkey);
		}

		// If some of the registry keys were missing then alert the user
		if (TRUE == missing_keys)
		{
			display_warning("Some of the project default registry keys are missing\n");
		}

	} else
	{
#endif  // Non-windows check

		// Which monitor are we displaying on?
		which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

		// Initialise the application variables to sensible defaults
		g_string_printf(default_project_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "projects");
		g_string_printf(screenshots_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "screenshots");
		g_string_printf(default_output_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "output");
		g_string_printf(project_name, "%s", "New Project");
		project_width = gdk_screen_get_width(which_screen);
		project_height = gdk_screen_get_height(which_screen);
		default_output_width = 640;
		default_output_height = 480;
		default_slide_length = slide_length = 60;  // Default number of frames to use in new slides
		default_output_quality = output_quality = 9;  // Default quality to save png images with
		scaling_quality =  GDK_INTERP_TILES;  // Hyper is VERY, VERY slow with large high res images [GDK_INTERP_NEAREST | GDK_INTERP_TILES | GDK_INTERP_BILINEAR | GDK_INTERP_HYPER]
	}

	// Set various required defaults that will be overwritten by the first project loaded
	g_string_printf(project_folder, "%s", default_project_folder->str);
	g_string_printf(output_folder, "%s", default_output_folder->str);

// fixme4: Workaround for now as GConf on windows doesn't seem optimal
//         May be better to abstract this stuff into a function that switches backend transparently (GConf/Windows-registry)
#ifndef _WIN32  // Non-windows check

	// * Setup the Control-Printscreen key to capture screenshots *

	// Search for the first unused run command
	command_key = g_string_new(NULL);
	for (tmp_guint = 10; tmp_guint >= 1; tmp_guint--)
	{
		// Create the name of the key to check
		g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", tmp_guint);

		// Get the value for the key
		gconf_value = gconf_engine_get_string(gconf_engine, command_key->str, NULL);

		// Check if the key is unused
		tmp_int = g_ascii_strncasecmp(gconf_value, "", 1);
		if (0 == tmp_int)
		{
			// Yes it's unused, so make a note of it
			unused_num = tmp_guint;
		} else
		{
			// This command is being used, so check if it's already assigned to flame-capture
			tmp_int = g_ascii_strncasecmp(gconf_value, "flame-capture", 13);
			if (0 == tmp_int)
			{
				key_already_set = TRUE;
			}
		}
	}

	// If an unused run command was found and we haven't already assigned a screenshot key, then assign one
	if (FALSE == key_already_set)
	{
		if (0 != unused_num)
		{
			g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", unused_num);
			gconf_engine_set_string(gconf_engine, command_key->str, "flame-capture", NULL);
			g_string_printf(command_key, "%s%u", "/apps/metacity/global_keybindings/run_command_", unused_num);
			gconf_engine_set_string(gconf_engine, command_key->str, "<Control>Print", NULL);
		}
	}
	g_string_free(command_key, TRUE);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

#endif  // Non-windows check

	// Maximise the window if our saved configuration says to
	if (TRUE == should_maximise)
	{
		// Maximise the window
		gtk_window_maximize(GTK_WINDOW(main_window));
	}

	// Set defaults values for the window capture code
	capture_height = project_height;
	capture_width = project_width;
	capture_x = 0;
	capture_y = 0;

	// Set the application title
	snprintf(wintitle, 40, "%s v%s", APP_NAME, APP_VERSION);

	// Set the window title and border
	gtk_window_set_title(GTK_WINDOW(main_window), wintitle);
	gtk_container_set_border_width(GTK_CONTAINER (main_window), 0);

	// Set a handler for destroy event to immediately exit GTK
	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(quit_event), NULL);

	// Create the outermost container that everything (menu, main, status bar) fits into
	outer_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(outer_box));

	// * Create the menu *
	create_menu_bar();
	if (FALSE == menu_bar)
	{
		// Something went wrong when creating the menu bar
		g_error("Error ED01: Something went wrong when creating the menu bar");
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(gtk_item_factory_get_widget(menu_bar, "<main>")), FALSE, FALSE, 0);

	// * Create the toolbar *
	toolbar = create_toolbar(toolbar);
	if (FALSE == toolbar)
	{
		// Something went wrong when creating the toolbar
		g_error("Error ED07: Something went wrong when creating the toolbar");
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(toolbar), FALSE, FALSE, 0);

	// * Create a HPaned to pack the film strip into, and a vbox for the time line plus working area *
	main_area = gtk_hpaned_new();
	handle_size = g_new0(GValue, 1);
	g_value_init(handle_size, G_TYPE_INT);
	gtk_widget_style_get_property(GTK_WIDGET(main_area), "handle-size", handle_size);
	gtk_paned_set_position(GTK_PANED(main_area), g_value_get_int(handle_size) + preview_width + 15);
	gtk_box_pack_start_defaults(GTK_BOX(outer_box), GTK_WIDGET(main_area));

	// * Create a table for the status bar, zoom selector, and resolution selectors to go in *
	// fixme4: Might be better going back to a HBox, and using filling (on for the status bar, off for the others), to achieve a better result
	message_bar = GTK_TABLE(gtk_table_new(1, 9, TRUE));
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(message_bar), FALSE, FALSE, 0);

	// Create the status bar
	status_bar = gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(status_bar), FALSE);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(status_bar), 0, 5, 0, 1);

	// Create the zoom selector label
	zoom_label = GTK_LABEL(gtk_label_new("Zoom: "));
	gtk_misc_set_alignment(GTK_MISC(zoom_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_label), 5, 6, 0, 1);

	// Create the zoom selector
	zoom_selector = GTK_COMBO_BOX(gtk_combo_box_new_text());

// fixme4: gtk_combo_box_set_focus_on_click function isn't present in GTK 2.4.x (shipped with Solaris 10)
#ifndef __sun
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(zoom_selector), FALSE);
#endif

	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "1600%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "800%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "400%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "300%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "200%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "150%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "100%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "75%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "50%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "25%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "10%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "Fit to width");
	gtk_combo_box_set_active(GTK_COMBO_BOX(zoom_selector), 11);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_selector), 6, 7, 0, 1);

	// Link the zoom selector to the function that recalculates the zoom and redraws the working area
	g_signal_connect(G_OBJECT(zoom_selector), "changed", G_CALLBACK(zoom_selector_changed), (gpointer) NULL);

	// Create the resolution selector label
	resolution_label = GTK_LABEL(gtk_label_new("Output: "));
	gtk_misc_set_alignment(GTK_MISC(resolution_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_label), 7, 8, 0, 1);

	// Create the resolution selector, setting 640x480 as the default
	// fixme3: The setting of default doesn't work yet.  Needs to be changed directly in the function for now
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(res_array, num_res_items, 640, 480));
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 8, 9, 0, 1);

	// Link the resolution selector to the function that stores the new values in global variables
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);

	// * Create the film strip area *
	create_film_strip();
	if (FALSE == film_strip_container)
	{
		// Something went wrong when creating the film strip
		g_error("Error ED02: Something went wrong when creating the film strip");
		exit(2);
	}
	gtk_paned_add1(GTK_PANED(main_area), GTK_WIDGET(film_strip_container));

	// * Create the vertical box to pack the time line and working area into *
	right_side = gtk_vpaned_new();
	create_time_line();  // Create the time line
	tmp_widget = create_working_area(tmp_widget);  // Create the working area

	gtk_paned_add1(GTK_PANED(right_side), GTK_WIDGET(time_line_vbox));
	gtk_paned_add2(GTK_PANED(right_side), GTK_WIDGET(tmp_widget));
	gtk_paned_set_position(GTK_PANED(right_side), 250);
	gtk_paned_add2(GTK_PANED(main_area), GTK_WIDGET(right_side));

	// Create the general global status bar context
	statusbar_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "Status bar messages");

	// Display a "Ready" message in the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Ready");

	// Calculate the zoom and drawing area, and initialise the project dimensions
	zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
	resolution_selector_changed(GTK_WIDGET(resolution_selector), NULL, (gpointer) NULL);

	// Catch when the window is resized, to automatically recalculate the zoom and redraw the drawing area
	g_signal_connect(G_OBJECT(right_side), "size-allocate", G_CALLBACK(event_size_allocate_received), (gpointer) NULL);

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Things we need:
	//  + Menu bar at the top for standard items
	//    + Have this toggleable, to enable the saving of screen real estate
	//  + Time line (top?)
	//    + Output file frame rate selector (i.e. 15 fps)
	//    + Layers
	//  + Preview/working area (main?)
	//  + Icons to change working state (top? right?)
	//  + Key bindings to make common tasks efficient
	//    + One handed keyboard operation possible (very efficient)?
	//      + Use keyboard modifiers for right mouse button, etc?
	//    + Maya/SoftImage spacebar/overlay thing?
	//  + Support wheel mouse especially in some way down the track?
	//    + Maya style, whereby shift/ctrl/alt changes scroll wheel meaning?
	//  + Status bar of some sort
	//    + Have this toggleable, to enable the saving of screen real estate
	//  + Killer feature? 3D

	// Windows we'll need
	//  + Sprite/Object window
	//  + Layers

	// Display the main window
	gtk_widget_show_all(main_window);

	// Start the main event loop
	gtk_main();

	// Free the memory use in this function
	g_string_free(icon_extension, TRUE);
	g_string_free(icon_path, TRUE);
	g_slist_free(supported_formats);

	// Exit
	exit(0);
}


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.28  2006/12/30 12:59:58  vapour
 * Updated with new path info for the shared files, which should make packaging easier.
 *
 * Revision 1.27  2006/12/26 03:56:14  vapour
 * Removing unnecessary xml includes.
 *
 * Revision 1.26  2006/07/30 04:58:50  vapour
 * Removed the ming includes, commented out the code needing ming, and added a notice to the user if they try and export flash that it's not  going to work as the code is being rewritten.
 *
 * Revision 1.25  2006/07/30 02:30:13  vapour
 * Added several new output resolutions, specifically for mobile phones.  352 x 288, 176 x 144, 160 x 120, 128 x 96.
 *
 * Revision 1.24  2006/07/09 08:05:09  vapour
 * + Centralised the test for svg image loading support.
 * + Added code to load the initial mouse pointer image at program start.
 *
 * Revision 1.23  2006/07/04 12:46:35  vapour
 * Re-enabled the mouse pointer button on the layer toolbar.
 *
 * Revision 1.22  2006/07/03 13:42:28  vapour
 * + Updated the layer toolbar creation code to use the new reduced-namespace-conflict enum definitions.
 * + Small code improvement, the handle size is dynamically read as the film strip is created rather than having a size hard coded in.
 *
 * Revision 1.21  2006/07/02 11:08:06  vapour
 * Now grays out the layer toolbar buttons at program startup.
 *
 * Revision 1.20  2006/07/02 09:24:33  vapour
 * Rewrote the code that creates the layer toolbar icons, in preparation for adding functions to disable and enable the icons.
 *
 * Revision 1.19  2006/06/28 13:42:54  vapour
 * Added further code for working with grayed out toolbar buttons.
 *
 * Revision 1.18  2006/06/27 13:41:37  vapour
 * Rewrote the main toolbar creation function so we can enable/disable individual buttons later on during program execution as needed.
 *
 * Revision 1.17  2006/06/12 03:51:27  vapour
 * Now redirects log output to our logging functions.
 *
 * Revision 1.16  2006/06/10 14:48:30  vapour
 * Removed File -> Save As menu option (not needed), and added menu items for editing the name of a slide.
 *
 * Revision 1.15  2006/05/31 14:02:03  vapour
 * Added code so the capture offsets are kept through a session.
 *
 * Revision 1.14  2006/05/28 17:36:05  vapour
 * Completed first pass of working code for loading default values stored in the windows registry.
 *
 * Revision 1.13  2006/05/28 09:35:31  vapour
 * + Moved some structures from the main function to the global context.
 * + Re-tab aligned some variables for my Linux Eclipse.
 *
 * Revision 1.12  2006/05/23 13:28:44  vapour
 * Now uses the new function for creating the zoom resolution selection from a list.
 *
 * Revision 1.11  2006/05/20 12:46:24  vapour
 * Fixed a bug in the gconf calling code that was causing a realloc crash.
 *
 * Revision 1.10  2006/05/17 11:22:14  vapour
 * Added the export_time_counter global variable.
 *
 * Revision 1.9  2006/05/15 13:37:38  vapour
 * + Sorted global definitions globally.
 * + Removed linefeed on the end of the main window title.
 * + Changed ifdefs so gnome functions aren't used on Windows.
 *
 * Revision 1.8  2006/04/29 17:02:39  vapour
 * Moved the Slide menu shortcut key to Alt-i, as it was overlapping with the Layers menu shortcut key.
 *
 * Revision 1.7  2006/04/23 11:29:09  vapour
 * Added a mouse pointer icon to the timeline toolbar.  No code behind it yet.
 *
 * Revision 1.6  2006/04/21 17:47:56  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.5  2006/04/20 12:27:00  vapour
 * Updated to find the icon files in /(basepath)/share/icons/flame/(scalable or 72x72)/.
 *
 * Revision 1.4  2006/04/18 18:00:52  vapour
 * Tweaks to allow compilation to succeed on both Windows and Solaris as well.
 * On Windows, the app will fire up as it only really required changes to not use GConf.
 * On Solaris however, a lot of stuff needed to be disabled, so it core dumps right away, prior to even displaying a window.
 * However, this *is* progress of a sort. :)
 *
 * Revision 1.3  2006/04/17 01:21:35  vapour
 * + Fixed a bug that would allow use of GTK functions before GTK initialisation (bad).
 * + Toolbar icons will fall back to the png version if loading of svg images isn't supported on the running platform (i.e. MinGW out-of-the-box).
 *
 * Revision 1.2  2006/04/16 06:03:22  vapour
 * + Removed header info copied from my local repository.
 * + Removed GDK include, as it's not needed.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

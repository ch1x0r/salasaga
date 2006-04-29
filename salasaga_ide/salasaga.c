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

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#endif

// Gnome include (for sound)
#include <libgnome/libgnome.h>

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Ming includes
#include <ming.h>

// Flame Edit includes
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"


// Global variables
GtkWidget			*main_window;			// Widget for the main window
GdkPixmap			*backing_store;			// Pixel buffer most drawing operations are done on
GtkWidget			*right_side;				// Widget for the right side area
GtkWidget			*film_strip;				// The film strip area
GtkScrolledWindow	*film_strip_container;	// Container for the film strip
GtkWidget			*time_line_container;	// Scrolled window widget, to add scroll bars to the time line widget
GtkWidget			*time_line_vbox;			// VBox widget holding all of the time line elements
GtkWidget			*working;				// Widget for the working area
GtkWidget			*main_drawing_area;		// Widget for the drawing area
GtkItemFactory		*menu_bar = NULL;		// Widget for the menu bar
GtkWidget			*status_bar;				// Widget for the status bar
GtkComboBox			*zoom_selector;			// Widget for the zoom selector
GtkComboBox			*resolution_selector;	// Widget for the resolution selector
GList				*slides = NULL;			// Linked list holding the slide info
GList				*current_slide = NULL;	// Pointer to the presently selected slide
GIOChannel			*output_file;			// The output file handle
guint				working_width;			// Width of the display portion of the working area in pixels
guint				working_height;			// Height of the display portion of the working area in pixels
GList				*boundary_list = NULL;	// Stores a linked list of layer object boundaries
gint					stored_x;				// X co-ordinate of the mouse last click
gint					stored_y;				// Y co-ordinate of the mouse last click
guint				frames_per_second;		// Number of frames per second
guint				zoom;					// Percentage zoom to use in the drawing area
gboolean				mouse_dragging = FALSE;	// Is the mouse being dragged?

// Application default preferences
GString				*default_project_folder;	// Application default save path for project folders
GString				*default_output_folder;	// Application default save path for exporting animations
guint				default_output_width;	// Application default for how wide to create project output
guint				default_output_height;	// Application default for how high to create project output
guint				default_output_quality;	// Application default quality level [0-9] to save output png files with
GString				*screenshots_folder;		// Application default for where to store screenshots
guint				preview_width = 300;		// Width in pixel for the film strip preview (might turn into a structure later)
guint				icon_height = 30;		// Height in pixels for the toolbar icons (they're scalable SVG's)
guint				default_slide_length;	// Default length of all new slides, in frames
GdkColor				default_bg_colour;		// Default background color for slides
guint				scaling_quality;			// Default image scaling quality used

// Project preferences
GString				*project_name;			// The name of the project
GString				*project_folder;			// The path to the project folder
GString				*output_folder;			// Where to export animated SVG files too
guint				output_width;			// How wide to create project output
guint				output_height;			// How high to create project output
guint				output_quality;			// The quality level [0-9] to save output png files with
guint				project_width;			// The width of the project in pixels
guint				project_height;			// The height of the project in pixels
guint				slide_length;			// Length of all new slides, in frames

// Various globals
guint				statusbar_context;		// Context id for the status bar messages


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
	GtkAccelGroup				*accel_group;		// Holds the accelerator (shortcut) keys
	static GtkItemFactoryEntry	menu_items[] =
	{
		{"/_File",					NULL,					NULL,							0,	"<Branch>"},
		{"/File/_New",				"<control>N",			menu_file_new,					0,	"<Item>"},
		{"/File/_Open",				"<control>O",			menu_file_open,					0,	"<Item>"},
		{"/File/_Save",				"<control>S",			menu_file_save,					0,	"<Item>"},
		{"/File/Save _As",			NULL,					menu_file_save_as,				0,	"<Item>"},
		{"/File/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/File/_Quit",				"<control>Q",			save_preferences_and_exit,		0,	"<Item>"},

		{"/_Edit",					NULL,					NULL,							0,	"<Branch>"},
		{"/Edit/_Preferences",		NULL,					menu_edit_preferences,			0,	"<Item>"},

		{"/_Screenshots",			NULL,					NULL,							0,	"<Branch>"},
		{"/Screenshots/_Capture",	NULL,					menu_screenshots_capture,		0,	"<Item>"},
		{"/Screenshots/_Import",		NULL,					menu_screenshots_import,			0,	"<Item>"},

		{"/Sl_ide",					NULL,					NULL,							0,	"<Branch>"},
		{"/Slide/_Insert",			NULL,					slide_insert,					0,	"<Item>"},
		{"/Slide/De_lete",			NULL,					slide_delete,					0,	"<Item>"},
		{"/Slide/Move _up",			NULL,					slide_move_up,					0,	"<Item>"},
		{"/Slide/Move _down",		NULL,					slide_move_down,					0,	"<Item>"},

		{"/_Layer",					NULL,					NULL,							0,	"<Branch>"},
		{"/Layer/_Edit",				NULL,					layer_edit,						0,	"<Item>"},
		{"/Layer/_Delete",			NULL,					NULL,							0,	"<Item>"},
		{"/Layer/Add _Text",			NULL,					layer_new_text,					0,	"<Item>"},
		{"/Layer/Add _Highlight",	NULL,					layer_new_highlight,				0,	"<Item>"},
		{"/Layer/Add _Image",		NULL,					layer_new_image,					0,	"<Item>"},

		{"/E_xport",					NULL,					NULL,							0,	"<Branch>"},
		{"/Export/_Flash Animation",	"<control><shift>F",		menu_export_flash_animation,		0,	"<Item>"},
		{"/Export/_SVG Animation",	"<control><shift>S",		menu_export_svg_animation,		0,	"<Item>"},

		{"/_Help",					NULL,					NULL,							0,	"<LastBranch>"},
		{"/_Help/_About",			NULL,					menu_help_about,					0,	"<Item>"	},
		{"/_Help/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/_Help/_Register",			NULL,					menu_help_register,				0,	"<Item>"	},
		{"/_Help/_Survey",			NULL,					menu_help_survey,				0,	"<Item>"	},
		{"/_Help/_Website",			NULL,					menu_help_website,				0,	"<Item>"	},
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
	gint				format_counter;						// Used to determine if SVG images can be loaded
	GdkPixbufFormat	*format_data;						// Used to determine if SVG images can be loaded
	GString			*icon_extension;						// Used to determine if SVG images can be loaded
	GString			*icon_path;							// Used to determine if SVG images can be loaded
	gint				num_formats;							// Used to determine if SVG images can be loaded
	GSList			*supported_formats;					// Used to determine if SVG images can be loaded

	GtkWidget		*capture_widget, *capture_button;
	GtkWidget		*crop_widget, *crop_button;
	GtkWidget		*export_flash_widget, *export_flash_button;
	GtkWidget		*export_svg_widget, *export_svg_button;
	GtkWidget		*import_widget, *import_button;
	GtkWidget		*new_widget, *new_button;
	GtkWidget		*open_widget, *open_button;
	GtkWidget		*quit_widget, *quit_button;
	GtkWidget		*save_widget, *save_button;

	GdkPixbuf		*tmp_gdk_pixbuf;						// Temporary GDK Pixbuf
	GString			*tmp_gstring;						// Temporary GString


	// Create the toolbar widget
	inner_toolbar = gtk_toolbar_new();

	// Work out if SVG images can be loaded
	icon_extension = g_string_new("png");  // Fallback to png format if SVG isn't supported
	icon_path = g_string_new("../share/icons/flame/72x72");
	supported_formats = gdk_pixbuf_get_formats();
	num_formats = g_slist_length(supported_formats);
	for (format_counter = 0; format_counter < num_formats; format_counter++)
	{
		format_data = g_slist_nth_data(supported_formats, format_counter);
		if (0 == g_ascii_strncasecmp(gdk_pixbuf_format_get_name(format_data), "svg", 3))
		{
			// SVG is supported
			g_string_assign(icon_extension, "svg");
			g_string_assign(icon_path, "../share/icons/flame/scalable");
		}
	}

	// * Create the layer toolbar icons *
	tmp_gstring = g_string_new(NULL);

	// * Create the icons with a given size *

	// Create the New button
	new_widget = gtk_image_new_from_stock(GTK_STOCK_NEW, icon_height);
	new_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"New",  // Button label
										"Begin a new project",  // Tooltip
										"Private",  // Tooltip private info
										new_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_new),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Open button
	open_widget = gtk_image_new_from_stock(GTK_STOCK_OPEN, icon_height);
	open_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Open",  // Button label
										"Open an existing project",  // Tooltip
										"Private",  // Tooltip private info
										open_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_open),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Save button
	save_widget = gtk_image_new_from_stock(GTK_STOCK_SAVE, icon_height);
	save_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Save",  // Button label
										"Save the project",  // Tooltip
										"Private",  // Tooltip private info
										save_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_save),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Quit button
	quit_widget = gtk_image_new_from_stock(GTK_STOCK_QUIT, icon_height);
	quit_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Quit",  // Button label
										"Quit the application",  // Tooltip
										"Private",  // Tooltip private info
										quit_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(quit_event),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

	// Create the Capture button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "capture", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	capture_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	capture_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Capture",  // Button label
										"Capture screenshots",  // Tooltip
										"Private",  // Tooltip private info
										capture_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_screenshots_capture),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Import button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "import", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	import_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	import_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Import",  // Button label
										"Import screenshots",  // Tooltip
										"Private",  // Tooltip private info
										import_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_screenshots_import),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	crop_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	crop_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Crop",  // Button label
										"Crop the project",  // Tooltip
										"Private",  // Tooltip private info
										crop_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(project_crop),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

	// Create the Export Flash button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_flash", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	export_flash_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	export_flash_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
												"Flash",  // Button label
												"Export as a Flash animation",  // Tooltip
												"Private",  // Tooltip private info
												export_flash_widget,  // Image/icon widget
												GTK_SIGNAL_FUNC(menu_export_flash_animation),  // Function to call when clicked
												NULL);  // Our function doesn't need any data passed to it

	// Create the Export SVG button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "export_svg", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	export_svg_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	export_svg_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
												"SVG",  // Button label
												"Export as an SVG animation",  // Tooltip
												"Private",  // Tooltip private info
												export_svg_widget,  // Image/icon widget
												GTK_SIGNAL_FUNC(menu_export_svg_animation),  // Function to call when clicked
												NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

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
	gint				format_counter;							// Used to determine if SVG images can be loaded
	GdkPixbufFormat	*format_data;							// Used to determine if SVG images can be loaded
	GString			*icon_extension;							// Used to determine if SVG images can be loaded
	GString			*icon_path;								// Used to determine if SVG images can be loaded
	gint				num_formats;								// Used to determine if SVG images can be loaded
	GSList			*supported_formats;						// Used to determine if SVG images can be loaded
	GtkWidget		*time_line_toolbar;						// Widget for holding the time line toolbar
	GtkWidget		*time_line_scrolled_window;				// Widget for holding the scrolled window

	// Toolbar variables
	GtkWidget		*crop_widget, *crop_button;				// Crop layer widgets
	GtkWidget		*delete_widget, *delete_button;			// Delete layer widgets
	GtkWidget		*edit_widget, *edit_button;				// Edit layer widgets
	GtkWidget		*highlight_widget, *highlight_button;	// Highlight layer widgets
	GtkWidget		*image_widget, *image_button;			// Highlight layer widgets
	GtkWidget		*mouse_widget, *mouse_button;			// Mouse arryw layer widgets
	GtkWidget		*move_down_widget, *move_down_button;	// Move layer down widgets
	GtkWidget		*move_up_widget, *move_up_button;		// Move layer up widgets
	GtkWidget		*text_widget, *text_button;				// Text layer widgets

	GdkPixbuf		*tmp_gdk_pixbuf;							// Temporary GDK Pixbuf
	GString			*tmp_gstring;							// Temporary GString


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

	// Work out if SVG images can be loaded
	icon_extension = g_string_new("png");  // Fallback to png format if SVG isn't supported
	icon_path = g_string_new("../share/icons/flame/72x72");
	supported_formats = gdk_pixbuf_get_formats();
	num_formats = g_slist_length(supported_formats);
	for (format_counter = 0; format_counter < num_formats; format_counter++)
	{
		format_data = g_slist_nth_data(supported_formats, format_counter);
		if (0 == g_ascii_strncasecmp(gdk_pixbuf_format_get_name(format_data), "svg", 3))
		{
			// SVG is supported
			g_string_assign(icon_extension, "svg");
			g_string_assign(icon_path, "../share/icons/flame/scalable");
		}
	}

	// * Create the layer toolbar icons *
	tmp_gstring = g_string_new(NULL);

	// Create the Edit Layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "edit", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	edit_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	edit_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Edit",  // Button label
										"Edit layer",  // Tooltip
										"Private",  // Tooltip private info
										edit_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_edit),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	crop_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	crop_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Crop",  // Button label
										"Crop image",  // Tooltip
										"Private",  // Tooltip private info
										crop_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(image_crop),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Delete layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "delete", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	delete_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	delete_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Delete",  // Button label
										"Delete selected layer",  // Tooltip
										"Private",  // Tooltip private info
										delete_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_delete),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Move Layer Down button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "down_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	move_down_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	move_down_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Down",  // Button label
										"Move layer down",  // Tooltip
										"Private",  // Tooltip private info
										move_down_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_move_down),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Move Layer Up button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "up_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	move_up_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	move_up_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Up",  // Button label
										"Move layer up",  // Tooltip
										"Private",  // Tooltip private info
										move_up_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_move_up),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(time_line_toolbar));

	// Create the add mouse pointer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_mouse", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	mouse_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	mouse_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Mouse",  // Button label
										"Add a mouse pointer",  // Tooltip
										"Private",  // Tooltip private info
										mouse_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_text),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the add text layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_text", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	text_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	text_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Text",  // Button label
										"Add a text layer",  // Tooltip
										"Private",  // Tooltip private info
										text_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_text),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the add highlight layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_highlight", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	highlight_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	highlight_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Highlight",  // Button label
										"Add a highlight layer",  // Tooltip
										"Private",  // Tooltip private info
										highlight_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_highlight),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the add image layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_image", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);
	image_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	image_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Image",  // Button label
										"Add an image layer",  // Tooltip
										"Private",  // Tooltip private info
										image_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_image),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Free the memory allocated during this function
	g_string_free(icon_extension, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_slist_free(supported_formats);

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
	GtkWidget			*main_area;				// Widget for the onscreen display
	GtkTable				*message_bar;			// Widget for message bar
	GtkWidget			*outer_box;				// Widget for the onscreen display
	GtkLabel				*resolution_label;		// Widget for the resolution selector label
	gboolean				should_maximise = FALSE;	// Briefly keeps track of whether the window should be maximised
	GtkWidget			*toolbar = NULL;			// Widget for the toolbar
	GdkScreen			*which_screen;			// Gets given the screen the monitor is on
	gchar				wintitle[40];			// Stores the window title
	GtkLabel				*zoom_label;				// Widget for the zoom selector label

	GString				*tmp_gstring;			// Temporary GString
	GtkWidget			*tmp_widget = NULL;		// Temporary widget

	// GConf related variables (not for windows)
#ifndef _WIN32
	GString				*command_key;			// Used to work out paths into the GConf structure
	GError				*error = NULL;			// Pointer to error return structure
	gboolean				key_already_set = FALSE;// Used to work out which metacity run command is unassigned
	GConfEngine			*gconf_engine;			// GConf engine
	guint				unused_num = 0;			// Used to work out which metacity run command is unassigned
	gboolean				tmp_boolean;				// Temporary boolean
	guint				tmp_guint;				// Temporary guint
	guint				tmp_int;					// Temporary guint
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

	// Initialise sound
	gnome_sound_init(NULL);

	// Start up the GUI part of things	
	gtk_init(&argc, &argv);
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

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
		default_output_width = 320;
		default_output_height = 240;
		default_slide_length = slide_length = 2;  // Default number of frames to use in new slides
		default_output_quality = output_quality = 9;  // Default quality to save png images with
		scaling_quality =  GDK_INTERP_TILES;  // Hyper is VERY, VERY slow with large high res images [GDK_INTERP_NEAREST | GDK_INTERP_TILES | GDK_INTERP_BILINEAR | GDK_INTERP_HYPER]

#ifndef _WIN32  // Non-windows check
	}
#endif  // Non-windows check

	// Set various required defaults that will be overwritten by the first project loaded
	g_string_printf(project_folder, "%s", default_project_folder->str);
	g_string_printf(output_folder, "%s", default_output_folder->str);

// fixme4: Workaround for now as GConf on windows doesn't seem optimal
//         May be better to abstract this stuff into a function that switches backend transparently (GConf/?)
#ifndef _WIN32  // Non-windows check
	// * Setup the Control-Printscreen key to capture screenshots *
	// Search for the first unused run command
	command_key = g_string_new(NULL);
	for (tmp_guint = 10; tmp_guint >= 1; tmp_guint--)
	{
		// Create the name of the key to check
		g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", tmp_guint);

		// Get the value for the key
		g_string_printf(tmp_gstring, "%s", gconf_engine_get_string(gconf_engine, command_key->str, NULL));

		// Check if the key is unused
		tmp_int = g_ascii_strncasecmp(tmp_gstring->str, "", 1);
		if (0 == tmp_int)
		{
			// Yes it's unused, so make a note of it
			unused_num = tmp_guint;
		} else
		{
			// This command is being used, so check if it's already assigned to flame-capture
			tmp_int = g_ascii_strncasecmp(tmp_gstring->str, "flame-capture", 13);
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

#ifdef _WIN32  // Windows check
	// On windows, we'll default to starting with the window maximised
	// fixme4: This is also a hack until we get a storage preferences backend working on windows
	should_maximise = TRUE;
#endif  // Windows check

	// Maximise the window if our saved configuration says to
	if (TRUE == should_maximise)
	{
		// Maximise the window
		gtk_window_maximize(GTK_WINDOW(main_window));
	}

	// Set the application title
	snprintf(wintitle, 40, "%s v%s\n", APP_NAME, APP_VERSION);

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
	gtk_paned_set_position(GTK_PANED(main_area), preview_width + 20); // fixme3: Should read the handle-size of the HPaned (or something) rather than assume 10
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

	// Create the resolution selector
	resolution_selector = GTK_COMBO_BOX(gtk_combo_box_new_text());

// fixme4: gtk_combo_box_set_focus_on_click function isn't present in GTK 2.4.x (shipped with Solaris 10)
#ifndef __sun
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(resolution_selector), FALSE);
#endif

	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1600x1200 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1280x1024 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1024x768 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "800x600 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "640x480 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "320x240 px");
	gtk_combo_box_set_active(GTK_COMBO_BOX(resolution_selector), 5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 8, 9, 0, 1);

	// Link the resolution selector to the function that stores the new values in global variables
	g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);

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

	gtk_main();

	// Exit
	exit(0);
}


/*
 * History
 * +++++++
 * 
 * $Log$
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

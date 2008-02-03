/*
 * $Id$
 *
 * Flame Project: Editing GUI
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
#include "flame-types.h"
#include "externs.h"
#include "functions/create_film_strip.h"
#include "functions/create_menu_bar.h"
#include "functions/create_resolution_selector.h"
#include "functions/create_time_line.h"
#include "functions/create_toolbar.h"
#include "functions/create_working_area.h"
#include "functions/disable_layer_toolbar_buttons.h"
#include "functions/disable_main_toolbar_buttons.h"
#include "functions/display_warning.h"
#include "functions/event_size_allocate_received.h"
#include "functions/film_strip_handle_changed.h"
#include "functions/film_strip_handle_released.h"
#include "functions/logger_simple.h"
#include "functions/logger_with_domain.h"
#include "functions/quit_event.h"
#include "functions/resolution_selector_changed.h"
#include "functions/zoom_selector_changed.h"


// Global variables
GdkPixmap				*backing_store;				// Pixel buffer most drawing operations are done on
GList					*boundary_list = NULL;		// Stores a linked list of layer object boundaries
guint					capture_height;				// Height for screen captures
guint					capture_width;				// Width for screen captures
guint					capture_x;					// X offset for screen captures
guint					capture_y;					// Y offset for screen captures
GList					*current_slide = NULL;		// Pointer to the presently selected slide
guint					debug_level = 0;			// Used to indicate debugging level
gfloat					export_time_counter;		// Used when exporting, holds the number of seconds thus far
GString					*file_name = NULL;			// Holds the file name the project is saved as
gboolean				film_strip_being_resized;	// Toggle to indicate if the film strip is being resized
GtkTreeViewColumn		*film_strip_column;			// Pointer to the film strip column
GtkScrolledWindow		*film_strip_container;		// Container for the film strip
GtkListStore			*film_strip_store;			// Film strip list store
GtkWidget				*film_strip_view;			// The view of the film strip list store
gchar					*font_path;					// Points to the base location for Flames font files
guint					frames_per_second;			// Number of frames per second
GString					*icon_extension;			// Used to determine if SVG images can be loaded
GString					*icon_path;					// Points to the base location for Flames icon files
GtkWidget				*main_drawing_area;			// Widget for the drawing area
GtkWidget				*main_window;				// Widget for the main window
GtkItemFactory			*menu_bar = NULL;			// Widget for the menu bar
GtkTable				*message_bar;				// Widget for message bar
gboolean				mouse_dragging = FALSE;		// Is the mouse being dragged?
GdkPixbuf				*mouse_ptr_pixbuf;			// Temporary GDK Pixbuf
GIOChannel				*output_file;				// The output file handle
gulong					resolution_callback;		// Holds the id of the resolution selector callback
GtkComboBox				*resolution_selector;		// Widget for the resolution selector
GtkWidget				*right_side;				// Widget for the right side area
GList					*slides = NULL;				// Linked list holding the slide info
GtkWidget				*status_bar;				// Widget for the status bar
guint					statusbar_context;			// Context id for the status bar messages
gint					stored_x;					// X co-ordinate of the mouse last click
gint					stored_y;					// Y co-ordinate of the mouse last click
GtkWidget				*time_line_container;		// Scrolled window widget, to add scroll bars to the time line widget
GtkWidget				*time_line_vbox;			// VBox widget holding all of the time line elements
guint					unscaled_button_height;		// Height of buttons in swf output control bar
guint					unscaled_button_spacing;	// Spacing between buttons of swf output control bar
guint					unscaled_button_width;		// Width of buttons in swf output control bar
GtkWidget				*working;					// Widget for the working area
guint					working_width;				// Width of the display portion of the working area in pixels
guint					working_height;				// Height of the display portion of the working area in pixels
guint					zoom;						// Percentage zoom to use in the drawing area
GtkComboBox				*zoom_selector;				// Widget for the zoom selector

// Main toolbar items
GtkTooltips				*main_toolbar_tooltips;		// Tooltips structure
GtkWidget				*main_toolbar_icons[MAIN_TB_COUNT];	// Array of toolbar icons
GtkWidget				*main_toolbar_icons_gray[MAIN_TB_COUNT];  // Array of toolbar icons (the grayed out ones)
GtkToolItem				*main_toolbar_items[MAIN_TB_COUNT];	// Array of toolbar items
gulong					main_toolbar_signals[MAIN_TB_COUNT];	// Array of toolbar signals

// Layer toolbar items
GtkTooltips				*layer_toolbar_tooltips;	// Tooltips structure
GtkWidget				*layer_toolbar_icons[MAIN_TB_COUNT];	// Array of toolbar icons
GtkWidget				*layer_toolbar_icons_gray[MAIN_TB_COUNT];   // Array of toolbar icons (the grayed out ones)
GtkToolItem				*layer_toolbar_items[MAIN_TB_COUNT];	// Array of toolbar items
gulong					layer_toolbar_signals[MAIN_TB_COUNT];	// Array of toolbar signals

// Application default preferences
GdkColor				default_bg_colour;			// Default background color for slides
GString					*default_output_folder;		// Application default save path for exporting animations
guint					default_output_height;		// Application default for how high to create project output
guint					default_output_quality;		// Application default quality level [0-9] to save output png files with
guint					default_output_width;		// Application default for how wide to create project output
GString					*default_project_folder;	// Application default save path for project folders
guint					default_slide_length;		// Default length of all new slides, in frames
guint					icon_height = 30;			// Height in pixels for the toolbar icons (they're scalable SVG's)
guint					preview_width;				// Width in pixel for the film strip preview (might turn into a structure later)
guint					scaling_quality;			// Default image scaling quality used
GString					*screenshots_folder;		// Application default for where to store screenshots

// Project preferences
GString					*output_folder;				// Where to export output files too
guint					output_height;				// How high to create project output
guint					output_quality;				// The quality level [0-9] to save output png files with
guint					output_width;				// How wide to create project output
GString					*project_folder;			// The path to the project folder
guint					project_height;				// The height of the project in pixels
GString					*project_name;				// The name of the project
guint					project_width;				// The width of the project in pixels
guint					slide_length;				// Length of all new slides, in frames

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

#ifdef _WIN32
// Windows only variables
HHOOK					win32_keyboard_hook_handle = NULL;		// Handle used to keep track of the Win32 keyboard hook
#endif


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	GString				*dot_string;
	gint				format_counter;				// Used to determine if SVG images can be loaded
	GdkPixbufFormat		*format_data;				// Used to determine if SVG images can be loaded
	GValue				*handle_size;				// The size of the handle in the main area
	GtkWidget			*main_area;					// Widget for the onscreen display
	gint				num_formats;				// Used to determine if SVG images can be loaded
	GtkWidget			*outer_box;					// Widget for the onscreen display
	GtkLabel			*resolution_label;			// Widget for the resolution selector label
	gboolean			should_maximise = FALSE;	// Briefly keeps track of whether the window should be maximised
	GSList				*supported_formats;			// Used to determine if SVG images can be loaded
	GtkWidget			*toolbar = NULL;			// Widget for the toolbar
	GdkScreen			*which_screen;				// Gets given the screen the monitor is on
	gchar				wintitle[40];				// Stores the window title
	GtkLabel			*zoom_label;				// Widget for the zoom selector label

	GString				*tmp_gstring;				// Temporary GString
	guint				tmp_int;					// Temporary guint
	GtkWidget			*tmp_widget = NULL;			// Temporary widget

#ifndef _WIN32
	// GConf related variables (not for windows)
	GString				*command_key;				// Used to work out paths into the GConf structure
	GError				*error = NULL;				// Pointer to error return structure
	gboolean			key_already_set = FALSE;		// Used to work out which metacity run command is unassigned
	GConfEngine			*gconf_engine;				// GConf engine
	gchar				*gconf_value;				//
	guint				unused_num = 0;				// Used to work out which metacity run command is unassigned
	gboolean			tmp_boolean;				// Temporary boolean

	guint				tmp_guint;					// Temporary guint
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
	preview_width = 300;
	frames_per_second = 12;  // Half of 24 fps (film)
	icon_path = g_string_new(NULL);
	icon_extension = g_string_new("png");  // Fallback to png format if SVG isn't supported
	dot_string = g_string_new(".");
	unscaled_button_height = 50;
	unscaled_button_spacing = 5;
	unscaled_button_width = 50;

	// Initialise the button event handlers on the toolbars to NULL
	main_toolbar_signals[CROP_ALL] = 0;
	main_toolbar_signals[EXPORT_FLASH] = 0;
	for (tmp_int = 0; tmp_int < LAYER_TB_COUNT; tmp_int++)
	{
	    layer_toolbar_signals[tmp_int] = 0;
	}

	// Set the locale to C for now, so the output of numbers is correct
	if ((NULL == setlocale(LC_ALL, "")) | (NULL == setlocale(LC_NUMERIC, "C")))
	{
		printf("Locale was unable to be set.\n");
	}

	// Initialise GTK
	gtk_disable_setlocale();
	gtk_init(&argc, &argv);

	// Redirect log output so it doesn't pop open a console window
	if ((2 == argc) && (0 == g_ascii_strncasecmp("-d", argv[1], 2)))
	{
		// Disable logging
		debug_level = 1;
		printf("Debugging mode, errors will be shown on stdout.\n");
	} else
	{
		g_set_print_handler(logger_simple);
		g_set_printerr_handler(logger_simple);
		g_log_set_handler(NULL, G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("Gdk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("Gtk", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("GModule", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("GLib-GObject", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
		g_log_set_handler("GThread", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION, logger_with_domain, NULL);
	}

	if (debug_level)
	{
		printf("Program path: '%s'\n", argv[0]);
		printf("Directory base: '%s'\n", g_path_get_dirname(argv[0]));
		g_string_printf(icon_path, g_path_get_dirname(argv[0]));
		g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "..", "share", NULL));
		printf("Location for icons: '%s'\n", icon_path->str);
	}

	// * Work out if SVG images can be loaded *
#ifdef _WIN32
	// Hard code a different path for windows
	icon_path = g_string_assign(icon_path, "icons");
#else
	g_string_assign(icon_path, g_path_get_dirname(argv[0]));
	g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "..", "share", "flame", "icons", "72x72", NULL));

	// Display debugging info if requested
	if (debug_level)
		printf("Icon path being checked: %s\n", icon_path->str);

	// Check if the above directory exists
	if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
	{
		// First guess of icon directory didn't work, lets try /usr/share/flame/icons next
		g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, "/", "usr", "share", "flame", "icons", "72x72", NULL));

		// Display debugging info if requested
		if (debug_level)
			printf("Icon path being checked: %s\n", icon_path->str);

		if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
		{
			// Not there, try /usr/local/share/flame/icons
			g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, "/", "usr", "local", "share", "flame", "icons", "72x72", NULL));

			// Display debugging info if requested
			if (debug_level)
				printf("Icon path being checked: %s\n", icon_path->str);

			if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
			{
				// Unable to determine the location for icons, so generate an error then exit
				display_warning("Error ED112: Unable to find the icons directory. Exiting.");
				exit(1);				
			}
		}
	}
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
			g_string_assign(icon_path, g_path_get_dirname(argv[0]));
			g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "..", "share", "flame", "icons", "scalable", NULL));

			// Display debugging info if requested
			if (debug_level)
				printf("Icon path being checked: %s\n", icon_path->str);

			// Check if the above directory exists
			if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
			{
				// First guess of icon directory didn't work, lets try /usr/share/flame/icons next
				g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, "/", "usr", "share", "flame", "icons", "scalable", NULL));

				// Display debugging info if requested
				if (debug_level)
					printf("Icon path being checked: %s\n", icon_path->str);

				if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
				{
					// Not there, try /usr/local/share/flame/icons
					g_string_printf(icon_path, g_build_path(G_DIR_SEPARATOR_S, "/", "usr", "local", "share", "flame", "icons", "scalable", NULL));

					// Display debugging info if requested
					if (debug_level)
						printf("Icon path being checked: %s\n", icon_path->str);

					if (TRUE != g_file_test(icon_path->str, G_FILE_TEST_IS_DIR))
					{
						// Unable to determine the location for icons, so generate an error then exit
						display_warning("Error ED113: Unable to find the icons directory. Exiting.");
						exit(1);				
					}
				}
			}
#endif

		}
	}
	g_string_free(dot_string, TRUE);

	// Load initial mouse pointer graphic
	g_string_printf(tmp_gstring, "%s%c%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", icon_extension->str);
	mouse_ptr_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, icon_height, NULL);

	// Create the path to the font files
#ifdef _WIN32
	// Hard code a different path for windows
	font_path = g_build_path(G_DIR_SEPARATOR_S, "fonts", "BitstreamVera", NULL);
#else
	font_path = g_build_path(G_DIR_SEPARATOR_S, icon_path->str, "..", "..", "fonts", "BitstreamVera", NULL);
#endif

	// Start up the GUI part of things
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
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
		preview_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/thumbnail_width", NULL);
		if (0 == preview_width) preview_width = 300;
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

		// Retrieve the value for the default thumbnail width
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\FlameProject\\defaults", 0, KEY_QUERY_VALUE, &hkey))
		{
			// Value is missing, so warn the user and set a sensible default
			missing_keys = TRUE;
			preview_width = 300;
		} else
		{
			// Retrieve the value
			buffer_size = sizeof(buffer_data);
			return_code = RegQueryValueExA(hkey, "thumbnail_width", NULL, NULL, buffer_ptr, &buffer_size);
			if (ERROR_SUCCESS == return_code)
			{
				preview_width = atoi(buffer_ptr);
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
#endif  // End of windows code

		// Looks like Flame hasn't been run before, so create it's home directory structure
		g_string_printf(tmp_gstring, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "projects");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "screenshots");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "output");
		g_mkdir(tmp_gstring->str, 0750);

		// Which monitor are we displaying on?
		which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

		// Initialise the application variables to sensible defaults
		g_string_printf(default_project_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "projects");
		g_string_printf(screenshots_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "screenshots");
		g_string_printf(default_output_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "flame", G_DIR_SEPARATOR, "output");
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

	// * Setup the Control-Printscreen key combination to capture screenshots *
#ifndef _WIN32
	// Non-windows code

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

#else
	// Windows code
	


#endif // End of windows code

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
		display_warning("Error ED01: Something went wrong when creating the menu bar");
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(gtk_item_factory_get_widget(menu_bar, "<main>")), FALSE, FALSE, 0);

	// * Create the toolbar *
	toolbar = create_toolbar(toolbar);
	if (FALSE == toolbar)
	{
		// Something went wrong when creating the toolbar
		display_warning("Error ED07: Something went wrong when creating the toolbar");
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

	// Attach signal handlers to the movable handle between film strip and right hand side
	film_strip_being_resized = FALSE;
	g_signal_connect(G_OBJECT(main_area), "notify::position", G_CALLBACK(film_strip_handle_changed), (gpointer) NULL);
	g_signal_connect(G_OBJECT(main_area), "button_release_event", G_CALLBACK(film_strip_handle_released), (gpointer) NULL);

	// * Create a table for the status bar, zoom selector, and resolution selectors to go in *
	// fixme4: Might be better going back to a HBox, and using filling (on for the status bar, off for the others), to achieve a better result
	message_bar = GTK_TABLE(gtk_table_new(1, 6, TRUE));
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(message_bar), FALSE, FALSE, 0);

	// Create the status bar
	status_bar = gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(status_bar), FALSE);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(status_bar), 0, 2, 0, 1);

	// Create the zoom selector label
	zoom_label = GTK_LABEL(gtk_label_new("Zoom: "));
	gtk_misc_set_alignment(GTK_MISC(zoom_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_label), 2, 3, 0, 1);

	// Create the zoom selector
	zoom_selector = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(zoom_selector), FALSE);
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
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_selector), 3, 4, 0, 1);

	// Link the zoom selector to the function that recalculates the zoom and redraws the working area
	g_signal_connect(G_OBJECT(zoom_selector), "changed", G_CALLBACK(zoom_selector_changed), (gpointer) NULL);

	// Create the resolution selector label
	resolution_label = GTK_LABEL(gtk_label_new("Output: "));
	gtk_misc_set_alignment(GTK_MISC(resolution_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_label), 4, 5, 0, 1);

	// Create the resolution selector, setting 640x480 as the default
	// fixme3: The setting of default doesn't work yet.  Needs to be changed directly in the function for now
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(res_array, num_res_items, 640, 480));
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 5, 6, 0, 1);

	// Link the resolution selector to the function that stores the new values in global variables
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);

	// * Create the film strip area *
	create_film_strip();
	if (FALSE == film_strip_container)
	{
		// Something went wrong when creating the film strip
		display_warning("Error ED02: Something went wrong when creating the film strip");
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
 * Revision 1.73  2008/02/03 05:46:11  vapour
 * Removing svg export functionality.
 *
 * Revision 1.72  2008/02/03 02:12:15  vapour
 * Added more debugging output code.
 *
 * Revision 1.71  2008/02/02 04:06:42  vapour
 * Updated the code that looks for the icon directory, so it now actively checks for things and then complains if nothing obvious works.
 *
 * Revision 1.70  2008/01/31 07:09:52  vapour
 * Shrunk the size of the buttons in the swf output control bar by about 40%.
 *
 * Revision 1.69  2008/01/31 01:05:50  vapour
 * Converted the swf control bar defines into variables, so they can be adjusted by the user in future.
 *
 * Revision 1.68  2008/01/19 07:13:38  vapour
 *  + Improved the directory separator used in strings, for better cross platform functionality.
 *  + Added the font_path variable and code to set it at program start up.
 *
 * Revision 1.67  2008/01/15 16:20:32  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.66  2008/01/13 06:45:01  vapour
 * Realigned tabs to line up on my display again.  This is annoying.
 *
 * Revision 1.65  2007/10/04 19:51:12  vapour
 * Began adjusting the includes in each function to only call the ones it needs, in order to dramatically speed up recompilation time for developers.
 *
 * Revision 1.64  2007/09/29 04:22:18  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.63  2007/09/29 02:42:28  vapour
 * Broke flame-edit.c into its component functions.
 *
 * Revision 1.62  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.61  2007/09/27 10:40:56  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 * Revision 1.60  2007/09/19 13:31:49  vapour
 * Adding initial working code to set a keyboard hook for the Control Printscreen key through the flame-keycapture dll.
 *
 * Revision 1.59  2007/09/18 02:53:42  vapour
 * Updated copyright year to 2007.
 *
 * Revision 1.58  2007/09/11 13:03:55  vapour
 * Moved declaration of tmp_int variable, so compilation works properly on windows.
 *
 * Revision 1.57  2007/07/28 16:25:44  vapour
 * Small code cleanup.
 *
 * Revision 1.56  2007/07/28 15:06:50  vapour
 * Added code to check for successful creation of icon images, to stop a bunch of warning messages that occur when debugging.
 *
 * Revision 1.55  2007/07/23 14:23:28  vapour
 * Added SVG decimal separator fix that has been verified to work.
 *
 * Revision 1.54  2007/07/18 13:31:59  vapour
 * Initial code to explicitly set the locale for numbers at program start time.
 *
 * Revision 1.53  2007/07/15 06:45:42  vapour
 * Oops, previous commit introduced a bug, now fixed.
 *
 * Revision 1.52  2007/07/15 06:36:06  vapour
 * If the absolute path to flame-edit can't be worked out, now defaults to /usr/bin.
 *
 * Revision 1.51  2007/07/09 13:18:43  vapour
 * Added a default of 300 pixels for the thumbnail width in those cases where people have run an older version of flame-edit before.
 *
 * Revision 1.50  2007/07/09 12:42:53  vapour
 * Updated the film strip column resizing to work nicer.
 *
 * Revision 1.49  2007/07/09 12:24:52  vapour
 * Added a signal handler and supporting global variable for the new film strip resizing functions.
 *
 * Revision 1.48  2007/07/09 11:17:52  vapour
 * Film strip column will look ok when resizing now.
 *
 * Revision 1.47  2007/07/09 10:35:52  vapour
 * Icon path is now calculated dynamically.
 *
 * Revision 1.46  2007/07/09 09:58:22  vapour
 * Removed unused variables.
 *
 * Revision 1.45  2007/07/08 14:07:17  vapour
 * Hooked up a signal handler for the new film_strip_handle_changed function.
 *
 * Revision 1.44  2007/07/08 13:19:53  vapour
 * Thumbnail width is now kept between sessions.
 *
 * Revision 1.43  2007/07/07 12:43:43  vapour
 * GUI window now starts up at 800x600 size by default, and doesn't expand too far to the right at initial GUI creation.
 *
 * Revision 1.42  2007/07/03 14:48:55  vapour
 * Updated the film strip widget to no longer be inside a viewport, as it was doing bad things to the scrolling.
 *
 * Revision 1.41  2007/07/03 14:27:38  vapour
 * Explicitly set the film strip area selection policy to single selection only.
 * Also introduced a new debug_level global variable.
 *
 * Revision 1.40  2007/07/02 10:40:36  vapour
 * Added a -d switch to the command line options, which enables errors to be shown on the command line.  Useful for debugging.
 *
 * Revision 1.39  2007/07/01 12:46:50  vapour
 * Removed the film_strip global variable, and removed an uneeded VBox out of the film strip structure.
 *
 * Revision 1.38  2007/06/30 10:03:35  vapour
 * Began writing code to move the thumbnails around in the film strip.
 *
 * Revision 1.37  2007/06/30 07:58:10  vapour
 * Lined up the menu bar definition columns prior to making changes.
 *
 * Revision 1.36  2007/06/30 06:04:19  vapour
 * The timeline and workspace area are now updated when a slide is selected in the film strip.  All done with the GtkTreeView approach now.
 *
 * Revision 1.35  2007/06/30 03:18:18  vapour
 * Began re-writing the film strip area to use a GtkListView widget instead of the hodge podge of event boxes, signal handlers, and other bits.
 *
 * Revision 1.34  2007/06/25 08:50:07  vapour
 * Updated pointer types of new code to not give errors with gcc.
 *
 * Revision 1.33  2007/06/25 05:49:01  vapour
 * Fixed the bug whereby toolbar dialogs would be called multiple times after the importing of slides.
 *
 * Revision 1.32  2007/06/25 04:55:55  vapour
 * Neatened up the columns in the main function, prior to fixing the bug with layer dialogs being called multiple times.
 *
 * Revision 1.31  2007/06/24 11:33:48  vapour
 * Removed the __sun workarounds for Solaris 10, as Solaris 11 should have the missing bits.
 *
 * Revision 1.30  2007/01/05 08:31:36  vapour
 * Added code to create flame home directory structure the first time it's run.
 *
 * Revision 1.29  2007/01/05 06:46:47  vapour
 * Hard coded to expect icons and similar in /usr/share/.
 *
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

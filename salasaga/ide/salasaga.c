/*
 * $Id$
 *
 * Salasaga: Main source file for the Salasaga IDE
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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


// Standard includes
#include <unistd.h>

// GLib include
#include <glib/gstdio.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// libintl.h is required here
	#include <libintl.h>

	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Ming include
#include <ming.h>

// Salasaga includes
#include "../config.h"
#include "salasaga_types.h"
#include "externs.h"
#include "functions/call_back/quit_event.h"
#include "functions/call_back/track_window_state.h"
#include "functions/dialog/display_warning.h"
#include "functions/film_strip/create_film_strip.h"
#include "functions/film_strip/film_strip_handle_changed.h"
#include "functions/film_strip/film_strip_handle_released.h"
#include "functions/menu/create_menu_bar.h"
#include "functions/other/key_bind.h"
#include "functions/other/load_fonts.h"
#include "functions/other/logger_simple.h"
#include "functions/other/logger_with_domain.h"
#include "functions/preference/application_preferences.h"
#include "functions/preference/preferences_load.h"
#include "functions/preference/project_preferences.h"
#include "functions/resolution_selector/create_resolution_selector.h"
#include "functions/resolution_selector/resolution_selector_changed.h"
#include "functions/status_icon/create_status_icon.h"
#include "functions/text_tags/create_text_tags.h"
#include "functions/time_line/create_time_line.h"
#include "functions/tool_bars/create_toolbar.h"
#include "functions/tool_bars/disable_layer_toolbar_buttons.h"
#include "functions/tool_bars/disable_main_toolbar_buttons.h"
#include "functions/tool_bars/shared_toolbar_functions.h"
#include "functions/working_area/create_working_area.h"
#include "functions/working_area/event_size_allocate_received.h"
#include "functions/zoom_selector/create_zoom_selector.h"
#include "functions/zoom_selector/zoom_selector_changed.h"


// Global variables
GList					*current_slide = NULL;		// Pointer to the presently selected slide
GList					*slides = NULL;				// Linked list holding the slide info

#ifdef _WIN32
// Windows only global variable
HHOOK					win32_keyboard_hook_handle = NULL;		// Handle used to keep track of the Win32 keyboard hook
#endif

// Fonts available for use in text layers
gchar	*salasaga_font_names[] =
{
	"DejaVu Sans",							// FONT_DEJAVU_SANS
	"DejaVu Sans Bold",						// FONT_DEJAVU_SANS_B
	"DejaVu Sans Bold Oblique",				// FONT_DEJAVU_SANS_B_O
	"DejaVu Sans Condensed",				// FONT_DEJAVU_SANS_C
	"DejaVu Sans Condensed Bold",			// FONT_DEJAVU_SANS_C_B
	"DejaVu Sans Condensed Bold Oblique",	// FONT_DEJAVU_SANS_C_B_O
	"DejaVu Sans Condensed Oblique",		// FONT_DEJAVU_SANS_C_O
	"DejaVu Sans Extra Light",				// FONT_DEJAVU_SANS_EL
	"DejaVu Sans Mono",						// FONT_DEJAVU_SANS_MONO
	"DejaVu Sans Mono Bold",				// FONT_DEJAVU_SANS_MONO_B
	"DejaVu Sans Mono Bold Oblique",		// FONT_DEJAVU_SANS_MONO_B_O
	"DejaVu Sans Mono Oblique",				// FONT_DEJAVU_SANS_MONO_O
	"DejaVu Sans Oblique",					// FONT_DEJAVU_SANS_O
	"DejaVu Serif",							// FONT_DEJAVU_SERIF
	"DejaVu Serif Bold",					// FONT_DEJAVU_SERIF_B
	"DejaVu Serif Bold Italic",				// FONT_DEJAVU_SERIF_B_I
	"DejaVu Serif Condensed",				// FONT_DEJAVU_SERIF_C
	"DejaVu Serif Condensed Bold",			// FONT_DEJAVU_SERIF_C_B
	"DejaVu Serif Condensed Bold Italic",	// FONT_DEJAVU_SERIF_C_B_I
	"DejaVu Serif Condensed Italic",		// FONT_DEJAVU_SERIF_C_I
	"DejaVu Serif Italic"					// FONT_DEJAVU_SERIF_I
};


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	gboolean			font_status;				// Receives a return code from our font loading function
	gint				format_counter;				// Used to determine if SVG images can be loaded
	GdkPixbufFormat		*format_data;				// Used to determine if SVG images can be loaded
	GValue				*handle_size;				// The size of the handle in the main area
	char				*locale_return;				// Catches the return code when setting locale
	GString				*message;					// Used to construct message strings
	gint				num_formats;				// Used to determine if SVG images can be loaded
	GtkWidget			*outer_box;					// Widget for the onscreen display
	GtkLabel			*resolution_label;			// Widget for the resolution selector label
	GSList				*supported_formats;			// Used to determine if SVG images can be loaded
	GdkColor			temp_colour;				// Temporarily holds colour information
	GString				*title_bar_icon_path;		// Path to the title bar icon image
	gchar				*tmp_gchar;					// Temporary gchar
	guint				tmp_int;					// Temporary guint
	GString				*tmp_gstring;				// Temporary GString
	GtkWidget			*tmp_widget;				// Temporary widget
	GtkWidget			*toolbar;					// Widget for the toolbar
	GdkScreen			*which_screen;				// Gets given the screen the monitor is on
	gchar				wintitle[40];				// Stores the window title
	GtkLabel			*zoom_label;				// Widget for the zoom selector label


	// Set defaults
	set_project_active(FALSE);
	set_default_bg_colour_red(0);
	set_default_bg_colour_green(0);
	set_default_bg_colour_blue(0);
	set_preview_width(300);
	set_frames_per_second(12);  // Half of 24 fps (film)
	set_table_x_padding(5);
	set_table_y_padding(5);
	toolbar = NULL;
	tmp_widget = NULL;

	// Initialise i18n
	locale_return = setlocale(LC_ALL, "");
	if (NULL == locale_return)
	{
		printf("setlocale() failed, unable to set system locale\n");
	} else
	{
		bindtextdomain(PACKAGE, LOCALEDIR);
		textdomain(PACKAGE);
	}

	// Initialise GTK
	gtk_init(&argc, &argv);

	// Initialise various strings
	set_default_zoom_level(_("Fit to width"));  // Sensible default
	set_icon_extension("png");  // Fallback to png format if SVG isn't supported
	message = g_string_new(NULL);
	set_project_name(_("New Project"));
	title_bar_icon_path = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);

	// Load the fonts we use for rendering display
	font_status = load_fonts();

	// Initialise the button event handlers on the toolbars
	set_main_toolbar_signal(DIMENSIONS, 0);
	set_main_toolbar_signal(EXPORT_FLASH, 0);
	for (tmp_int = 0; tmp_int < MAIN_TB_COUNT; tmp_int++)
	{
	    set_layer_toolbar_signal(tmp_int, 0);
	}

	// Initialise the string holding the path of the most recent directory the user accessed
	g_string_printf(tmp_gstring, "%s%s", g_get_home_dir(), G_DIR_SEPARATOR_S);  // We specifically add a trailing slash to the end
	set_last_folder(tmp_gstring->str);

	// Redirect log output so it doesn't pop open a console window
	if ((2 == argc) && (0 == g_ascii_strncasecmp("-d", argv[1], 2)))
	{
		// Disable logging
		set_debug_level(1);
		printf(_("Debugging mode, errors will be shown on stdout.\n"));
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

	if (get_debug_level())
	{
		printf(_("Program path: '%s'\n"), argv[0]);
		printf(_("Directory base: '%s'\n"), g_path_get_dirname(argv[0]));
	}

	// Initialise Ming
	Ming_init();

	// * Work out if SVG images can be loaded *

#ifdef _WIN32
	// Hard coded icon path for windows
	set_icon_path(_("icons"));

	// Mouse pointer image file
	g_string_printf(tmp_gstring, "%s%c%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", get_icon_extension());
	set_mouse_ptr_string(tmp_gstring->str);

#else
	// Default to PNG images, in case an SVG loader isn't present
	set_icon_path(IMAGES_PNG_DIR);

#endif

	// Display debugging info if requested
	if (get_debug_level()) printf(_("Icon path: %s\n"), get_icon_path());

	supported_formats = gdk_pixbuf_get_formats();
	num_formats = g_slist_length(supported_formats);
	for (format_counter = 0; format_counter < num_formats; format_counter++)
	{
		format_data = g_slist_nth_data(supported_formats, format_counter);
		tmp_gchar = gdk_pixbuf_format_get_name(format_data);
		if (0 == g_ascii_strncasecmp(tmp_gchar, "svg", 3));
		{
			// SVG is supported
			set_icon_extension("svg");

#ifndef _WIN32
			// Point to the svg image directory
			set_icon_path(IMAGES_SVG_DIR);

			// Determine path for mouse pointer image file
			g_string_printf(tmp_gstring, "%s%c%s.%s", MOUSE_PTR_SVG_DIR, G_DIR_SEPARATOR, "standard", get_icon_extension());
			set_mouse_ptr_string(tmp_gstring->str);
#else
			// Windows specific
			g_string_printf(tmp_gstring, "%s%c%s%c%s.%s", get_icon_path(), G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", get_icon_extension());
			set_mouse_ptr_string(tmp_gstring->str);
#endif
		}
		g_free(tmp_gchar);
	}

	// Display debugging info if requested
	if (get_debug_level()) printf(_("Path to mouse pointer image: %s\n"), get_mouse_ptr_string());

	// Load initial mouse pointer graphic
	set_mouse_ptr_pixbuf(gdk_pixbuf_new_from_file_at_size(get_mouse_ptr_string(), -1, -1, NULL));

	// Start up the GUI part of things
	set_main_window(gtk_window_new(GTK_WINDOW_TOPLEVEL));
	gtk_window_set_default_size(GTK_WINDOW(get_main_window()), 800, 600);
	gtk_window_set_position(GTK_WINDOW(get_main_window()), GTK_WIN_POS_CENTER);

	// Create the Salasaga status bar icon
	create_status_icon();

	// Attach a signal handler to the main window, so we can track when the window is iconified and deiconfied
	g_signal_connect(G_OBJECT(get_main_window()), "window-state-event", G_CALLBACK(window_state_changed), (gpointer) NULL);

#ifndef _WIN32  // Non-windows check
	// Initialise sound
	gnome_sound_init(NULL);
#endif

	// Load the saved application preferences, if available
	if (FALSE == preferences_load())
	{
		// Looks like Salasaga hasn't been run before, so create it's home directory structure
		g_string_printf(tmp_gstring, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "projects");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "screenshots");
		g_mkdir(tmp_gstring->str, 0750);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "output");
		g_mkdir(tmp_gstring->str, 0750);

		// Which monitor are we displaying on?
		which_screen = gtk_window_get_screen(GTK_WINDOW(get_main_window()));

		// Initialise the application variables to sensible defaults
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "projects");
		set_default_project_folder(tmp_gstring->str);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "screenshots");
		set_screenshots_folder(tmp_gstring->str);
		g_string_printf(tmp_gstring, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "output");
		set_default_output_folder(tmp_gstring->str);
		set_project_width(gdk_screen_get_width(which_screen));
		set_project_height(gdk_screen_get_height(which_screen));
		set_default_output_width(800);
		set_default_output_height(600);
		set_default_slide_duration(5);  // Default number of seconds to use for new slides
		set_default_layer_duration(5);  // Default number of seconds to use for new layers
		set_default_fps(12);
		set_screenshot_key_warning(TRUE);
		temp_colour.red = 0;
		temp_colour.green = 0;
		temp_colour.blue = 0;
		set_default_text_fg_colour(temp_colour);
		set_default_text_font_size(10.0);
		set_default_text_font_face(0);  // Set the default text layer font face to DejaVu Sans
	}

	// Set various required defaults that will be overwritten by the first project loaded
	set_project_folder(get_default_project_folder());
	set_output_folder(get_default_output_folder());

#ifndef _WIN32
	// * Setup the Control-Printscreen key combination to capture screenshots - Non-windows only *
	set_screenshot_command_num(key_bind());
#endif // End of non-windows code

	// Set defaults values for the window capture code
	set_capture_height(get_project_height());
	set_capture_width(get_project_width());
	set_capture_x(0);
	set_capture_y(0);

	// Use the default output width and height
	set_output_width(get_default_output_width());
	set_output_height(get_default_output_height());

	// Set the application title
	snprintf(wintitle, 40, "%s v%s", APP_NAME, APP_VERSION);

	// Set the window title and border
	gtk_window_set_title(GTK_WINDOW(get_main_window()), wintitle);
	gtk_container_set_border_width(GTK_CONTAINER(get_main_window()), 0);

	// Create the title bar icon
	g_string_printf(title_bar_icon_path, "%s%c%s", STATUS_ICON_DIR, G_DIR_SEPARATOR, "salasaga-icon.png");
	gtk_window_set_icon_from_file(GTK_WINDOW(get_main_window()), title_bar_icon_path->str, NULL);
	g_string_free(title_bar_icon_path, TRUE);

	// Set signal handlers to catch user requests to quit
	g_signal_connect(G_OBJECT(get_main_window()), "delete-event", G_CALLBACK(quit_event), NULL);

	// Create the outermost container that everything (menu, main, status bar) fits into
	outer_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(get_main_window()), GTK_WIDGET(outer_box));

	// * Create the menu *
	create_menu_bar();
	if (FALSE == get_menu_bar())
	{
		// Something went wrong when creating the menu bar
		g_string_printf(message, "%s ED01: %s", _("Error"), _("Something went wrong when creating the menu bar."));
		display_warning(message->str);
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(gtk_item_factory_get_widget(get_menu_bar(), "<main>")), FALSE, FALSE, 0);

	// * Create the toolbar *
	toolbar = create_toolbar(toolbar);
	if (FALSE == toolbar)
	{
		// Something went wrong when creating the toolbar
		g_string_printf(message, "%s ED07: %s", _("Error"), _("Something went wrong when creating the toolbar."));
		display_warning(message->str);
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(toolbar), FALSE, FALSE, 0);

	// * Create a HPaned to pack the film strip into, and a vbox for the time line plus working area *
	set_main_area(gtk_hpaned_new());
	handle_size = g_new0(GValue, 1);
	g_value_init(handle_size, G_TYPE_INT);
	gtk_widget_style_get_property(GTK_WIDGET(get_main_area()), "handle-size", handle_size);
	gtk_paned_set_position(GTK_PANED(get_main_area()), g_value_get_int(handle_size) + get_preview_width() + 15);
	gtk_box_pack_start_defaults(GTK_BOX(outer_box), GTK_WIDGET(get_main_area()));

	// Attach signal handlers to the movable handle between film strip and right hand side
	set_film_strip_being_resized(FALSE);
	g_signal_connect(G_OBJECT(get_main_area()), "notify::position", G_CALLBACK(film_strip_handle_changed), (gpointer) NULL);
	g_signal_connect(G_OBJECT(get_main_area()), "button_release_event", G_CALLBACK(film_strip_handle_released), (gpointer) NULL);

	// * Create a table for the status bar, zoom selector, and resolution selectors to go in *
	set_message_bar(GTK_TABLE(gtk_table_new(1, 6, TRUE)));
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(get_message_bar()), FALSE, FALSE, 0);

	// Create the status bar
	set_status_bar(gtk_progress_bar_new());
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(get_status_bar()), GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(get_status_bar()), PANGO_ELLIPSIZE_END);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(get_status_bar()), 0.0);
	gtk_table_attach(get_message_bar(), GTK_WIDGET(get_status_bar()), 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	// Create the zoom selector label
	zoom_label = GTK_LABEL(gtk_label_new(_("Zoom: ")));
	gtk_misc_set_alignment(GTK_MISC(zoom_label), 1, 0.5);
	gtk_table_attach(get_message_bar(), GTK_WIDGET(zoom_label), 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

	// Create the zoom selector
	set_zoom_selector(GTK_COMBO_BOX(create_zoom_selector(get_default_zoom_level())));
	gtk_table_attach(get_message_bar(), GTK_WIDGET(get_zoom_selector()), 3, 4, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);

	// Link the zoom selector to the function that recalculates the zoom and redraws the working area
	g_signal_connect(G_OBJECT(get_zoom_selector()), "changed", G_CALLBACK(zoom_selector_changed), (gpointer) NULL);

	// Create the resolution selector label
	resolution_label = GTK_LABEL(gtk_label_new(_("Output: ")));
	gtk_misc_set_alignment(GTK_MISC(resolution_label), 1, 0.5);
	gtk_table_attach(get_message_bar(), GTK_WIDGET(resolution_label), 4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

	// Create the resolution selector
	set_resolution_selector(GTK_COMBO_BOX(create_resolution_selector(get_output_width(), get_output_height())));
	gtk_table_attach(get_message_bar(), GTK_WIDGET(get_resolution_selector()), 5, 6, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);

	// Link the resolution selector to the function that stores the new values in global variables
	set_resolution_callback(g_signal_connect(G_OBJECT(get_resolution_selector()), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL));

	// * Create the film strip area *
	create_film_strip();
	if (FALSE == get_film_strip_container())
	{
		// Something went wrong when creating the film strip
		g_string_printf(message, "%s ED02: %s", _("Error"), _("Something went wrong when creating the film strip."));
		display_warning(message->str);
		exit(2);
	}
	gtk_paned_add1(GTK_PANED(get_main_area()), GTK_WIDGET(get_film_strip_container()));

	// * Create the vertical box to pack the time line and working area into *
	set_right_side(gtk_vpaned_new());
	create_time_line();  // Create the time line
	tmp_widget = create_working_area(tmp_widget);  // Create the working area
	gtk_paned_add1(GTK_PANED(get_right_side()), GTK_WIDGET(get_time_line_vbox()));
	gtk_paned_add2(GTK_PANED(get_right_side()), GTK_WIDGET(tmp_widget));
	gtk_paned_set_position(GTK_PANED(get_right_side()), 250);
	gtk_paned_add2(GTK_PANED(get_main_area()), GTK_WIDGET(get_right_side()));

	// Create the text tags used in text layers
	create_text_tags();

	// Display a "Ready" message in the status bar
	g_string_printf(message, " %s", _("Ready"));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), message->str);

	// Catch when the window is resized, to automatically recalculate the zoom and redraw the drawing area
	g_signal_connect(G_OBJECT(get_right_side()), "size-allocate", G_CALLBACK(event_size_allocate_received), (gpointer) NULL);

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Gray out the screenshot capture main toolbar icon if screenshots aren't enabled
	if (FALSE == get_screenshots_enabled())
	{
		// Disable the Capture icon
		if (NULL != get_main_toolbar_icon(CAPTURE))
		{
			g_object_ref(get_main_toolbar_icon(CAPTURE));
			gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_main_toolbar_item(CAPTURE)), get_main_toolbar_icon_gray(CAPTURE));
			gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(CAPTURE)), get_main_toolbar_tooltips(), _("Screenshots disabled: salasaga_screencapture not found in search path"), "Private");
			gtk_widget_show_all(GTK_WIDGET(get_main_toolbar_item(CAPTURE)));
		}

		// Disconnect the Capture icon signal handler
		g_signal_handler_disconnect(G_OBJECT(get_main_toolbar_item(CAPTURE)), get_main_toolbar_signal(CAPTURE));
	}

	// Display the main window
	gtk_widget_show_all(get_main_window());

	// Calculate the zoom and drawing area, and initialise the project dimensions
	zoom_selector_changed(GTK_WIDGET(get_zoom_selector()), NULL, (gpointer) NULL);
	resolution_selector_changed(GTK_WIDGET(get_resolution_selector()), NULL, (gpointer) NULL);

	// Start the main event loop
	gtk_main();

	// Free the memory use in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_slist_free(supported_formats);
	g_free(handle_size);

	// Exit
	exit(0);
}

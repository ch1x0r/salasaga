/*
 * $Id$
 *
 * Salasaga: Main source file for the Salasaga IDE
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "config.h"
#include "salasaga_types.h"
#include "externs.h"
#include "functions/create_menu_bar.h"
#include "functions/create_resolution_selector.h"
#include "functions/create_time_line.h"
#include "functions/create_toolbar.h"
#include "functions/create_zoom_selector.h"
#include "functions/disable_layer_toolbar_buttons.h"
#include "functions/disable_main_toolbar_buttons.h"
#include "functions/key_bind.h"
#include "functions/load_fonts.h"
#include "functions/logger_simple.h"
#include "functions/logger_with_domain.h"
#include "functions/preferences_load.h"
#include "functions/quit_event.h"
#include "functions/resolution_selector_changed.h"
#include "functions/zoom_selector_changed.h"
#include "functions/dialog/display_warning.h"
#include "functions/film_strip/create_film_strip.h"
#include "functions/film_strip/film_strip_handle_changed.h"
#include "functions/film_strip/film_strip_handle_released.h"
#include "functions/working_area/create_working_area.h"
#include "functions/working_area/event_size_allocate_received.h"


// Global variables
GList					*boundary_list = NULL;		// Stores a linked list of layer object boundaries
cairo_font_face_t		*cairo_font_face[FONT_DEJAVU_SERIF_I + 1];  // The font face we use
guint					capture_height;				// Height for screen captures
guint					capture_width;				// Width for screen captures
guint					capture_x;					// X offset for screen captures
guint					capture_y;					// Y offset for screen captures
gboolean				changes_made = FALSE;		// Have there been changes to the project since the last save?
layer					*copy_layer = NULL;			// The copy buffer for layers
GList					*current_slide = NULL;		// Pointer to the presently selected slide
guint					debug_level = 0;			// Used to indicate debugging level
guint					end_behaviour = END_BEHAVIOUR_STOP;  // Holds the end behaviour for output animations
guint					end_point_status = END_POINTS_INACTIVE;  // Is one of the layer end points being moved?
gfloat					export_time_counter;		// Used when exporting, holds the number of seconds thus far
GString					*file_name = NULL;			// Holds the file name the project is saved as
gboolean				film_strip_being_resized;	// Toggle to indicate if the film strip is being resized
GtkTreeViewColumn		*film_strip_column;			// Pointer to the film strip column
GtkScrolledWindow		*film_strip_container;		// Container for the film strip
GtkListStore			*film_strip_store;			// Film strip list store
GtkWidget				*film_strip_view;			// The view of the film strip list store
FT_Face					ft_font_face[FONT_DEJAVU_SERIF_I + 1];  // Array of FreeType font face handles
guint					frames_per_second;			// Number of frames per second
GdkPixmap				*front_store;				// Front store for double buffering the workspace area
GString					*icon_extension;			// Used to determine if SVG images can be loaded
GString					*icon_path;					// Points to the base location for Salasaga icon files
gboolean				info_display = TRUE;		// Toggle for whether to display the information button in swf output
GString					*info_link;					//
GString					*info_link_target;			//
GtkTextBuffer			*info_text;					// Text to be shown in the information button in swf output
gint					invalidation_end_x;			// Right side of the front store area to invalidate
gint					invalidation_end_y;			// Bottom of the front store area to invalidate
gint					invalidation_start_x;		// Left side of the front store area to invalidate
gint					invalidation_start_y;		// Top of the front store area to invalidate
GString					*last_folder;				// Keeps track of the last folder the user visited
GtkWidget				*main_area;					// Widget for the onscreen display
GtkWidget				*main_drawing_area;			// Widget for the drawing area
GtkWidget				*main_window;				// Widget for the main window
GtkItemFactory			*menu_bar = NULL;			// Widget for the menu bar
GtkTable				*message_bar;				// Widget for message bar
gboolean				metacity_key_warning;		// Should the metacity key bind warning be displayed?
gboolean				mouse_click_double_added;	// Have we added a double mouse click to the exported swf yet?
gboolean				mouse_click_single_added;	// Have we added a single mouse click to the exported swf yet?
gboolean				mouse_click_triple_added;	// Have we added a triple mouse click to the exported swf yet?
gboolean				mouse_dragging = FALSE;		// Is the mouse being dragged?
GdkPixbuf				*mouse_ptr_pixbuf;			// Temporary GDK Pixbuf
GString					*mouse_ptr_string;			// Full path to the mouse pointer graphic
gboolean				new_layer_selected = TYPE_NONE;	// Is a new layer being created?
GIOChannel				*output_file;				// The output file handle
gboolean				project_active;				// Whether or not a project is active (i.e. something is loaded or has been created)
gulong					resolution_callback;		// Holds the id of the resolution selector callback
GtkComboBox				*resolution_selector;		// Widget for the resolution selector
GdkRectangle			resize_handles_rect[8];		// Contains the onscreen offsets and size for the resize handles
guint					resize_handles_status;		// Are the layer resize handles active, in progress, etc
guint					resize_handle_size = 6;		// Size of the resize handles
GtkWidget				*right_side;				// Widget for the right side area
gboolean				screenshots_enabled = FALSE;  // Toggle for whether to enable screenshots
gint					screenshot_command_num = -1;  // The metacity run command number used for the screenshot key
gboolean				show_control_bar = TRUE;	// Toggle for whether to display the control bar in swf output
GList					*slides = NULL;				// Linked list holding the slide info
guint					start_behaviour = START_BEHAVIOUR_PAUSED;  // Holds the start behaviour for output animations
GtkWidget				*status_bar;				// Widget for the status bar
gint					stored_x;					// X co-ordinate of the mouse last click
gint					stored_y;					// Y co-ordinate of the mouse last click
gint					table_x_padding;			// Number of pixels to pad table entries by
gint					table_y_padding;			// Number of pixels to pad table entries by
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
GtkWidget				*main_toolbar_icons[MAIN_TB_COUNT];			// Array of toolbar icons
GtkWidget				*main_toolbar_icons_gray[MAIN_TB_COUNT];	// Array of toolbar icons (the grayed out ones)
GtkToolItem				*main_toolbar_items[MAIN_TB_COUNT];			// Array of toolbar items
gulong					main_toolbar_signals[MAIN_TB_COUNT];		// Array of toolbar signals

// Layer toolbar items
GtkTooltips				*layer_toolbar_tooltips;	// Tooltips structure
GtkWidget				*layer_toolbar_icons[MAIN_TB_COUNT];		// Array of toolbar icons
GtkWidget				*layer_toolbar_icons_gray[MAIN_TB_COUNT];	// Array of toolbar icons (the grayed out ones)
GtkToolItem				*layer_toolbar_items[MAIN_TB_COUNT];		// Array of toolbar items
gulong					layer_toolbar_signals[MAIN_TB_COUNT];		// Array of toolbar signals

// Application default preferences
GdkColor				default_bg_colour;			// Default background colour for slides
guint					default_fps;				// Default number of frames per second
GString					*default_output_folder;		// Application default save path for exporting animations
guint					default_output_height;		// Application default for how high to create project output
guint					default_output_width;		// Application default for how wide to create project output
GString					*default_project_folder;	// Application default save path for project folders
gfloat					default_slide_duration;		// Default length of all new slides, in frames
GString					*default_zoom_level;		// Default zoom level to use
guint					icon_height = 30;			// Height in pixels for the toolbar icons (they're scalable SVG's)
gfloat					default_layer_duration;		// Length of all new layers, in frames
guint					preview_width;				// Width in pixel for the film strip preview (might turn into a structure later)
GString					*screenshots_folder;		// Application default for where to store screenshots

// Project preferences
GString					*output_folder;				// Where to export output files too
guint					output_height;				// How high to create project output
guint					output_width;				// How wide to create project output
GString					*project_folder;			// The path to the project folder
guint					project_height;				// The height of the project in pixels
GString					*project_name;				// The name of the project
guint					project_width;				// The width of the project in pixels

#ifdef _WIN32
// Windows only variables
HHOOK					win32_keyboard_hook_handle = NULL;		// Handle used to keep track of the Win32 keyboard hook
#endif


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
	GtkWidget			*toolbar;					// Widget for the toolbar
	GdkScreen			*which_screen;				// Gets given the screen the monitor is on
	gchar				wintitle[40];				// Stores the window title
	GtkLabel			*zoom_label;				// Widget for the zoom selector label

	gchar				*tmp_gchar;					// Temporary gchar
	GString				*tmp_gstring;				// Temporary GString
	guint				tmp_int;					// Temporary guint
	GtkWidget			*tmp_widget;				// Temporary widget


	// Set defaults
	project_active = FALSE;
	default_bg_colour.red = 0;
	default_bg_colour.green = 0;
	default_bg_colour.blue = 0;
	preview_width = 300;
	frames_per_second = 12;  // Half of 24 fps (film)
	table_x_padding = 5;
	table_y_padding = 5;
	toolbar = NULL;
	tmp_widget = NULL;
	unscaled_button_height = 50;
	unscaled_button_spacing = 5;
	unscaled_button_width = 50;


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

	// Initialise various strings
	default_output_folder = g_string_new(NULL);
	default_project_folder = g_string_new(NULL);
	default_zoom_level = g_string_new(_("Fit to width"));  // Sensible default
	icon_extension = g_string_new("png");  // Fallback to png format if SVG isn't supported
	icon_path = g_string_new(NULL);
	message = g_string_new(NULL);
	mouse_ptr_string = g_string_new(NULL);
	output_folder = g_string_new(NULL);
	project_folder = g_string_new(NULL);
	project_name = g_string_new(_("New Project"));
	screenshots_folder = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);

	// Load the fonts we use for rendering display
	font_status = load_fonts();

	// Initialise the button event handlers on the toolbars to NULL
	main_toolbar_signals[CROP_ALL] = 0;
	main_toolbar_signals[EXPORT_FLASH] = 0;
	for (tmp_int = 0; tmp_int < MAIN_TB_COUNT; tmp_int++)
	{
	    layer_toolbar_signals[tmp_int] = 0;
	}

	// Initialise GTK
	gtk_init(&argc, &argv);

	// Initialise the string holding the path of the most recent directory the user accessed
	last_folder = g_string_new(g_get_home_dir());
	g_string_append(last_folder, G_DIR_SEPARATOR_S);  //  Add a trailing slash to the folder name

	// Redirect log output so it doesn't pop open a console window
	if ((2 == argc) && (0 == g_ascii_strncasecmp("-d", argv[1], 2)))
	{
		// Disable logging
		debug_level = 1;
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

	if (debug_level)
	{
		printf(_("Program path: '%s'\n"), argv[0]);
		printf(_("Directory base: '%s'\n"), g_path_get_dirname(argv[0]));
	}

	// Initialise Ming
	Ming_init();

	// * Work out if SVG images can be loaded *

#ifdef _WIN32
	// Hard coded icon path for windows
	icon_path = g_string_assign(icon_path, _("icons"));

	// Mouse pointer image file
	g_string_printf(mouse_ptr_string, "%s%c%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", icon_extension->str);

#else
	// Default to PNG images, in case an SVG loader isn't present
	g_string_assign(icon_path, IMAGES_PNG_DIR);

#endif

	// Display debugging info if requested
	if (debug_level) printf(_("Icon path: %s\n"), icon_path->str);

	supported_formats = gdk_pixbuf_get_formats();
	num_formats = g_slist_length(supported_formats);
	for (format_counter = 0; format_counter < num_formats; format_counter++)
	{
		format_data = g_slist_nth_data(supported_formats, format_counter);
		tmp_gchar = gdk_pixbuf_format_get_name(format_data);
		if (0 == g_ascii_strncasecmp(tmp_gchar, "svg", 3));
		{
			// SVG is supported
			icon_extension = g_string_assign(icon_extension, "svg");

#ifndef _WIN32
			// Point to the svg image directory
			g_string_assign(icon_path, IMAGES_SVG_DIR);

			// Determine path for mouse pointer image file
			g_string_printf(mouse_ptr_string, "%s%c%s.%s", MOUSE_PTR_SVG_DIR, G_DIR_SEPARATOR, "standard", icon_extension->str);
#else
			// Windows specific
			g_string_printf(mouse_ptr_string, "%s%c%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "pointers", G_DIR_SEPARATOR, "standard", icon_extension->str);
#endif
		}
		g_free(tmp_gchar);
	}

	// Display debugging info if requested
	if (debug_level) printf(_("Path to mouse pointer image: %s\n"), mouse_ptr_string->str);

	// Load initial mouse pointer graphic
	mouse_ptr_pixbuf = gdk_pixbuf_new_from_file_at_size(mouse_ptr_string->str, -1, -1, NULL);

	// Start up the GUI part of things
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

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
		which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

		// Initialise the application variables to sensible defaults
		g_string_printf(default_project_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "projects");
		g_string_printf(screenshots_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "screenshots");
		g_string_printf(default_output_folder, "%s%c%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "salasaga", G_DIR_SEPARATOR, "output");
		project_width = gdk_screen_get_width(which_screen);
		project_height = gdk_screen_get_height(which_screen);
		default_output_width = 800;
		default_output_height = 600;
		default_slide_duration = 5;  // Default number of seconds to use for new slides
		default_layer_duration = 5;  // Default number of seconds to use for new layers
		default_fps = 12;
		metacity_key_warning = TRUE;
	}

	// Set various required defaults that will be overwritten by the first project loaded
	g_string_printf(project_folder, "%s", default_project_folder->str);
	g_string_printf(output_folder, "%s", default_output_folder->str);

#ifndef _WIN32
	// * Setup the Control-Printscreen key combination to capture screenshots - Non-windows only *
	screenshot_command_num = key_bind();
#endif // End of non-windows code

	// Set defaults values for the window capture code
	capture_height = project_height;
	capture_width = project_width;
	capture_x = 0;
	capture_y = 0;

	// Use the default output width and height
	output_width = default_output_width;
	output_height = default_output_height;

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
		g_string_printf(message, "%s ED01: %s", _("Error"), _("Something went wrong when creating the menu bar."));
		display_warning(message->str);
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(gtk_item_factory_get_widget(menu_bar, "<main>")), FALSE, FALSE, 0);

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
	message_bar = GTK_TABLE(gtk_table_new(1, 6, TRUE));
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(message_bar), FALSE, FALSE, 0);

	// Create the status bar
	status_bar = gtk_progress_bar_new();
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(status_bar), GTK_PROGRESS_LEFT_TO_RIGHT);
	gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(status_bar), PANGO_ELLIPSIZE_END);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(status_bar), 0.0);
	gtk_table_attach(message_bar, GTK_WIDGET(status_bar), 0, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);

	// Create the zoom selector label
	zoom_label = GTK_LABEL(gtk_label_new(_("Zoom: ")));
	gtk_misc_set_alignment(GTK_MISC(zoom_label), 1, 0.5);
	gtk_table_attach(message_bar, GTK_WIDGET(zoom_label), 2, 3, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

	// Create the zoom selector
	zoom_selector = GTK_COMBO_BOX(create_zoom_selector(default_zoom_level->str));
	gtk_table_attach(message_bar, GTK_WIDGET(zoom_selector), 3, 4, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);

	// Link the zoom selector to the function that recalculates the zoom and redraws the working area
	g_signal_connect(G_OBJECT(zoom_selector), "changed", G_CALLBACK(zoom_selector_changed), (gpointer) NULL);

	// Create the resolution selector label
	resolution_label = GTK_LABEL(gtk_label_new(_("Output: ")));
	gtk_misc_set_alignment(GTK_MISC(resolution_label), 1, 0.5);
	gtk_table_attach(message_bar, GTK_WIDGET(resolution_label), 4, 5, 0, 1, GTK_SHRINK, GTK_SHRINK, 0, 0);

	// Create the resolution selector
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(output_width, output_height));
	gtk_table_attach(message_bar, GTK_WIDGET(resolution_selector), 5, 6, 0, 1, GTK_FILL, GTK_SHRINK, 0, 0);

	// Link the resolution selector to the function that stores the new values in global variables
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);

	// * Create the film strip area *
	create_film_strip();
	if (FALSE == film_strip_container)
	{
		// Something went wrong when creating the film strip
		g_string_printf(message, "%s ED02: %s", _("Error"), _("Something went wrong when creating the film strip."));
		display_warning(message->str);
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

	// Display a "Ready" message in the status bar
	g_string_printf(message, " %s", _("Ready"));
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), message->str);

	// Calculate the zoom and drawing area, and initialise the project dimensions
	zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
	resolution_selector_changed(GTK_WIDGET(resolution_selector), NULL, (gpointer) NULL);

	// Catch when the window is resized, to automatically recalculate the zoom and redraw the drawing area
	g_signal_connect(G_OBJECT(right_side), "size-allocate", G_CALLBACK(event_size_allocate_received), (gpointer) NULL);

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Gray out the screenshot capture main toolbar icon if screenshots aren't enabled
	if (FALSE == screenshots_enabled)
	{
		// Disable the Capture icon
		if (NULL != main_toolbar_icons[CAPTURE])
		{
			g_object_ref(main_toolbar_icons[CAPTURE]);
			gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[CAPTURE]), main_toolbar_icons_gray[CAPTURE]);
			gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CAPTURE]), main_toolbar_tooltips, _("Screenshots disabled: salasaga_screencapture not found in search path"), "Private");
			gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[CAPTURE]));
		}

		// Disconnect the Capture icon signal handler
		g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[CAPTURE]), main_toolbar_signals[CAPTURE]);
	}

	// Display the main window
	gtk_widget_show_all(main_window);

	// Start the main event loop
	gtk_main();

	// Free the memory use in this function
	g_string_free(icon_extension, TRUE);
	g_string_free(icon_path, TRUE);
	g_string_free(mouse_ptr_string, TRUE);
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_slist_free(supported_formats);
	g_free(handle_size);

	// Exit
	exit(0);
}

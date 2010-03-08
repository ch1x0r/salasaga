/*
 * $Id$
 *
 * Salasaga: Functions for accessing and setting globally available variables
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


// Global variables
static GList				*boundary_list = NULL;			// Stores a linked list of layer object boundaries
static cairo_font_face_t	*cairo_font_face[FONT_COUNT];	// The ttf font faces we use get loaded into this
static guint				capture_height;					// Height for screen captures
static guint				capture_width;					// Width for screen captures
static guint				capture_x;
static guint				capture_y;
static gboolean				changes_made = FALSE;			// Have there been changes to the project since the last save?
static layer				*copy_layer = NULL;				// The copy buffer for layers
static guint				debug_level = 0;				// Used to indicate debugging level
static GdkColor				default_text_fg_colour;			// Default foreground colour for text layer text
static gint					default_text_font_face;			// Default font face in text layers
static gdouble				default_text_font_size;			// Default font size in text layers
static gboolean				display_help_text = TRUE;		// Should we display help text and dialogs?
static guint				end_behaviour = END_BEHAVIOUR_STOP;  // Holds the end behaviour for output animations
static guint				end_point_status = END_POINTS_INACTIVE;  // Is one of the layer end points being moved?
static gboolean				film_strip_being_resized;		// Toggle to indicate if the film strip is being resized
static GtkWidget			*film_strip_view;				// The view of the film strip list store
static guint				frames_per_second;				// Number of frames per second
static GString				*icon_extension;				// Used to determine if SVG images can be loaded
static GString				*icon_path;						// Points to the base location for Salasaga icon files
static gboolean				info_display = TRUE;			// Toggle for whether to display the information button in swf output
static gint					invalidation_end_x;				// Right side of the front store area to invalidate
static gint					invalidation_end_y;				// Bottom of the front store area to invalidate
static gint					invalidation_start_x;			// Left side of the front store area to invalidate
static gint					invalidation_start_y;			// Top of the front store area to invalidate
static GString				*last_folder;					// Keeps track of the last folder the user visited
static GtkWidget			*main_area;						// Widget for the onscreen display
static GtkWidget			*main_drawing_area;				// Widget for the drawing area
static GtkWidget			*main_window;					// Widget for the main window
static gboolean				mouse_click_double_added;		// Have we added a double mouse click to the exported swf yet?
static gboolean				mouse_click_single_added;		// Have we added a single mouse click to the exported swf yet?
static gboolean				mouse_click_triple_added;		// Have we added a triple mouse click to the exported swf yet?
static gboolean				mouse_dragging = FALSE;			// Is the mouse being dragged?
static GString				*mouse_ptr_string;				// Full path to the mouse pointer graphic
static gboolean				new_layer_selected = TYPE_NONE;	// Is a new layer being created?
static gboolean				project_active;					// Whether or not a project is active (i.e. something is loaded or has been created)
static guint				resize_handles_status;			// Are the layer resize handles active, in progress, etc
static guint				resize_handle_size = 6;			// Size of the resize handles
static gulong				resolution_callback;			// Holds the id of the resolution selector callback
static GtkWidget			*right_side;					// Widget for the right side area
static gint					screenshot_command_num = -1;	// The metacity run command number used for the screenshot key
static guint				screenshot_delay_time = 5;		// The number of seconds the screenshot trigger is delayed
static gboolean				screenshot_key_warning;			// Should the warning about not being able to set the screenshot key be displayed?
static gboolean				screenshots_enabled = FALSE;	// Toggle for whether to enable screenshots
static gboolean				show_control_bar = TRUE;		// Toggle for whether to display the control bar in swf output
static guint				start_behaviour = START_BEHAVIOUR_PAUSED;  // Holds the start behaviour for output animations
static GtkWidget			*status_bar;					// Widget for the status bar
static gint					stored_x;						// X co-ordinate of the mouse last click
static gint					stored_y;						// Y co-ordinate of the mouse last click
static gint					table_x_padding;				// Number of pixels to pad table entries by
static gint					table_y_padding;				// Number of pixels to pad table entries by
static GtkWidget			*time_line_container;			// Scrolled window widget, to add scroll bars to the time line widget
static GtkWidget			*time_line_vbox;				// VBox widget holding all of the time line elements
static guint				working_height;					// Height of the display portion of the working area in pixels
static guint				working_width;					// Width of the display portion of the working area in pixels
static guint				zoom;							// Percentage zoom to use in the drawing area


// Functions to get and set the variables
GList *get_boundary_list()
{
	return boundary_list;
}

cairo_font_face_t *get_cairo_font_face(guint index)
{
	return cairo_font_face[index];
}

guint get_capture_height()
{
	return capture_height;
}

guint get_capture_width()
{
	return capture_width;
}

guint get_capture_x()
{
	return capture_x;
}

guint get_capture_y()
{
	return capture_y;
}

gboolean get_changes_made()
{
	return changes_made;
}

layer *get_copy_layer()
{
	return copy_layer;
}

guint get_debug_level()
{
	return debug_level;
}

GdkColor get_default_text_fg_colour()
{
	return default_text_fg_colour;
}

gint get_default_text_font_face()
{
	return default_text_font_face;
}

gdouble get_default_text_font_size()
{
	return default_text_font_size;
}

gboolean get_display_help_text()
{
	return display_help_text;
}

guint get_end_behaviour()
{
	return end_behaviour;
}

guint get_end_point_status()
{
	return end_point_status;
}

gboolean get_film_strip_being_resized()
{
	return film_strip_being_resized;
}

GtkWidget *get_film_strip_view()
{
	return film_strip_view;
}

guint get_frames_per_second()
{
	return frames_per_second;
}

gchar *get_icon_extension()
{
	return icon_extension->str;
}

gsize get_icon_extension_length()
{
	return icon_extension->len;
}

gchar *get_icon_path()
{
	return icon_path->str;
}

gsize get_icon_path_length()
{
	return icon_path->len;
}

gboolean get_info_display()
{
	return info_display;
}

gint get_invalidation_end_x()
{
	return invalidation_end_x;
}

gint get_invalidation_end_y()
{
	return invalidation_end_y;
}

gint get_invalidation_start_x()
{
	return invalidation_start_x;
}

gint get_invalidation_start_y()
{
	return invalidation_start_y;
}

gchar *get_last_folder()
{
	return last_folder->str;
}

gsize get_last_folder_length()
{
	return last_folder->len;
}

GtkWidget *get_main_area()
{
	return main_area;
}

GtkWidget *get_main_drawing_area()
{
	return main_drawing_area;
}

GtkWidget *get_main_window()
{
	return main_window;
}

gboolean get_mouse_click_double_added()
{
	return mouse_click_double_added;
}

gboolean get_mouse_click_single_added()
{
	return mouse_click_single_added;
}

gboolean get_mouse_click_triple_added()
{
	return mouse_click_triple_added;
}

gboolean get_mouse_dragging()
{
	return mouse_dragging;
}

gchar *get_mouse_ptr_string()
{
	return mouse_ptr_string->str;
}

gsize get_mouse_ptr_string_length()
{
	return mouse_ptr_string->len;
}

gboolean get_new_layer_selected()
{
	return new_layer_selected;
}

gboolean get_project_active()
{
	return project_active;
}

GtkWidget *get_right_side()
{
	return right_side;
}

guint get_resize_handles_status()
{
	return resize_handles_status;
}

guint get_resize_handle_size()
{
	return resize_handle_size;
}

gulong get_resolution_callback()
{
	return resolution_callback;
}

gint get_screenshot_command_num()
{
	return screenshot_command_num;
}

guint get_screenshot_delay_time()
{
	return screenshot_delay_time;
}

gboolean get_screenshot_key_warning()
{
	return screenshot_key_warning;
}

gboolean get_screenshots_enabled()
{
	return screenshots_enabled;
}

gboolean get_show_control_bar()
{
	return show_control_bar;
}

guint get_start_behaviour()
{
	return start_behaviour;
}

GtkWidget *get_status_bar()
{
	return status_bar;
}

gint get_stored_x()
{
	return stored_x;
}

gint get_stored_y()
{
	return stored_y;
}

gint get_table_x_padding()
{
	return table_x_padding;
}

gint get_table_y_padding()
{
	return table_y_padding;
}

GtkWidget *get_time_line_container()
{
	return time_line_container;
}

GtkWidget *get_time_line_vbox()
{
	return time_line_vbox;
}

guint get_working_height()
{
	return working_height;
}

guint get_working_width()
{
	return working_width;
}

guint get_zoom()
{
	return zoom;
}

void set_boundary_list(GList *new_boundary_list)
{
	boundary_list = new_boundary_list;
}

void set_cairo_font_face(guint index, cairo_font_face_t *new_cairo_font_face)
{
	cairo_font_face[index] = new_cairo_font_face;
}

void set_capture_height(guint new_capture_height)
{
	capture_height = new_capture_height;
}

void set_capture_width(guint new_capture_width)
{
	capture_width = new_capture_width;
}

void set_capture_x(guint new_capture_x)
{
	capture_x = new_capture_x;
}

void set_capture_y(guint new_capture_y)
{
	capture_y = new_capture_y;
}

void set_changes_made(gboolean new_changes_made)
{
	changes_made = new_changes_made;
}

void set_copy_layer(layer *new_copy_layer)
{
	copy_layer = new_copy_layer;
}

void set_debug_level(guint new_debug_level)
{
	debug_level = new_debug_level;
}

void set_default_text_fg_colour(GdkColor new_default_text_fg_colour)
{
	default_text_fg_colour = new_default_text_fg_colour;
}

void set_default_text_font_face(gint new_default_text_font_face)
{
	default_text_font_face = new_default_text_font_face;
}

void set_default_text_font_size(gdouble new_default_text_font_size)
{
	default_text_font_size = new_default_text_font_size;
}

void set_display_help_text(gboolean new_display_help_text)
{
	display_help_text = new_display_help_text;
}

void set_end_behaviour(guint new_end_behaviour)
{
	end_behaviour = new_end_behaviour;
}

void set_end_point_status(guint new_end_point_status)
{
	end_point_status = new_end_point_status;
}

void set_film_strip_being_resized(gboolean new_film_strip_being_resized)
{
	film_strip_being_resized = new_film_strip_being_resized;
}

void set_film_strip_view(GtkWidget *new_film_strip_view)
{
	film_strip_view = new_film_strip_view;
}

void set_frames_per_second(guint new_frames_per_second)
{
	frames_per_second = new_frames_per_second;
}

void set_icon_extension(gchar *new_icon_extension)
{
	if (NULL == icon_extension)
	{
		icon_extension = g_string_new(new_icon_extension);
	} else
	{
		icon_extension = g_string_assign(icon_extension, new_icon_extension);
	}
}

void set_icon_path(gchar *new_icon_path)
{
	if (NULL == icon_path)
	{
		icon_path = g_string_new(new_icon_path);
	} else
	{
		icon_path = g_string_assign(icon_path, new_icon_path);
	}
}

void set_info_display(gboolean new_info_display)
{
	info_display = new_info_display;
}

void set_invalidation_end_x(gint new_invalidation_end_x)
{
	invalidation_end_x = new_invalidation_end_x;
}

void set_invalidation_end_y(gint new_invalidation_end_y)
{
	invalidation_end_y = new_invalidation_end_y;
}

void set_invalidation_start_x(gint new_invalidation_start_x)
{
	invalidation_start_x = new_invalidation_start_x;
}

void set_invalidation_start_y(gint new_invalidation_start_y)
{
	invalidation_start_y = new_invalidation_start_y;
}

void set_last_folder(gchar *new_last_folder)
{
	if (NULL == last_folder)
	{
		last_folder = g_string_new(new_last_folder);
	} else
	{
		last_folder = g_string_assign(last_folder, new_last_folder);
	}
}

void set_main_area(GtkWidget *new_main_area)
{
	main_area = new_main_area;
}

void set_main_drawing_area(GtkWidget *new_main_drawing_area)
{
	main_drawing_area = new_main_drawing_area;
}

void set_main_window(GtkWidget *new_main_window)
{
	main_window = new_main_window;
}

void set_mouse_click_double_added(gboolean new_mouse_click_double_added)
{
	mouse_click_double_added = new_mouse_click_double_added;
}

void set_mouse_click_single_added(gboolean new_mouse_click_single_added)
{
	mouse_click_single_added = new_mouse_click_single_added;
}

void set_mouse_click_triple_added(gboolean new_mouse_click_triple_added)
{
	mouse_click_triple_added = new_mouse_click_triple_added;
}

void set_mouse_dragging(gboolean new_mouse_dragging)
{
	mouse_dragging = new_mouse_dragging;
}

void set_mouse_ptr_string(gchar *new_mouse_ptr_string)
{
	if (NULL == last_folder)
	{
		mouse_ptr_string = g_string_new(new_mouse_ptr_string);
	} else
	{
		mouse_ptr_string = g_string_assign(mouse_ptr_string, new_mouse_ptr_string);
	}
}

void set_new_layer_selected(gboolean new_new_layer_selected)
{
	new_layer_selected = new_new_layer_selected;
}

void set_project_active(gboolean new_project_active)
{
	project_active = new_project_active;
}

void set_resize_handles_status(guint new_resize_handles_status)
{
	resize_handles_status = new_resize_handles_status;
}

void set_resize_handle_size(guint new_resize_handle_size)
{
	resize_handle_size = new_resize_handle_size;
}

void set_resolution_callback(gulong new_resolution_callback)
{
	resolution_callback = new_resolution_callback;
}

void set_right_side(GtkWidget *new_right_side)
{
	right_side = new_right_side;
}

void set_screenshot_command_num(gint new_screenshot_command_num)
{
	screenshot_command_num = new_screenshot_command_num;
}

void set_screenshot_delay_time(guint new_screenshot_delay_time)
{
	screenshot_delay_time = new_screenshot_delay_time;
}

void set_screenshot_key_warning(gboolean new_screenshot_key_warning)
{
	screenshot_key_warning = new_screenshot_key_warning;
}

void set_screenshots_enabled(gboolean new_screenshots_enabled)
{
	screenshots_enabled = new_screenshots_enabled;
}

void set_show_control_bar(gboolean new_show_control_bar)
{
	show_control_bar = new_show_control_bar;
}

void set_start_behaviour(guint new_start_behaviour)
{
	start_behaviour = new_start_behaviour;
}

void set_status_bar(GtkWidget *new_status_bar)
{
	status_bar = new_status_bar;
}

void set_stored_x(gint new_stored_x)
{
	stored_x = new_stored_x;
}

void set_stored_y(gint new_stored_y)
{
	stored_y = new_stored_y;
}

void set_table_x_padding(gint new_table_x_padding)
{
	table_x_padding = new_table_x_padding;
}

void set_table_y_padding(gint new_table_y_padding)
{
	table_y_padding = new_table_y_padding;
}

void set_time_line_container(GtkWidget *new_time_line_container)
{
	time_line_container = new_time_line_container;
}

void set_time_line_vbox(GtkWidget *new_time_line_vbox)
{
	time_line_vbox = new_time_line_vbox;
}

void set_working_height(guint new_working_height)
{
	working_height = new_working_height;
}

void set_working_width(guint new_working_width)
{
	working_width = new_working_width;
}

void set_zoom(guint new_zoom)
{
	zoom = new_zoom;
}

/*
 * $Id$
 *
 * Salasaga: Functions for accessing and setting globally available variables
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

// FreeType includes
#include <ft2build.h>
#include FT_FREETYPE_H

// Ming include
#include <ming.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../valid_fields.h"


// Application wide variables
static GList				*boundary_list = NULL;			// Stores a linked list of layer object boundaries
static cairo_font_face_t	*cairo_font_face[FONT_COUNT];	// The ttf font faces we use get loaded into this
static guint				capture_height;					// Height for screen captures
static guint				capture_width;					// Width for screen captures
static guint				capture_x;
static guint				capture_y;
static gboolean				changes_made = FALSE;			// Have there been changes to the project since the last save?
static layer				*copy_layer = NULL;				// The copy buffer for layers
static GList				*current_slide = NULL;			// Pointer to the presently selected slide
static guint				debug_level = 0;				// Used to indicate debugging level
static GdkColor				default_text_fg_colour;			// Default foreground colour for text layer text
static gint					default_text_font_face;			// Default font face in text layers
static gdouble				default_text_font_size;			// Default font size in text layers
static gboolean				display_help_text = TRUE;		// Should we display help text and dialogs?
static guint				end_behaviour = END_BEHAVIOUR_STOP;  // Holds the end behaviour for output animations
static guint				end_point_status = END_POINTS_INACTIVE;  // Is one of the layer end points being moved?
static SWFFont				fdb_font_object[FONT_COUNT];	// The fdb font faces used by Ming are loaded into this
static GString				*file_name = NULL;				// Holds the file name the project is saved as
static gboolean				film_strip_being_resized;		// Toggle to indicate if the film strip is being resized
static GtkTreeViewColumn	*film_strip_column;				// Pointer to the film strip column
static GtkScrolledWindow	*film_strip_container;			// Container for the film strip
static GtkListStore			*film_strip_store;				// Film strip list store
static GtkWidget			*film_strip_view;				// The view of the film strip list store
static guint				frames_per_second;				// Number of frames per second
static GdkPixmap			*front_store;					// Front store for double buffering the workspace area
static FT_Face				ft_font_face[FONT_COUNT];		// Array of FreeType font face handles
static GString				*icon_extension = NULL;			// Used to determine if SVG images can be loaded
static GString				*icon_path = NULL;				// Points to the base location for Salasaga icon files
static gboolean				info_display = TRUE;			// Toggle for whether to display the information button in swf output
static GtkTextBuffer		*info_text;						// Text to be shown in the information button in swf output
static gint					invalidation_end_x;				// Right side of the front store area to invalidate
static gint					invalidation_end_y;				// Bottom of the front store area to invalidate
static gint					invalidation_start_x;			// Left side of the front store area to invalidate
static gint					invalidation_start_y;			// Top of the front store area to invalidate
static GString				*last_folder = NULL;			// Keeps track of the last folder the user visited
static GtkWidget			*main_area;						// Widget for the onscreen display
static GtkWidget			*main_drawing_area;				// Widget for the drawing area
static GtkWidget			*main_window;					// Widget for the main window
static GtkItemFactory		*menu_bar = NULL;				// Widget for the menu bar
static GtkTable				*message_bar;					// Widget for message bar
static gboolean				mouse_click_double_added;		// Have we added a double mouse click to the exported swf yet?
static gboolean				mouse_click_single_added;		// Have we added a single mouse click to the exported swf yet?
static gboolean				mouse_click_triple_added;		// Have we added a triple mouse click to the exported swf yet?
static gboolean				mouse_dragging = FALSE;			// Is the mouse being dragged?
static GdkPixbuf			*mouse_ptr_pixbuf;				// Temporary GDK Pixbuf
static GString				*mouse_ptr_string = NULL;		// Full path to the mouse pointer graphic
static gboolean				new_layer_selected = TYPE_NONE;	// Is a new layer being created?
static gboolean				project_active;					// Whether or not a project is active (i.e. something is loaded or has been created)
static guint				resize_handle_size = 6;			// Size of the resize handles
static GdkRectangle			resize_handles_rect[8];			// Contains the onscreen offsets and size for the resize handles
static guint				resize_handles_status;			// Are the layer resize handles active, in progress, etc
static gulong				resolution_callback;			// Holds the id of the resolution selector callback
static GtkComboBox			*resolution_selector;			// Widget for the resolution selector
static GtkWidget			*right_side;					// Widget for the right side area
static gint					screenshot_command_num = -1;	// The metacity run command number used for the screenshot key
static guint				screenshot_delay_time = 5;		// The number of seconds the screenshot trigger is delayed
static gboolean				screenshot_key_warning;			// Should the warning about not being able to set the screenshot key be displayed?
static gboolean				screenshots_enabled = FALSE;	// Toggle for whether to enable screenshots
static gboolean				show_control_bar = TRUE;		// Toggle for whether to display the control bar in swf output
static GList				*slides = NULL;					// Linked list holding the slide info
static guint				start_behaviour = START_BEHAVIOUR_PAUSED;  // Holds the start behaviour for output animations
static GtkWidget			*status_bar;					// Widget for the status bar
static GtkStatusIcon		*status_icon;					// Pointer to the GtkStatusIcon object, used for StatusIcon communication
static gint					stored_x;						// X co-ordinate of the mouse last click
static gint					stored_y;						// Y co-ordinate of the mouse last click
static gint					table_x_padding;				// Number of pixels to pad table entries by
static gint					table_y_padding;				// Number of pixels to pad table entries by
static GSList				*text_tags_fg_colour_slist = NULL;  // Text tags for text foreground colour, used for changing text colour in text layers
static GtkTextTag			*text_tags_fonts[FONT_COUNT];	// Text tags for font faces, used for applying font faces in text layers
static GSList				*text_tags_size_slist = NULL;	// Text tags for text sizes, used for changing text size in text layers
static GtkTextTagTable		*text_tags_table;				// The table of all text tags, used for applying text tags in text layers
static GtkWidget			*time_line_container;			// Scrolled window widget, to add scroll bars to the time line widget
static GtkWidget			*time_line_vbox;				// VBox widget holding all of the time line elements
static guint				working_height;					// Height of the display portion of the working area in pixels
static guint				working_width;					// Width of the display portion of the working area in pixels
static guint				zoom;							// Percentage zoom to use in the drawing area
static GtkComboBox			*zoom_selector;					// Widget for the zoom selector

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

GList *get_current_slide()
{
	return current_slide;
}

slide *get_current_slide_data()
{
	return current_slide->data;
}

gfloat get_current_slide_duration()
{
	return ((slide *) current_slide->data)->duration;
}

GList *get_current_slide_layers_pointer()
{
	return ((slide *) current_slide->data)->layers;
}

gint get_current_slide_num_layers()
{
	return ((slide *) current_slide->data)->num_layers;
}

GtkWidget *get_current_slide_timeline_widget()
{
	return GTK_WIDGET(((slide *) current_slide->data)->timeline_widget);
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

SWFFont get_fdb_font_object(guint index)
{
	return fdb_font_object[index];
}

gchar *get_file_name()
{
	if (NULL == file_name)
	{
		return NULL;
	} else
	{
		return file_name->str;
	}
}

gsize get_file_name_length()
{
	return file_name->len;
}

gboolean get_film_strip_being_resized()
{
	return film_strip_being_resized;
}

GtkTreeViewColumn *get_film_strip_column()
{
	return film_strip_column;
}

GtkScrolledWindow *get_film_strip_container()
{
	return film_strip_container;
}

GtkListStore *get_film_strip_store()
{
	return film_strip_store;
}

GtkWidget *get_film_strip_view()
{
	return film_strip_view;
}

guint get_frames_per_second()
{
	return frames_per_second;
}

GdkPixmap *get_front_store()
{
	return front_store;
}

FT_Face get_ft_font_face(guint index)
{
	return ft_font_face[index];
}

FT_Face *get_ft_font_face_ptr(guint index)
{
	return &ft_font_face[index];
}

gchar *get_icon_extension()
{
	if (NULL == icon_extension)
	{
		return NULL;
	} else
	{
		return icon_extension->str;
	}
}

gsize get_icon_extension_length()
{
	return icon_extension->len;
}

gchar *get_icon_path()
{
	if (NULL == icon_path)
	{
		return NULL;
	} else
	{
		return icon_path->str;
	}
}

gsize get_icon_path_length()
{
	return icon_path->len;
}

gboolean get_info_display()
{
	return info_display;
}

GtkTextBuffer *get_info_text()
{
	return info_text;
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
	if (NULL == last_folder)
	{
		return NULL;
	} else
	{
		return last_folder->str;
	}
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

GtkItemFactory *get_menu_bar()
{
	return menu_bar;
}

GtkTable *get_message_bar()
{
	return message_bar;
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

GdkPixbuf *get_mouse_ptr_pixbuf()
{
	return mouse_ptr_pixbuf;
}

gchar *get_mouse_ptr_string()
{
	if (NULL == mouse_ptr_string)
	{
		return NULL;
	} else
	{
		return mouse_ptr_string->str;
	}
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

guint get_resize_handle_size()
{
	return resize_handle_size;
}

GdkRectangle *get_resize_handles_rect(guint index)
{
	return &resize_handles_rect[index];
}

gint get_resize_handles_rect_height(guint index)
{
	return resize_handles_rect[index].height;
}

gint get_resize_handles_rect_width(guint index)
{
	return resize_handles_rect[index].width;
}

gint get_resize_handles_rect_x(guint index)
{
	return resize_handles_rect[index].x;
}

gint get_resize_handles_rect_y(guint index)
{
	return resize_handles_rect[index].y;
}

guint get_resize_handles_status()
{
	return resize_handles_status;
}

gulong get_resolution_callback()
{
	return resolution_callback;
}

GtkComboBox *get_resolution_selector()
{
	return resolution_selector;
}

GtkWidget *get_right_side()
{
	return right_side;
}

gchar *get_salasaga_font_name(guint index)
{
	return salasaga_font_names[index];
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

GList *get_slides()
{
	return slides;
}

guint get_start_behaviour()
{
	return start_behaviour;
}

GtkWidget *get_status_bar()
{
	return status_bar;
}

GtkStatusIcon *get_status_icon()
{
	return status_icon;
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

GSList *get_text_tags_fg_colour_slist()
{
	return text_tags_fg_colour_slist;
}

GtkTextTag *get_text_tags_font(guint index)
{
	return text_tags_fonts[index];
}

GSList *get_text_tags_size_slist()
{
	return text_tags_size_slist;
}

GtkTextTagTable *get_text_tags_table()
{
	return text_tags_table;
}

GtkWidget *get_time_line_container()
{
	return time_line_container;
}

GtkWidget *get_time_line_vbox()
{
	return time_line_vbox;
}

guint get_valid_fields_base_type(guint index)
{
	return valid_fields[index].base_type;
}

guint get_valid_fields_capabilities(guint index)
{
	return valid_fields[index].capabilities;
}

gfloat get_valid_fields_min_value(guint index)
{
	return valid_fields[index].min_value;
}

gfloat get_valid_fields_max_value(guint index)
{
	return valid_fields[index].max_value;
}

gchar *get_valid_fields_name(guint index)
{
	return valid_fields[index].name_string;
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

GtkComboBox *get_zoom_selector()
{
	return zoom_selector;
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

void set_current_slide(GList *new_current_slide)
{
	current_slide = new_current_slide;
}

void set_current_slide_data(slide *new_data)
{
	current_slide->data = new_data;
}

void set_current_slide_layers_pointer(GList *new_layer_pointer)
{
	((slide *) current_slide->data)->layers = new_layer_pointer;
}

void set_current_slide_num_layers(gint new_num_layers)
{
	((slide *) current_slide->data)->num_layers = new_num_layers;
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

void set_fdb_font_object(guint index, SWFFont new_fdb_font_object)
{
	fdb_font_object[index] = new_fdb_font_object;
}

void set_file_name(gchar *new_file_name)
{
	if (NULL == file_name)
	{
		file_name = g_string_new(new_file_name);
	} else
	{
		file_name = g_string_assign(file_name, new_file_name);
	}
}

void set_film_strip_being_resized(gboolean new_film_strip_being_resized)
{
	film_strip_being_resized = new_film_strip_being_resized;
}

void set_film_strip_column(GtkTreeViewColumn *new_film_strip_column)
{
	film_strip_column = new_film_strip_column;
}

void set_film_strip_container(GtkScrolledWindow *new_film_strip_container)
{
	film_strip_container = new_film_strip_container;
}

void set_film_strip_store(GtkListStore *new_film_strip_store)
{
	film_strip_store = new_film_strip_store;
}

void set_film_strip_view(GtkWidget *new_film_strip_view)
{
	film_strip_view = new_film_strip_view;
}

void set_frames_per_second(guint new_frames_per_second)
{
	frames_per_second = new_frames_per_second;
}

void set_front_store(GdkPixmap *new_front_store)
{
	front_store = new_front_store;
}

void set_ft_font_face(guint index, FT_Face new_ft_font_face)
{
	ft_font_face[index] = new_ft_font_face;
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

void set_info_text(GtkTextBuffer *new_info_text)
{
	info_text = new_info_text;
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

void set_menu_bar(GtkItemFactory *new_menu_bar)
{
	menu_bar = new_menu_bar;
}

void set_message_bar(GtkTable *new_message_bar)
{
	message_bar = new_message_bar;
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

void set_mouse_ptr_pixbuf(GdkPixbuf *new_mouse_ptr_pixbuf)
{
	mouse_ptr_pixbuf = new_mouse_ptr_pixbuf;
}

void set_mouse_ptr_string(gchar *new_mouse_ptr_string)
{
	if (NULL == mouse_ptr_string)
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

void set_resize_handle_size(guint new_resize_handle_size)
{
	resize_handle_size = new_resize_handle_size;
}

void set_resize_handles_rect_height(guint index, gint new_height)
{
	resize_handles_rect[index].height = new_height;
}

void set_resize_handles_rect_width(guint index, gint new_width)
{
	resize_handles_rect[index].width = new_width;
}

void set_resize_handles_rect_x(guint index, gint new_x)
{
	resize_handles_rect[index].x = new_x;
}

void set_resize_handles_rect_y(guint index, gint new_y)
{
	resize_handles_rect[index].y = new_y;
}

void set_resize_handles_status(guint new_resize_handles_status)
{
	resize_handles_status = new_resize_handles_status;
}

void set_resolution_callback(gulong new_resolution_callback)
{
	resolution_callback = new_resolution_callback;
}

void set_resolution_selector(GtkComboBox *new_resolution_selector)
{
	resolution_selector = new_resolution_selector;
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

void set_slides(GList *new_slides)
{
	slides = new_slides;
}

void set_start_behaviour(guint new_start_behaviour)
{
	start_behaviour = new_start_behaviour;
}

void set_status_bar(GtkWidget *new_status_bar)
{
	status_bar = new_status_bar;
}

void set_status_icon(GtkStatusIcon *new_status_icon)
{
	status_icon = new_status_icon;
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

void set_text_tags_fg_colour_slist(GSList *new_text_tags_fg_colour_slist)
{
	text_tags_fg_colour_slist = new_text_tags_fg_colour_slist;
}

void set_text_tags_font(guint index, GtkTextTag *new_text_tags_font)
{
	text_tags_fonts[index] = new_text_tags_font;
}

void set_text_tags_size_slist(GSList *new_text_tags_size_slist)
{
	text_tags_size_slist = new_text_tags_size_slist;
}

void set_text_tags_table(GtkTextTagTable *new_text_tags_table)
{
	text_tags_table = new_text_tags_table;
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

void set_zoom_selector(GtkComboBox *new_zoom_selector)
{
	zoom_selector = new_zoom_selector;
}

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
static gboolean				info_display = TRUE;			// Toggle for whether to display the information button in swf output
static gboolean				mouse_click_double_added;		// Have we added a double mouse click to the exported swf yet?
static gboolean				mouse_click_single_added;		// Have we added a single mouse click to the exported swf yet?
static gboolean				mouse_click_triple_added;		// Have we added a triple mouse click to the exported swf yet?
static gboolean				mouse_dragging = FALSE;			// Is the mouse being dragged?
static gboolean				new_layer_selected = TYPE_NONE;	// Is a new layer being created?
static gboolean				project_active;					// Whether or not a project is active (i.e. something is loaded or has been created)
static gboolean				screenshot_key_warning;			// Should the warning about not being able to set the screenshot key be displayed?
static gboolean				screenshots_enabled = FALSE;	// Toggle for whether to enable screenshots
static gboolean				show_control_bar = TRUE;		// Toggle for whether to display the control bar in swf output


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

gboolean get_info_display()
{
	return info_display;
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

gboolean get_new_layer_selected()
{
	return new_layer_selected;
}

gboolean get_project_active()
{
	return project_active;
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

void set_info_display(gboolean new_info_display)
{
	info_display = new_info_display;
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

void set_new_layer_selected(gboolean new_new_layer_selected)
{
	new_layer_selected = new_new_layer_selected;
}

void set_project_active(gboolean new_project_active)
{
	project_active = new_project_active;
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

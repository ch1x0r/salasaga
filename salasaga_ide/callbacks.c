/*
 * $Id$
 *
 * Flame Project: Source file for all callback functions
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
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// GTK includes
#include <gtk/gtk.h>

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#endif

// Flame Edit includes
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"
#include "externs.h"


// Function called when the drawing area is resized
gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	static gint			old_width = 0;


	// Check if the width of the drawing area has changed
	if (old_width != right_side->allocation.width)
	{
		// The width has been changed, so recalculate the zoom and redraw the drawing area
		zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
		old_width = right_side->allocation.width;

		// Resize the drawing area so it draws properly
		gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), working_width, working_height);

	}

	// Return FALSE to continue event propagation
	return FALSE;
}


// Callback function that updates the workspace when a new slide in the film strip is clicked
// fixme3: This function will likely need to be adapted to become the callback for the new film strip selection bits
gint film_strip_slide_clicked(GtkWidget *widget, GdkEvent *event, slide *clicked_slide)
{
	// Local variables
	gint 		slide_pos;
	guint		which_button;


	// Determine which mouse button was clicked
	which_button = event->button.button;

	// We're only interested in left mouse clicks at the moment
	switch (which_button)
	{
		case 1:
			// * Yes, it's a left click.  Update the current_slide to the clicked on slide's GList container, then redraw the timeline and workspace *

			// Get a pointer to the clicked on slide's GList
			slide_pos = g_list_index(slides, clicked_slide);
			current_slide = g_list_nth(slides, slide_pos);

			// Redraw the timeline
			draw_timeline();

			// Redraw the workspace
			draw_workspace();
			break;

		default:
			break;
	}

	return TRUE;
}


// Function called when the user chooses a new output resolution
gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	GString		*tmp_string;
	gchar		**strings;

	// Get the new output resolution
	tmp_string = g_string_new(NULL);
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(resolution_selector)));

	// Parse and store the new project output size
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
	strings = g_strsplit(tmp_string->str, "x", 2);
	output_width = atoi(strings[0]);
	output_height = atoi(strings[1]);

	// Free the memory allocated in this function
	g_strfreev(strings);
	g_string_free(tmp_string, TRUE);

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}


// Function called when the finish field in the timeline widget is edited
void timeline_edited_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	layer_data->finish_frame = atoi(new_value);

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_FINISH, atoi(new_value),
						-1);
}


// Function called when the name field in the timeline widget is edited
void timeline_edited_name(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	g_string_printf(layer_data->name, "%s", new_value);

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_NAME, layer_data->name->str,
						-1);
}


// Function called when the start field in the timeline widget is edited
void timeline_edited_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	layer_data->start_frame = atoi(new_value);

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_START, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the x offset finish field in the timeline widget is edited
void timeline_edited_x_offset_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:
			((layer_image *) layer_data->object_data)->x_offset_finish = atoi(new_value);
			break;

		case TYPE_HIGHLIGHT:
			((layer_highlight *) layer_data->object_data)->x_offset_finish = atoi(new_value);
			break;

		case TYPE_MOUSE_CURSOR:
			((layer_mouse *) layer_data->object_data)->x_offset_finish = atoi(new_value);
			break;

		case TYPE_TEXT:
			((layer_text *) layer_data->object_data)->x_offset_finish = atoi(new_value);
			break;

		default:
			display_warning("ED30: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_X_OFF_FINISH, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the x offset start field in the timeline widget is edited
void timeline_edited_x_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:
			((layer_image *) layer_data->object_data)->x_offset_start = atoi(new_value);
			break;

		case TYPE_HIGHLIGHT:
			((layer_highlight *) layer_data->object_data)->x_offset_start = atoi(new_value);
			break;

		case TYPE_MOUSE_CURSOR:
			((layer_mouse *) layer_data->object_data)->x_offset_start = atoi(new_value);
			break;

		case TYPE_TEXT:
			((layer_text *) layer_data->object_data)->x_offset_start = atoi(new_value);
			break;

		default:
			display_warning("ED59: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_X_OFF_START, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the y offset finish field in the timeline widget is edited
void timeline_edited_y_offset_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:
			((layer_image *) layer_data->object_data)->y_offset_finish = atoi(new_value);
			break;

		case TYPE_HIGHLIGHT:
			((layer_highlight *) layer_data->object_data)->y_offset_finish = atoi(new_value);
			break;

		case TYPE_MOUSE_CURSOR:
			((layer_mouse *) layer_data->object_data)->y_offset_finish = atoi(new_value);
			break;

		case TYPE_TEXT:
			((layer_text *) layer_data->object_data)->y_offset_finish = atoi(new_value);
			break;

		default:
			display_warning("ED31: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_Y_OFF_FINISH, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the y offset start field in the timeline widget is edited
void timeline_edited_y_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data)
{
	// Local variables
	GList				*layer_pointer;
	layer				*layer_data;


	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its value changed
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// Update the layer with the new value
	switch (layer_data->object_type)
	{
		case TYPE_EMPTY:
			// Nothing to do here
			break;

		case TYPE_GDK_PIXBUF:
			((layer_image *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_HIGHLIGHT:
			((layer_highlight *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_MOUSE_CURSOR:
			((layer_mouse *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		case TYPE_TEXT:
			((layer_text *) layer_data->object_data)->y_offset_start = atoi(new_value);
			break;

		default:
			display_warning("ED60: Unknown layer type\n");
	}

	// Update the timeline widget with the new value too
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, layer_data->row_iter,
						TIMELINE_Y_OFF_START, atoi(new_value),
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Detect when the user presses the mouse button on the drawing area
gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	guint				num_layers;
	guint				count_int;

	GList				*collision_list = NULL;
	slide				*current_slide_data;	// Alias to make things easier
	GtkWidget			*list_widget;			// Alias to the timeline widget to make things easier
	guint				num_collisions;
	gboolean			selection_hit;			// Status toggle
	guint				selected_row;			// Holds the number of the row that is selected

	guint				tmp_int;				// Temporary integer
	GtkTreeViewColumn	*tmp_column;			// Temporary column
	GString				*tmp_gstring;			// Temporary GString
	GtkTreePath			*tmp_path;				// Temporary path


	// Only do this function if we have a backing store available
	if (NULL == backing_store)
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;
	selection_hit = FALSE;

	// Check for primary mouse button click
	if (1 != event->button)
	{
		// Not a primary mouse click, so return
		return TRUE;
	}

	// Reset the mouse drag toggle
	mouse_dragging = FALSE;

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();

		return TRUE;
	}

	// Check if this was a triple mouse click.  If it was, ignore it
	if (GDK_3BUTTON_PRESS == event->type)
	{
		return TRUE;
	}

	// * Do collision detection here to determine if the user has clicked on a layer's object *
	tmp_gstring = g_string_new(NULL);
	collision_list = detect_collisions(collision_list, event->x, event->y);
	if (NULL == collision_list)
	{
		// If there was no collision, then select the background layer
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		num_layers = g_list_length(current_slide_data->layers);
		g_string_printf(tmp_gstring, "%d", num_layers - 1);
		tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
		gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

		// Reset the stored mouse coordinates
		stored_x = -1;
		stored_y = -1;

		// Free the memory allocated during the collision detection
		g_string_free(tmp_gstring, TRUE);
		g_list_free(collision_list);
		collision_list = NULL;

		return TRUE;
	}

	// * To get here there must have been at least one collision *

	// Save the mouse coordinates
	stored_x = event->x;
	stored_y = event->y;

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));

	// Is the presently selected layer in the collision list?
	collision_list = g_list_first(collision_list);
	num_collisions = g_list_length(collision_list);
	for (count_int = 0; count_int < num_collisions; count_int++)
	{
		collision_list = g_list_first(collision_list);
		collision_list = g_list_nth(collision_list, count_int);

		current_slide_data->layers = g_list_first(current_slide_data->layers);

		tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
		if (tmp_int == selected_row)
		{
			// * Yes, the presently selected row is in the collision list *
			selection_hit = TRUE;

			// If there are further collisions in the collision list, we just move the timeline row selection
			// to the next collision row, otherwise we wrap to the collision row
			if (count_int == num_collisions - 1)
			{
				// We're at the bottom of the collision hit list, so we wrap back to the start
				collision_list = g_list_first(collision_list);
				current_slide_data->layers = g_list_first(current_slide_data->layers);
				tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
				g_string_printf(tmp_gstring, "%d", tmp_int);
				tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
				gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

				// Free the memory allocated during the collision detection
				g_string_free(tmp_gstring, TRUE);
				g_list_free(collision_list);
				collision_list = NULL;

				return TRUE;
			} else
			{
				// There are further collision hits, so we just move the selection to the next one
				collision_list = g_list_next(collision_list);
				current_slide_data->layers = g_list_first(current_slide_data->layers);
				tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
				g_string_printf(tmp_gstring, "%d", tmp_int);
				tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
				gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

				// Draw a bounding box around the selected object
				// fixme4: Better to do this later, driven from changes in the timelines selected row
//				draw_bounding_box(widget, ((boundary_box *) collision_list->data)->region_ptr);

				// Free the memory allocated during the collision detection
				g_string_free(tmp_gstring, TRUE);
				g_list_free(collision_list);
				collision_list = NULL;

				return TRUE;
			}
		}
	}

	// * To get here, the presently selected layer wasn't in the collision list *

	// The presently selected row is not in the collision list, so move the selection row to the first collision
	collision_list = g_list_first(collision_list);
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	tmp_int = g_list_position(current_slide_data->layers, ((boundary_box *) collision_list->data)->layer_ptr);
	g_string_printf(tmp_gstring, "%d", tmp_int);
	tmp_path = gtk_tree_path_new_from_string(tmp_gstring->str);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);

	// Free the memory allocated during the collision detection
	g_string_free(tmp_gstring, TRUE);
	g_list_free(collision_list);
	collision_list = NULL;

	return TRUE;
}


// Detect when the user presses the mouse button on the drawing area
gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint					mouse_x;
	gint					mouse_y;
	gint					present_x;
	gint					present_y;
	gint					width;
	gint					height;
	layer				*layer_data;

	slide				*current_slide_data;			// Alias to make things easier
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	gboolean				selection_hit;				// Status toggle
	gchar				*selected_row;				// Holds the number of the row that is selected

	gdouble				tmp_double;					// Temporary double
	GtkTreeViewColumn	*tmp_column;					// Temporary column
	GString				*tmp_gstring;				// Temporary GString
	GtkTreePath			*tmp_path;					// Temporary path


	// Only do this function if we have a backing store available
	if (NULL == backing_store)
	{
		return TRUE;
	}

	// If this release isn't for the end of a mouse drag operation, we ignore it
	if (FALSE == mouse_dragging)
	{
		return TRUE;
	}

	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;
	tmp_gstring = g_string_new(NULL);
	selection_hit = FALSE;

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Check for primary mouse button release
	if (1 == event->button)
	{
		// Determine which layer is selected in the timeline
		tmp_path = gtk_tree_path_new();
		tmp_column = gtk_tree_view_column_new();
		gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
		selected_row = gtk_tree_path_to_string(tmp_path);

		// Get its present X and Y offsets
		current_slide_data->layers = g_list_first(current_slide_data->layers);
		layer_data = g_list_nth_data(current_slide_data->layers, atoi(selected_row));
		switch (layer_data->object_type)
		{
			case TYPE_EMPTY:
				// We can't drag an empty layer, so reset things and return
				mouse_dragging = FALSE;
				stored_x = -1;
				stored_y = -1;
				return TRUE;

			case TYPE_HIGHLIGHT:
				present_x = ((layer_highlight *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_highlight *) layer_data->object_data)->y_offset_start;
				width = ((layer_highlight *) layer_data->object_data)->width;
				height = ((layer_highlight *) layer_data->object_data)->height;
				break;

			case TYPE_GDK_PIXBUF:
				// If this is the background layer, then we ignore it
				if (0 == g_ascii_strncasecmp(layer_data->name->str, "Background", 10))
				{
					mouse_dragging = FALSE;
					stored_x = -1;
					stored_y = -1;
					return TRUE;
				}

				// No it's not, so process it
				present_x = ((layer_image *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_image *) layer_data->object_data)->y_offset_start;
				width = ((layer_image *) layer_data->object_data)->width;
				height = ((layer_image *) layer_data->object_data)->height;
				break;

			case TYPE_TEXT:
				present_x = ((layer_text *) layer_data->object_data)->x_offset_start;
				present_y = ((layer_text *) layer_data->object_data)->y_offset_start;
				width = ((layer_text *) layer_data->object_data)->rendered_width;
				height = ((layer_text *) layer_data->object_data)->rendered_height;
				break;

			default:
				display_warning("ED32: Unknown layer type\n");
				return TRUE;  // Unknown layer type, so no idea how to extract the needed data for the next code
		}

		// Work out and set the new X offset for the layer object
		// fixme3: This needs to be a lot more accurate
		tmp_double = mouse_x - stored_x;
		tmp_double = (tmp_double * zoom) / 100;
		tmp_double = present_x + tmp_double;
		if (0 >= tmp_double)
		{
			tmp_double = 0;
		}
		if (tmp_double + width >= project_width)
		{
			tmp_double = project_width - width;
		}
		g_string_printf(tmp_gstring, "%.0f", tmp_double);
		timeline_edited_x_offset_start(NULL, selected_row, tmp_gstring->str, NULL);

		// Work out and set the new Y offset for the layer object
		// fixme3: This needs to be a lot more accurate
		tmp_double = mouse_y - stored_y;
		tmp_double = (tmp_double * zoom) / 100;
		tmp_double = present_y + tmp_double;
		if (0 >= tmp_double)
		{
			tmp_double = 0;
		}
		if (tmp_double + height >= project_height)
		{
			tmp_double = project_height - height;
		}
		g_string_printf(tmp_gstring, "%.0f", tmp_double);
		timeline_edited_y_offset_start(NULL, selected_row, tmp_gstring->str, NULL);

		// Reset the mouse drag switch and related info
		mouse_dragging = FALSE;
		stored_x = -1;
		stored_y = -1;

		// Free the allocated memory
		g_free(selected_row);
	}

	return TRUE;
}


// Redraw the working area from the backing store
gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	// Only do this function if we have a backing store available
	if (NULL == backing_store)
	{
		return FALSE;
	}

	// Draw the invalidated backing store area onto the working area
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(widget->style->fg_gc[GTK_WIDGET_STATE(widget)]),
		GDK_PIXMAP(backing_store),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	return FALSE;
}


// Detect when the user moves a pressed down mouse button on the drawing area
gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	gint					mouse_x;
	gint					mouse_y;


	// If we're already aware of a mouse drag operation going on, then return
	if (TRUE == mouse_dragging)
	{
		return TRUE;
	}

	// * To get here, this must be the first time we've heard of this particular mouse drag *

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Check if the primary mouse button is down
	if (GDK_BUTTON1_MASK & button_state)
	{
		// It is, so we take notice of the mouse drag operation and return
		mouse_dragging = TRUE;
	}

	return TRUE;
}


// Function called when the user chooses a new zoom level
gint zoom_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	gint32		tmp_int;
	GString		*tmp_string;

	// Get the new zoom level
	tmp_string = g_string_new(NULL);
	g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));

	// Parse and store the new zoom level
	tmp_int = g_ascii_strncasecmp(tmp_string->str, "F", 1);
	if (0 == tmp_int)
	{
		// "Fit to width" is selected, so work out a new zoom level by figuring out how much space the widget really has
		//  (Look at the alloation of it's parent widget)
		//  Reduce the width calculated by 24 pixels (guessed) to give space for widget borders and such
		zoom = (guint) (((float) (right_side->allocation.width - 24) / (float) project_width) * 100);
	} else
	{
		tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
		zoom = atoi(tmp_string->str);
	}

	// Free the memory allocated in this function
	g_string_free(tmp_string, TRUE);

	// Calculate and set the display size of the working area
	working_width = (project_width * zoom) / 100;
	working_height = (project_height * zoom) / 100;

	// Redraw the workspace area
	draw_workspace();

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.11  2007/06/30 03:18:18  vapour
 * Began re-writing the film strip area to use a GtkListView widget instead of the hodge podge of event boxes, signal handlers, and other bits.
 *
 * Revision 1.10  2007/06/24 01:06:49  vapour
 * Removed the __sun workarounds for Solaris 10, as Solaris 11 should have the missing bits.
 *
 * Revision 1.9  2006/12/26 03:56:14  vapour
 * Removing unnecessary xml includes.
 *
 * Revision 1.8  2006/07/30 04:58:50  vapour
 * Removed the ming includes, commented out the code needing ming, and added a notice to the user if they try and export flash that it's not  going to work as the code is being rewritten.
 *
 * Revision 1.7  2006/07/09 11:43:48  vapour
 * + Editing mouse pointer values in the timeline view now works.
 * + Reordered the functions alphabetically.
 *
 * Revision 1.6  2006/06/12 03:50:43  vapour
 * Updated many of the warning messages to go through the display_warning function.
 *
 * Revision 1.5  2006/04/22 08:36:54  vapour
 * + Replaced the text string display in the timeline (layer) widget area, with the x and y finish positions.
 * + Updated the entire project to use the word "finish" consistently, instead of "final".
 *
 * Revision 1.4  2006/04/21 17:46:13  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.3  2006/04/18 18:00:52  vapour
 * Tweaks to allow compilation to succeed on both Windows and Solaris as well.
 * On Windows, the app will fire up as it only really required changes to not use GConf.
 * On Solaris however, a lot of stuff needed to be disabled, so it core dumps right away, prior to even displaying a window.
 * However, this *is* progress of a sort. :)
 *
 * Revision 1.2  2006/04/16 06:05:28  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

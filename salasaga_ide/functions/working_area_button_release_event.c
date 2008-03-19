/*
 * $Id$
 *
 * Salasaga: Detect when the user presses the mouse button on the drawing area 
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
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "layer_new_highlight_inner.h"
#include "layer_new_image_inner.h"
#include "layer_new_mouse_inner.h"
#include "layer_new_text_inner.h"
#include "timeline_edited_x_offset_start.h"
#include "timeline_edited_y_offset_start.h"


gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	GdkModifierType		button_state;
	slide				*current_slide_data;		// Alias to make things easier
	gint				height;
	layer				*layer_data;
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	gint				mouse_x;
	gint				mouse_y;
	gint				present_x;
	gint				present_y;
	gint				project_x_position;			// X position in the project image
	gint				project_y_position;			// Y position in the project image
	gfloat				scaled_height_ratio;		// Used to calculate a vertical scaling ratio 
	gfloat				scaled_width_ratio;			// Used to calculate a horizontal scaling ratio
	gboolean			selection_hit;				// Status toggle
	gchar				*selected_row;				// Holds the number of the row that is selected
	gint				width;

	gdouble				tmp_double;					// Temporary double
	GtkTreeViewColumn	*tmp_column;				// Temporary column
	GString				*tmp_gstring;				// Temporary GString
	GtkTreePath			*tmp_path;					// Temporary path


	// Only do this function if we have a front store available
	if (NULL == front_store)
	{
		return TRUE;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// Calculate the height and width scaling values for the main drawing area at its present size
	scaled_height_ratio = (gfloat) project_height / (gfloat) main_drawing_area->allocation.height;
	scaled_width_ratio = (gfloat) project_width / (gfloat) main_drawing_area->allocation.width;

	// Work out where the mouse is positioned
	project_x_position = mouse_x * scaled_width_ratio;
	project_y_position = mouse_y * scaled_height_ratio;

	// Code to run if this button release is for the creation of a new highlight layer
	if (TYPE_HIGHLIGHT == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_highlight_inner(mouse_x, mouse_y);  // Note that this one gets the unscaled mouse positions

		return TRUE;
	}

	// Code to run if this button release is for the creation of a new image layer
	if (TYPE_GDK_PIXBUF == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_image_inner(project_x_position, project_y_position);

		return TRUE;
	}

	// Code to run if this button release is for the creation of a new mouse layer
	if (TYPE_MOUSE_CURSOR == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_mouse_inner(project_x_position, project_y_position);

		return TRUE;
	}

	// Code to run if this button release is for the creation of a new text layer
	if (TYPE_TEXT == new_layer_selected)
	{
		// Reset the new layer toggle
		new_layer_selected = TYPE_NONE;

		// Ask the user for the rest of the required details
		layer_new_text_inner(project_x_position, project_y_position);

		return TRUE;
	}

	// If this release matches the end of a mouse drag operation, we process it
	if (TRUE == mouse_dragging)
	{

		// Initialise some things
		current_slide_data = current_slide->data;
		list_widget = current_slide_data->timeline_widget;
		tmp_gstring = g_string_new(NULL);
		selection_hit = FALSE;
	
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
					present_x = layer_data->x_offset_start;
					present_y = layer_data->y_offset_start;
					width = ((layer_highlight *) layer_data->object_data)->width;
					height = ((layer_highlight *) layer_data->object_data)->height;
					break;
	
				case TYPE_GDK_PIXBUF:
					// If this is the background layer, then we ignore it
					if (TRUE == layer_data->background)
					{
						mouse_dragging = FALSE;
						stored_x = -1;
						stored_y = -1;
						return TRUE;
					}
	
					// No it's not, so process it
					present_x = layer_data->x_offset_start;
					present_y = layer_data->y_offset_start;
					width = ((layer_image *) layer_data->object_data)->width;
					height = ((layer_image *) layer_data->object_data)->height;
					break;
	
				case TYPE_MOUSE_CURSOR:
	
					// No it's not, so process it
					present_x = layer_data->x_offset_start;
					present_y = layer_data->y_offset_start;
					width = ((layer_mouse *) layer_data->object_data)->width;
					height = ((layer_mouse *) layer_data->object_data)->height;
					break;
	
				case TYPE_TEXT:
					present_x = layer_data->x_offset_start;
					present_y = layer_data->y_offset_start;
					width = ((layer_text *) layer_data->object_data)->rendered_width;
					height = ((layer_text *) layer_data->object_data)->rendered_height;
					break;
	
				default:
					display_warning("Error ED32: Unknown layer type\n");
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
	}

	return TRUE;
}

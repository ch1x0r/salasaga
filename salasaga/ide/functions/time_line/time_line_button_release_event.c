/*
 * $Id$
 *
 * Salasaga: Callback function for when the user releases the mouse button on the time line widget
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_duplicate.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_layer_start_and_end_points.h"
#include "time_line.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_layer_duration.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_layer_name.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_invalidate_layer_area.h"
#include "time_line_internal_redraw_layer_bg.h"
#include "draw_timeline.h"
void time_line_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	layer				*background_layer_data;		// Data for the background layer
	GdkModifierType		button_state;				// Mouse button states
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GtkAllocation		area;						// Area covered by an individual guide line
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GString				*message;					// Used to construct message strings
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	TimeLinePrivate		*priv;
	gboolean			return_code_gbool;			// Receives boolean return codes
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	TimeLine			*this_time_line;
	GList				*tmp_glist = NULL;			// Is given a list of child widgets, if any exist
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// Safety check
	if (NULL == widget)
	{
		return;
	}

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// It's probably the child of the called widget that we need to get data from
	if (FALSE == IS_TIME_LINE(widget))
	{
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(widget));
		if (NULL == tmp_glist)
			return;
		if (FALSE == IS_TIME_LINE(tmp_glist->data))
		{

			g_list_free(tmp_glist);
			return;
		}

		// The child is the TimeLine widget
		this_time_line = TIME_LINE(tmp_glist->data);
	} else
	{
		// This is a time line widget
		this_time_line = TIME_LINE(widget);
	}

	// Initialisation
	message = g_string_new(NULL);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Check if this button release is within the vertical range of the adjustment buttons
	if ((ADJUSTMENTS_Y <= mouse_y) && (ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= mouse_y)
	{
		// * It's in the correct range *

		// Check if this button release is for the minus button
		if ((ADJUSTMENTS_X <= mouse_x) && ((ADJUSTMENTS_X + ADJUSTMENTS_SIZE) >= mouse_x))
		{
			// Sanity check
			if (time_line_get_pixels_per_second() >= 96)
			{
				// We're already at the acceptable scaling limit, so beep then return
				gdk_beep();
				g_list_free(tmp_glist);
				return;
			}

			// Adjust the number of pixels per second
			time_line_set_pixels_per_second(time_line_get_pixels_per_second() * 2);
			g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
			priv->cached_bg_image = NULL;

			// Regenerate the timeline images with the new pixel scale
			return_code_gbool = time_line_internal_initialise_bg_image(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED361: %s", _("Error"), _("Couldn't recreate time line background image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			return_code_gbool = time_line_internal_initialise_display_buffer(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED362: %s", _("Error"), _("Couldn't recreate time line display buffer."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			return_code_gbool = time_line_internal_draw_layer_info(priv);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED363: %s", _("Error"), _("Couldn't redraw the time line layer information."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			area.x = 0;
			area.y = 0;
			area.width = widget->allocation.width;
			area.height = widget->allocation.height;
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}

		// Check if this button release is for the plus button
		if ((ADJUSTMENTS_X + 15 <= mouse_x) && ((ADJUSTMENTS_X + 15 + ADJUSTMENTS_SIZE) >= mouse_x))
		{
			// Sanity check
			if (time_line_get_pixels_per_second() <= 24)
			{
				// We're already at the acceptable scaling limit, so beep then return
				gdk_beep();
				g_list_free(tmp_glist);
				return;
			}

			// Adjust the number of pixels per second
			time_line_set_pixels_per_second(time_line_get_pixels_per_second() / 2);
			g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
			priv->cached_bg_image = NULL;

			// Regenerate the timeline images with the new pixel scale
			return_code_gbool = time_line_internal_initialise_bg_image(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED364: %s", _("Error"), _("Couldn't recreate time line background image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			return_code_gbool = time_line_internal_initialise_display_buffer(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED365: %s", _("Error"), _("Couldn't recreate time line display buffer."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			return_code_gbool = time_line_internal_draw_layer_info(priv);
			if (FALSE == return_code_gbool)
			{
				g_string_printf(message, "%s ED366: %s", _("Error"), _("Couldn't redraw the time line layer information."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_list_free(tmp_glist);
				return;
			}
			area.x = 0;
			area.y = 0;
			area.width = widget->allocation.width;
			area.height = widget->allocation.height;
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}
	}

	// Remove guide lines from the widget
	area.x = priv->guide_line_start;
	area.y = 0;
	area.height = GTK_WIDGET(this_time_line)->allocation.height;
	area.width = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
	area.x = priv->guide_line_end;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

	// Check if this mouse release matches a cursor drag
	if (TRUE == priv->cursor_drag_active)
	{
		// Note that the cursor drag has finished
		priv->cursor_drag_active = FALSE;
	}

	// Check if this mouse release matches a resize
	if (RESIZE_NONE != priv->resize_type)
	{
		// Note that the resize has finished
		priv->resize_type = RESIZE_NONE;

		// Mark that there are unsaved changes
		set_changes_made(TRUE);

		// Remove the resize guideline from the widget
		area.x = priv->guide_line_resize;
		gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		priv->guide_line_resize = 0;

		// Create pointers to things we're working with
		this_slide_data = get_current_slide_data();
		layer_pointer = this_slide_data->layers;
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);

		// Calculate the end time of the layer (in seconds)
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;

		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
			// The new slide duration is longer than the old one, so update the slide and background layer to match
			this_slide_data->duration = priv->stored_slide_duration = end_time;
			end_row = this_slide_data->num_layers - 1;
			background_layer_data = g_list_nth_data(layer_pointer, end_row);
			background_layer_data->duration = priv->stored_slide_duration;

			// Refresh the time line display of the background layer
			time_line_internal_redraw_layer_bg(priv, end_row);
			time_line_internal_draw_layer_name(priv, end_row);
			time_line_internal_draw_layer_duration(priv, end_row);
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), end_row);
		}

		// Use the status bar to communicate the resize has completed
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Resize completed"));
		gdk_flush();

		// Store the undo item created in the button click event handler function
		undo_item_data = time_line_get_undo_item();
		undo_item_data->position_new = priv->selected_layer_num;
		undo_item_data->layer_data_new = layer_duplicate(this_layer_data);
		undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);
		draw_timeline();
	}

	// Check if this mouse release matches a drag
	if (TRUE == priv->drag_active)
	{
		// Note that the drag has finished
		priv->drag_active = FALSE;

		// Mark that there are unsaved changes
		set_changes_made(TRUE);

		// Create pointers to things we're working with
		this_slide_data = get_current_slide_data();
		layer_pointer = this_slide_data->layers;
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);

		// Calculate the end time of the layer (in seconds)
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;

		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
			// The new slide duration is longer than the old one, so update the slide and background layer to match
			this_slide_data->duration = priv->stored_slide_duration = end_time;
			end_row = this_slide_data->num_layers - 1;
			background_layer_data = g_list_nth_data(layer_pointer, end_row);
			background_layer_data->duration = priv->stored_slide_duration;

			// Refresh the time line display of the background layer
			time_line_internal_redraw_layer_bg(priv, end_row);
			time_line_internal_draw_layer_name(priv, end_row);
			time_line_internal_draw_layer_duration(priv, end_row);
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), end_row);
		}

		// Use the status bar to communicate the drag has completed
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Drag completed"));
		gdk_flush();

		// Store the undo item created in the button click event handler function
		undo_item_data = time_line_get_undo_item();
		undo_item_data->position_new = priv->selected_layer_num;
		undo_item_data->layer_data_new = layer_duplicate(this_layer_data);
		undo_history_add_item(UNDO_CHANGE_LAYER, undo_item_data, TRUE);
		// For Updating the scoll bars, just redraw the same timeline / only if it is a drag event
		draw_timeline();
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);

	// Recreate the film strip thumbnail
	film_strip_create_thumbnail(get_current_slide_data());

	// Draw the start and end points for the layer
	draw_layer_start_and_end_points();


	// Free the memory allocated in this function
	g_list_free(tmp_glist);
}


















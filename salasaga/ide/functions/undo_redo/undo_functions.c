/*
 * $Id$
 *
 * Salasaga: Functions for Salasaga's undo and redo history
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../layer/layer_free.h"
#include "../menu/menu_enable.h"
#include "../working_area/draw_workspace.h"


// Variables common to several undo/redo functions
static gboolean			just_did_redo = FALSE;		// Tracks if the last action was Edit -> Redo
static GList			*undo_history = NULL;
static gint				undo_cursor = -1;			// Tracks the present position in the undo history list


// Add an undo item to the undo history
gint undo_add_item(gint undo_type, gpointer undo_data)
{
	// Local variables
	gint				loop_counter;				// Simple counter used in loops
	undo_history_item	*new_item = NULL;
	gint				num_items;					// The number of items in the undo history
	GList				*this_item;


	// Remove any undo history items that are after the present cursor position
	num_items = g_list_length(undo_history);
	for (loop_counter = num_items; loop_counter > (undo_cursor + 1); loop_counter--)
	{
		// Remove the undo history item we're pointing to
		this_item = g_list_nth(undo_history, loop_counter - 1);

		// Free the layer memory used by link history items
		// fixme3: Probably needs to be coded for specific undo history types
//		layer_free(this_item->data);

		// Remove the link history items no longer needed
		undo_history = g_list_delete_link(undo_history, this_item);
	}

	// Create the undo history item
	new_item = g_new0(undo_history_item, 1);
	new_item->undo_type = undo_type;
	new_item->undo_data = undo_data;

	// Add the new undo item to the undo list
	undo_history = g_list_append(undo_history, new_item);
	undo_cursor++;

	// Unset the flag that indicates the redo function was just run
	just_did_redo = FALSE;

	// Enable the Edit -> Undo menu bar option
	menu_enable(_("/Edit/Undo"), TRUE);

	// Disable the Edit -> Redo menu bar option
	menu_enable(_("/Edit/Redo"), FALSE);

	return TRUE;
};


// Undo the last item in the undo history
gint undo_last_history_item(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to layer items
	GString				*message;					// Temporary string used for message creation
	undo_history_data	*new_undo_data;
	gint				num_items;					// The number of items in the undo history
	slide				*slide_data;
	undo_history_item	*undo_item;					// Points to the undo history item we're working with
	undo_history_data	*undo_data;
	gint				undo_type;


	// Initialisation
	message = g_string_new(NULL);

	// Determine which undo item type we're being called with, as each type is handled differently
	undo_item = g_list_nth_data(undo_history, undo_cursor);
	undo_data = undo_item->undo_data;
	undo_type = undo_item->undo_type;
	switch (undo_type)
	{
		case UNDO_CHANGE_LAYER:

			// * We're undoing a layer change *

			// Point to the layer we're going to change
			slide_data = undo_data->slide_data;
			layer_pointer = g_list_nth(slide_data->layers, undo_data->old_layer_position);

			// Save the present layer state in the undo history if it's not already there.  This
			// is so we can Edit -> Redo to it later on if asked
			num_items = g_list_length(undo_history);
			if ((FALSE == just_did_redo) && ((undo_cursor + 1) == num_items))
			{
				new_undo_data = g_new0(undo_history_data, 1);
				new_undo_data->layer_pointer = layer_pointer->data;
				new_undo_data->old_layer_position = undo_data->old_layer_position;
				new_undo_data->slide_data = slide_data;
				undo_add_item(UNDO_CHANGE_LAYER, new_undo_data);
				undo_cursor--;
			}

			// Update the slide to use the version of the layer stored in the undo history
			layer_pointer->data = undo_data->layer_pointer;

			break;

		default:
			// Unknown type of undo item.  Let the user know then return
			g_string_printf(message, "%s ED459: %s", _("Error"), _("Programming error.  The Undo/Redo functions were called with an unknown undo type."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
	}

	// Move the undo cursor back one item
	undo_cursor--;

	// Enable the Edit -> Redo option
	menu_enable(_("/Edit/Redo"), TRUE);

	// If we're at the start of the undo history we can't undo any further
	if (0 == undo_cursor)
	{
		menu_enable(_("/Edit/Undo"), FALSE);
	}

	// Redraw the workspace
	draw_workspace();

	// Tell (force) the window system to redraw the working area *immediately*
	gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

	// Recreate the slide thumbnail
	film_strip_create_thumbnail((slide *) current_slide->data);

	// Set the changes made variable
	changes_made = TRUE;

	// Use the status bar to give further feedback to the user
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Last action undone"));
	gdk_flush();

	return TRUE;
}


// Redo the last item in the undo history
gint undo_next_history_item(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to layer items
	GString				*message;					// Temporary string used for message creation
	gint				num_items;					// The number of items in the undo history
	slide				*slide_data;
	undo_history_data	*undo_data;
	undo_history_item	*undo_item;					// Points to the undo history item we're working with
	gint				undo_type;


	// Initialisation
	message = g_string_new(NULL);

	// Determine which undo item type we're being called with, as each type is handled differently
	num_items = g_list_length(undo_history);
	undo_item = g_list_nth_data(undo_history, undo_cursor + 2);
	undo_data = undo_item->undo_data;
	undo_type = undo_item->undo_type;
	switch (undo_type)
	{
		case UNDO_CHANGE_LAYER:
			// We're redoing a layer change, so we update the slide to use the new version of the layer
			slide_data = undo_data->slide_data;
			layer_pointer = g_list_nth(slide_data->layers, undo_data->old_layer_position);
			layer_pointer->data = undo_data->layer_pointer;
			break;

		default:
			// Unknown type of undo item.  Let the user know then return
			g_string_printf(message, "%s ED459: %s", _("Error"), _("Programming error.  The Undo/Redo functions were called with an unknown undo type."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
	}

	// Move the undo cursor back one item
	undo_cursor++;

	// Set the flag that indicates the redo function was just run
	just_did_redo = TRUE;

	// Enable the Edit -> Undo option
	menu_enable(_("/Edit/Undo"), TRUE);

	// If we're at the end of the undo history we can't redo any further
	num_items = g_list_length(undo_history);
	if (undo_cursor >= (num_items - 2))
	{
		menu_enable(_("/Edit/Redo"), FALSE);
	}

	// Redraw the workspace
	draw_workspace();

	// Tell (force) the window system to redraw the working area *immediately*
	gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

	// Recreate the slide thumbnail
	film_strip_create_thumbnail((slide *) current_slide->data);

	// Set the changes made variable
	changes_made = TRUE;

	// Use the status bar to give further feedback to the user
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Last action redone"));
	gdk_flush();

	return TRUE;
}

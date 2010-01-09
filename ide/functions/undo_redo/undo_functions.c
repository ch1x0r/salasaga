/*
 * $Id$
 *
 * Salasaga:
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
#include "../menu/menu_enable.h"
#include "../working_area/draw_workspace.h"


// Variables common to several undo/redo functions
static GList			*undo_history = NULL;
gint					undo_cursor = 0;		// Tracks the present position in the undo history list


// Add an undo item to the undo history
gint undo_add_item(gint undo_type, gpointer undo_data)
{
	// Local variables
	undo_item			*new_item = NULL;


	// Remove any undo history items that are after the present cursor position
// Fixme1:  Needs to be written


	// Create the undo item
	new_item = g_new0(undo_item, 1);
	new_item->undo_type = undo_type;
	new_item->undo_data = undo_data;

	// Add the new undo item to the undo list
	undo_history = g_list_append(undo_history, new_item);
	undo_cursor++;

	// Enable the Edit -> Undo menu bar option
	menu_enable(_("/Edit/Undo"), TRUE);

	return TRUE;
};


// Undo the last item in the undo history
gint undo_last_history_item(void)
{
	// Local variables
	layer				*layer_pointer;			// Points to layer items in undo history
	GString				*message;				// Temporary string used for message creation
	undo_data			*undo_data;
	gint				undo_type;


	// Initialisation
	message = g_string_new(NULL);

	// Determine which undo item type we're being called with, as each type is handled differently
	undo_history = g_list_last(undo_history);
	undo_data = ((undo_item *) undo_history->data)->undo_data;
	undo_type = ((undo_item *) undo_history->data)->undo_type;
	switch (undo_type)
	{
		case UNDO_CHANGE_HIGHLIGHT_SIZE:

			// We're working with a change in highlight layer size
printf("The size of a highlight layer is being undone\n");

			// Change the highlight layer size to the value stored in the undo item
			layer_pointer = undo_data->layer_pointer;
			((layer_highlight *) layer_pointer->object_data)->height = undo_data->old_highlight_height;
			((layer_highlight *) layer_pointer->object_data)->width = undo_data->old_highlight_width;

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

	// If we're at the start of the undo history we can't undo any further
	if (0 == undo_cursor)
	{
		menu_enable(_("/Edit/Undo"), FALSE);
	}
/*
	// If the undo history list is empty we disable the Edit -> Undo and Edit -> Redo menu bar options
	undo_history = g_list_first(undo_history);
	if (0 == g_list_length(undo_history))
	{
		menu_enable(_("/Edit/Redo"), FALSE);
		menu_enable(_("/Edit/Undo"), FALSE);
	}
*/

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

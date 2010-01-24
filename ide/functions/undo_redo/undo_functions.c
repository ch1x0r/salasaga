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
#include "../draw_timeline.h"
#include "../dialog/display_warning.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../film_strip/regenerate_film_strip_thumbnails.h"
#include "../layer/layer_duplicate.h"
#include "../layer/layer_free.h"
#include "../menu/menu_enable.h"
#include "../working_area/draw_workspace.h"


// Variables common to several undo/redo functions
static GList			*undo_history = NULL;
static gint				undo_cursor = -1;			// Tracks the present position in the undo history list


// Add an undo item to the undo history
gint undo_history_add_item(gint undo_type, gpointer undo_data, gboolean remove_new)
{
	// Local variables
	gint				loop_counter;				// Simple counter used in loops
	undo_history_item	*new_item = NULL;
	gint				num_items;					// The number of items in the undo history
	GList				*this_item;


	// If requested, remove any undo history items that are after the present cursor position
	if (TRUE == remove_new)
	{
		num_items = g_list_length(undo_history);
		for (loop_counter = num_items; loop_counter > (undo_cursor + 1); loop_counter--)
		{
			// Remove the undo history item we're pointing to
			this_item = g_list_nth(undo_history, loop_counter - 1);

			// Free the layer memory used by link history items
			// fixme3: Probably needs to be coded for specific undo history types
//			layer_free(this_item->data);

			// Remove the link history items no longer needed
			undo_history = g_list_delete_link(undo_history, this_item);
		}
	}

	// Create the undo history item
	new_item = g_new0(undo_history_item, 1);
	new_item->undo_type = undo_type;
	new_item->undo_data = undo_data;

	// Add the new undo item to the undo list
	undo_history = g_list_append(undo_history, new_item);
	undo_cursor++;

	// Enable the Edit -> Undo menu bar option
	menu_enable(_("/Edit/Undo"), TRUE);

	// Disable the Edit -> Redo menu bar option
	menu_enable(_("/Edit/Redo"), FALSE);

	return TRUE;
};


// Remove all items from the undo history
gint undo_history_clear(void)
{
	// Local variables
	gint				loop_counter;				// Simple counter used in loops
	gint				num_items;					// The number of items in the undo history
	GList				*this_item;


	// * Remove the undo history items *
	num_items = g_list_length(undo_history);
	for (loop_counter = num_items; loop_counter > 0; loop_counter--)
	{
		// Remove the undo history item we're pointing to
		this_item = g_list_nth(undo_history, loop_counter - 1);

		// Free the layer memory used by link history items
		// fixme3: Probably needs to be coded for specific undo history types
//			layer_free(this_item->data);

		// Remove the link history items no longer needed
		undo_history = g_list_delete_link(undo_history, this_item);
	}

	// Reset the undo history variables
	undo_cursor = -1;
	undo_history = NULL;

	// Disable the Edit menu bar options
	menu_enable(_("/Edit/Redo"), FALSE);
	menu_enable(_("/Edit/Undo"), FALSE);

	return TRUE;
}


// Redo the last item in the undo history
gint undo_history_redo_item(void)
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

	// Determine which redo item type we're being called with, as each type is handled differently
	num_items = g_list_length(undo_history);
	undo_item = g_list_nth_data(undo_history, undo_cursor + 1);
	undo_data = undo_item->undo_data;
	undo_type = undo_item->undo_type;
	switch (undo_type)
	{
		case UNDO_CHANGE_LAYER:
printf("Redoing UNDO_CHANGE_LAYER\n");
			// * We're redoing a layer change, so we update the slide to use the new version of the layer *

			// Point to the layer we're going to change
			slide_data = undo_data->slide_data;
			layer_pointer = g_list_nth(slide_data->layers, undo_data->position_old);

			// Remove the "old" layer from the slide
			slide_data->layers = g_list_remove(slide_data->layers, layer_pointer->data);

			// Insert the "new" layer into the slide at the old position
			slide_data->layers = g_list_insert(slide_data->layers, undo_data->layer_data_new, undo_data->position_new);

			// Free the layer data in the existing layer
//			layer_free(layer_pointer->data);

			// Redraw the timeline area
			draw_timeline();

			break;

		case UNDO_DELETE_LAYER:
printf("Redoing UNDO_DELETE_LAYER\n");
			break;

		case UNDO_DELETE_SLIDE:
printf("Redoing UNDO_DELETE_SLIDE\n");
			break;

		case UNDO_INSERT_LAYER:
printf("Redoing UNDO_INSERT_LAYER\n");

			// * We're redoing the addition of a layer *

			// Insert the slide back in its original position
			slide_data = undo_data->slide_data;
			slide_data->layers = g_list_insert(slide_data->layers, undo_data->layer_data_new, undo_data->position_new);

			// Increment the counter of layers in the slide
			slide_data->num_layers++;

			// Redraw the timeline area
			draw_timeline();

			break;

		case UNDO_INSERT_SLIDE:
printf("Redoing UNDO_INSERT_SLIDE\n");

			// * We're redoing the addition of a slide *
			slides = g_list_append(slides, undo_data->slide_data);

			// Point to the redone slide
			slides = g_list_first(slides);
			current_slide = g_list_nth(slides, undo_data->position_new);

			// Redraw the film strip
			regenerate_film_strip_thumbnails();

			// Redraw the timeline area
			draw_timeline();

			// Redraw the workspace
			draw_workspace();

			break;

		case UNDO_REORDER_SLIDE:
printf("Redoing UNDO_REORDER_SLIDE\n");
			break;

		default:
			// Unknown type of undo item.  Let the user know then return
			g_string_printf(message, "%s ED460: %s", _("Error"), _("Programming error.  The Undo/Redo functions were called with an unknown undo type."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
	}

	// Move the undo cursor forward one item
	undo_cursor++;

	// Enable the Edit -> Undo option
	menu_enable(_("/Edit/Undo"), TRUE);

	// If we're at the end of the undo history we can't redo any further
	num_items = g_list_length(undo_history);
	if (undo_cursor >= (num_items - 1))
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
printf("Redo done\n");
	return TRUE;
}


// Undo the last item in the undo history
gint undo_history_undo_item(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to layer items
	GString				*message;					// Temporary string used for message creation
//	undo_history_data	*new_undo_data;
//	gint				num_items;					// The number of items in the undo history
	gint				num_slides;
	slide				*slide_data;
	gint				slide_position;
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
printf("UNDO_CHANGE_LAYER item undone\n");
			// * We're undoing a layer change *

			// Point to the layer we're going to change
			slide_data = undo_data->slide_data;
			layer_pointer = g_list_nth(slide_data->layers, undo_data->position_new);

			// Remove the "new" layer from the slide
			slide_data->layers = g_list_remove(slide_data->layers, layer_pointer->data);

			// Insert the "old" layer into the slide at the old position
			slide_data->layers = g_list_insert(slide_data->layers, undo_data->layer_data_old, undo_data->position_old);

			// Free the layer data in the existing layer
//			layer_free(layer_pointer->data);

			// Redraw the timeline area
			draw_timeline();

			// Redraw the workspace
			draw_workspace();

			// Tell (force) the window system to redraw the working area *immediately*
			gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

			// Recreate the slide thumbnail
			film_strip_create_thumbnail((slide *) current_slide->data);

			break;

		case UNDO_DELETE_LAYER:
printf("UNDO_DELETE_LAYER item undone\n");
			break;

		case UNDO_DELETE_SLIDE:
printf("UNDO_DELETE_SLIDE item undone\n");
			break;

		case UNDO_INSERT_LAYER:
printf("UNDO_INSERT_LAYER item undone\n");

			// * We're undoing the addition of a layer *

			// Point to the layer we're going to change
			slide_data = undo_data->slide_data;

			// Remove the layer from the slide
			layer_pointer = g_list_nth(slide_data->layers, undo_data->position_new);
			slide_data->layers = g_list_remove(slide_data->layers, layer_pointer->data);

			// Decrement the counter of layers in the slide
			slide_data->num_layers--;

			// Redraw the timeline area
			draw_timeline();

			// Redraw the workspace
			draw_workspace();

			// Tell (force) the window system to redraw the working area *immediately*
			gtk_widget_draw(GTK_WIDGET(main_drawing_area), &main_drawing_area->allocation);  // Yes, this is deprecated, but it *works*

			// Recreate the slide thumbnail
			film_strip_create_thumbnail((slide *) current_slide->data);

			break;

		case UNDO_INSERT_SLIDE:
printf("UNDO_INSERT_SLIDE item undone\n");

			// * We're undoing the addition of a slide *

			// Determine the position of the selected slide in the project
			slide_position = g_list_index(slides, undo_data->slide_data);

			// Remove the slide from the project
			slides = g_list_first(slides);
			slides = g_list_remove(slides, undo_data->slide_data);

			// * Update current_slide to point to the next slide *

			// If the new position of the slide is off the end of the slide list, we instead select the last slide
			slides = g_list_first(slides);
			num_slides = g_list_length(slides);
			if (slide_position >= num_slides)
			{
				current_slide = g_list_last(slides);
			} else
			{
				current_slide = g_list_nth(slides, slide_position);
			}

			// Redraw the film strip
			regenerate_film_strip_thumbnails();

			// Redraw the timeline area
			draw_timeline();

			// Redraw the workspace
			draw_workspace();

			break;

		case UNDO_REORDER_SLIDE:
printf("UNDO_REORDER_SLIDE item undone\n");

			// * We're undoing the reordering of a slide *

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
	if (-1 == undo_cursor)
	{
		menu_enable(_("/Edit/Undo"), FALSE);
	}

	// Set the changes made variable
	changes_made = TRUE;

	// Use the status bar to give further feedback to the user
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Last action undone"));
	gdk_flush();

	return TRUE;
}

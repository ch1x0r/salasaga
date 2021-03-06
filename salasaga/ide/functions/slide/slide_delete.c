/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Delete from the top menu
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../time_line/draw_timeline.h"
#include "../undo_redo/undo_functions.h"
#include "../working_area/draw_workspace.h"
#include "slide_free.h"


void slide_delete(void)
{
	// Local variables
	GString				*message;					// Used to construct message strings
	GtkTreePath			*new_path;					// Temporary path
	gint				num_slides;					// Number of slides in the whole slide list
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gint				slide_position;				// Which slide in the slide list we are deleting
	GtkTreeSelection	*film_strip_selector;		//
	GtkTreeIter			selection_iter;				//
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// Are we trying to delete the only slide in the project (not good)?
	set_slides(g_list_first(get_slides()));
	num_slides = g_list_length(get_slides());
	if (1 == num_slides)
	{
		// Yes we are, so give a warning message and don't delete the slide
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED462: %s", _("Error"), _("You must leave at least one slide in a project."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return;
	}

	// Determine where the slide is positioned in the project
	slide_position = g_list_position(get_slides(), get_current_slide());

	// Create and store the undo history item for this slide
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_new = NULL;  // NULL means not set
	undo_item_data->layer_data_old = NULL;  // NULL means not set
	undo_item_data->position_new = -1;  // -1 means not set
	undo_item_data->position_old = slide_position;
	undo_item_data->slide_data = get_current_slide_data();
	undo_history_add_item(UNDO_DELETE_SLIDE, undo_item_data, TRUE);

	// Remove the current slide from the slide list
	set_slides(g_list_remove_link(get_slides(), get_current_slide()));

	// Remove the current slide from the film strip
	film_strip_selector = gtk_tree_view_get_selection(GTK_TREE_VIEW(get_film_strip_view()));
	gtk_tree_selection_get_selected(film_strip_selector, NULL, &selection_iter);
	gtk_list_store_remove(GTK_LIST_STORE(get_film_strip_store()), &selection_iter);

	// * Update the currently selected slide to point to the next slide *
	if (num_slides == (slide_position + 1))
	{
		// If we're deleting the last slide, we'll need to point to the previous one instead
		slide_position--;
	}
	set_current_slide(g_list_nth(get_slides(), slide_position));

	// Select the next thumbnail in the film strip and scroll to display it
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Slide deleted"));
	gdk_flush();
}

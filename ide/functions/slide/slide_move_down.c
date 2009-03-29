/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Move down from the top menu 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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


void slide_move_down(void)
{
	// Local variables
	GtkTreeIter			from_iter, to_iter;
	GtkTreePath			*new_path;					// Temporary path
	GList				*next_slide;				// Pointer to the slide below
	gint				num_slides;					// The total number of slides
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gint				slide_position;				// Which slide in the slide list we are moving
	slide				*this_slide_data;			// Pointer to the data for this slide
	GString				*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	num_slides = g_list_length(slides);
	if (num_slides == (slide_position + 1))
	{
		// We can't move the bottom most slide any further down, so just return
		return;
	}

	// Swap the slides around
	this_slide_data = current_slide->data;
	next_slide = g_list_nth(slides, slide_position + 1);
	current_slide->data = next_slide->data;
	next_slide->data = this_slide_data;
	current_slide = next_slide;

	// Move the thumbnail down one position in the film strip list
	g_string_printf(tmp_gstring, "%u", slide_position);
	if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &from_iter, tmp_gstring->str))
	{
		g_string_printf(tmp_gstring, "%u", slide_position + 1);
		if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &to_iter, tmp_gstring->str))
		{
			gtk_list_store_move_after(GTK_LIST_STORE(film_strip_store), &from_iter, &to_iter);
		}
	}
	g_string_free(tmp_gstring, TRUE);

	// Scroll the film strip to show the new thumbnail position
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_from_indices(slide_position + 1, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Slide moved down"));
	gdk_flush();
}

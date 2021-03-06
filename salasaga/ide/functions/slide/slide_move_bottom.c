/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Move to bottom from the top menu 
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


void slide_move_bottom(void)
{
	// Local variables
	GtkTreeIter			new_iter;
	GtkTreePath			*new_path;					// Temporary path
	gint				num_slides;					// The total number of slides
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gint				slide_position;				// Which slide in the slide list we are moving
	slide				*this_slide_data;			// Pointer to the data for this slide
	GString				*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);	

	// Safety check
	set_slides(g_list_first(get_slides()));
	slide_position = g_list_position(get_slides(), get_current_slide());
	num_slides = g_list_length(get_slides());
	if (num_slides == (slide_position + 1))
	{
		// We can't move the bottom most slide any further down, so just return
		gdk_beep();
		return;
	}

	// Remove this slide from the slides list, then re-attach it on the end
	this_slide_data = get_current_slide_data();
	set_slides(g_list_remove(get_slides(), this_slide_data));
	set_slides(g_list_append(get_slides(), this_slide_data));
	set_current_slide(g_list_last(get_slides()));

	// Move the thumbnail to the end of the slides list
	g_string_printf(tmp_gstring, "%u", slide_position);
	if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(get_film_strip_store()), &new_iter, tmp_gstring->str))
	{
		gtk_list_store_move_before(GTK_LIST_STORE(get_film_strip_store()), &new_iter, NULL);
	}
	g_string_free(tmp_gstring, TRUE);

	// Scroll the film strip to show the new thumbnail position
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_from_indices(num_slides - 1, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, TRUE, 1.0, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Slide moved to bottom"));
	gdk_flush();
}

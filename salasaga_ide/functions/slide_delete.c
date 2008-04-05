/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Delete from the top menu 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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
#include "draw_timeline.h"
#include "draw_workspace.h"
#include "slide_free.h"


void slide_delete(void)
{
	// Local variables
	GtkTreePath			*new_path;					// Temporary path
	gint				num_slides;					// Number of slides in the whole slide list
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gint				slide_position;				// Which slide in the slide list we are deleting
	GtkTreeSelection	*film_strip_selector;
	GtkTreeIter			selection_iter;

	GList				*tmp_glist;					// Temporary GList


	// Are we trying to delete the only slide in the project (not good)?
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	if (1 == num_slides)
	{
		// Yes we are, so give a warning message and don't delete the slide
		display_warning("You must leave at least one slide in a project.\n");
		return;
	}

	// Remove the current slide from the slide list
	slide_position = g_list_position(slides, current_slide);
	tmp_glist = current_slide;
	slides = g_list_remove_link(slides, current_slide);

	// Remove the current slide from the film strip
	film_strip_selector = gtk_tree_view_get_selection(GTK_TREE_VIEW(film_strip_view));
	gtk_tree_selection_get_selected(film_strip_selector, NULL, &selection_iter);
	gtk_list_store_remove(GTK_LIST_STORE(film_strip_store), &selection_iter);

	// * Update the currently selected slide to point to the next slide *
	if (num_slides == (slide_position + 1))
	{
		// If we're deleting the last slide, we'll need to point to the previous one instead
		slide_position--;
	}
	current_slide = g_list_nth(slides, slide_position);

	// Select the next thumbnail in the film strip and scroll to display it
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Slide deleted");
	gdk_flush();

	// Free the resources allocated to the deleted slide
	slide_free(tmp_glist->data, NULL);
	g_list_free(tmp_glist);
}

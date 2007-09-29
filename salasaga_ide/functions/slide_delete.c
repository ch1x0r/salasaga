/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Delete from the top menu 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void slide_delete(void)
{
	// Local variables
	GtkTreePath			*new_path;				// Temporary path
	gint				num_slides;				// Number of slides in the whole slide list
	gint				slide_position;				// Which slide in the slide list we are deleting
	GtkTreeSelection		*film_strip_selector;
	GtkTreeIter			selection_iter;

	GList				*tmp_glist;				// Temporary GList


	// Are we trying to delete the only slide in the project (not good)?
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	if (1 == num_slides)
	{
		// Yes we are, so give a warning message and don't delete the slide
		display_warning("You must have at least one slide in a project.\n");
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
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);

	// Recreate the slide tooltips
	create_tooltips();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Free the resources allocated to the deleted slide
	destroy_slide(tmp_glist->data, NULL);
	g_list_free(tmp_glist);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

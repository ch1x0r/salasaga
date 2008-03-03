/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Move up from the top menu 
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "create_tooltips.h"


void slide_move_up(void)
{
	// Local variables
	GtkTreeIter			from_iter, to_iter;
	GtkTreePath			*new_path;				// Temporary path
	GList				*previous_slide;			// Pointer to the slide above
	gint				slide_position;				// Which slide in the slide list we are moving
	slide				*this_slide_data;			// Pointer to the data for this slide
	GString				*tmp_gstring;


	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	if (0 == slide_position)
	{
		// We can't move the upper most slide any further up, so just return
		return;
	}

	// Swap the slides around
	this_slide_data = current_slide->data;
	previous_slide = g_list_nth(slides, slide_position - 1);
	current_slide->data = previous_slide->data;
	previous_slide->data = this_slide_data;
	current_slide = previous_slide;

	// Move the thumbnail up one position in the film strip list
	g_string_printf(tmp_gstring, "%u", slide_position);
	if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &from_iter, tmp_gstring->str))
	{
		if (debug_level) printf("slide_move_top: 'From' iter is valid\n");
		g_string_printf(tmp_gstring, "%u", slide_position - 1);
		if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &to_iter, tmp_gstring->str))
		{
			if (debug_level) printf("slide_move_top: 'To' iter is valid\n");
			gtk_list_store_move_before(GTK_LIST_STORE(film_strip_store), &from_iter, &to_iter);
		}
	} else
	{
		if (debug_level) printf("slide_move_top: 'From' iter is not valid\n");
	}
	g_string_free(tmp_gstring, TRUE);

	// Recreate the slide tooltips
	create_tooltips();

	// Scroll the film strip to show the new thumbnail position
	new_path = gtk_tree_path_new_from_indices(slide_position - 1, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
//	gtk_tree_path_free(new_path);

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Slide moved up");
	gdk_flush();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.5  2008/03/03 03:03:41  vapour
 * Added status bar feedback message.
 *
 * Revision 1.4  2008/02/04 17:15:45  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.3  2008/01/15 16:19:06  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:37:24  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

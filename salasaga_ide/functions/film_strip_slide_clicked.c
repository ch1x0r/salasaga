/*
 * $Id$
 *
 * Flame Project: Callback function that updates the workspace and timeline when a different slide in the film strip is selected 
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

// GTK includes
#include <gtk/gtk.h>

// GConf include (not for windows)
#ifndef _WIN32
	#include <gconf/gconf.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "draw_timeline.h"
#include "draw_workspace.h"


void film_strip_slide_clicked(GtkTreeSelection *selection, gpointer data)
{
	// Local variables
	GtkTreePath			*selected_path;
	GtkTreeIter			selected_iter;
	gchar				*selection_string;
	GString				*tmp_gstring;


	// Determine if a row has been selected
	if (TRUE == gtk_tree_selection_get_selected(selection, NULL, NULL))
        {
		// * Update current_slide to be the clicked on slide's GList entry, then redraw the timeline and workspace *

		// Determine which slide is now selected
		tmp_gstring = g_string_new("0");
		if (TRUE == gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &selected_iter, tmp_gstring->str))
		{
			if (debug_level) printf("film_strip_slide_clicked: Iter is valid\n");
		} else
		{
			if (debug_level) printf("film_strip_slide_clicked: Iter is not valid\n");
		}
		g_string_free(tmp_gstring, TRUE);

		gtk_tree_selection_get_selected(selection, NULL, &selected_iter);
		if ((debug_level) && (!gtk_list_store_iter_is_valid(GTK_LIST_STORE(film_strip_store), &selected_iter)))
		{
			printf("Invalid iter!\n");
		}

		selected_path = gtk_tree_model_get_path(GTK_TREE_MODEL(film_strip_store), &selected_iter);
		selection_string = gtk_tree_path_to_string(selected_path);
		if (debug_level) printf("film_strip_slide_clicked: Slide selected: %s\n", selection_string);

		// Get a pointer to the clicked on slide's GList
		slides = g_list_first(slides);
		current_slide = g_list_nth(slides, atoi(selection_string));
		
		// Redraw the timeline
		draw_timeline();

		// Redraw the workspace
		draw_workspace();

		// Free the memory used to deterine the newly selected slide
//		g_free(selection_path);
//		gtk_tree_path_free(path);
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.2  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.1  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

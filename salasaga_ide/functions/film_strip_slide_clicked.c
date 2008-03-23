/*
 * $Id$
 *
 * Salasaga: Callback function that updates the workspace and timeline when a different slide in the film strip is selected 
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "draw_handle_box.h"
#include "draw_timeline.h"
#include "draw_workspace.h"


void film_strip_slide_clicked(GtkTreeSelection *selection, gpointer data)
{
	// Local variables
	GtkTreePath			*selected_path;
	GtkTreeIter			selected_iter;
	gchar				*selection_string;


	// Determine if a row has been selected
	if (TRUE == gtk_tree_selection_get_selected(selection, NULL, NULL))
        {
		// * Update current_slide to be the clicked on slide's GList entry, then redraw the timeline and workspace *

		// Determine which slide is now selected
		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(film_strip_store), &selected_iter);
		gtk_tree_selection_get_selected(selection, NULL, &selected_iter);
		if ((debug_level) && (!gtk_list_store_iter_is_valid(GTK_LIST_STORE(film_strip_store), &selected_iter)))
		{
			printf("Invalid iter! No layer selected when changing slide!\n");
		}

		selected_path = gtk_tree_model_get_path(GTK_TREE_MODEL(film_strip_store), &selected_iter);
		selection_string = gtk_tree_path_to_string(selected_path);

		// Get a pointer to the clicked on slide's GList
		slides = g_list_first(slides);
		current_slide = g_list_nth(slides, atoi(selection_string));
		
		// Redraw the timeline
		draw_timeline();

		// Redraw the workspace
		draw_workspace();

		// Redraw the layer handle box
		draw_handle_box();

		// Free the memory used to deterine the newly selected slide
		g_free(selection_string);
	}
}

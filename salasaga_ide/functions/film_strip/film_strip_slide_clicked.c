/*
 * $Id$
 *
 * Salasaga: Callback function that updates the workspace and timeline when a different slide in the film strip is selected 
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


// Standard includes
#include <stdlib.h>

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
#include "../working_area/draw_handle_box.h"
#include "../working_area/draw_workspace.h"


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
			printf(_("Invalid iter! No layer selected when changing slide!\n"));
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

		// Update the status bar
		gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, _(" Ready"));
		gdk_flush();
	}
}

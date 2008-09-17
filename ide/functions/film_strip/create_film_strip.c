/*
 * $Id$
 *
 * Salasaga: Function to create the film strip area 
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
#include "film_strip_slide_clicked.h"


void create_film_strip()
{
	// Purpose: To create the left film strip area
	//
	// Inputs: None
	//
	// Returns: None
	//
	// Important variables:
	//				film_strip_store (global)
	//				film_strip_view (global)
	//				film_strip_container (global)
	//
	// Example:
	//
	//	GtkScrolledWindow	*film_strip_container;
	//
	//	create_film_strip();
	//

	// Local variables
	GtkCellRenderer			*renderer;
	GtkTreeSelection		*selector;


	// Create the film strip top widget
	film_strip_container = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(film_strip_container), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	// Create a GtkListStore, to have the thumbnail data in
	film_strip_store = gtk_list_store_new(1, GDK_TYPE_PIXBUF);

	// Create the view of the list store
	film_strip_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(film_strip_store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(film_strip_view), FALSE);
	renderer = gtk_cell_renderer_pixbuf_new();
	film_strip_column = gtk_tree_view_column_new_with_attributes(_("Slide"), renderer, "pixbuf", 0, NULL);
	gtk_tree_view_column_set_sizing(film_strip_column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_append_column(GTK_TREE_VIEW(film_strip_view), film_strip_column);

	// Add the list view to the film strip
	gtk_container_add(GTK_CONTAINER(film_strip_container), GTK_WIDGET(film_strip_view));

	// Set the selection mode of the film strip to single
	selector = gtk_tree_view_get_selection(GTK_TREE_VIEW(film_strip_view));
	gtk_tree_selection_set_mode(GTK_TREE_SELECTION(selector), GTK_SELECTION_SINGLE);

	// Connect a signal handler to the film strip, which gets called whenever a selection is made
	gtk_tree_selection_set_mode(selector, GTK_SELECTION_SINGLE);
	g_signal_connect(G_OBJECT(selector), "changed", G_CALLBACK(film_strip_slide_clicked), NULL);
}

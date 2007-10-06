/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Move Layer Down toolbar button 
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
#include "display_warning.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "sound_beep.h"


void layer_move_down(void)
{
	// Local variables
	GList				*below_layer;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				num_layers;				// The number of layers in the select slide
	GList				*our_layer;				// The selected layer
	gint				selected_row;				// Holds the row that is selected

	GtkTreeViewColumn		*tmp_column;				// Temporary column
	GList				*tmp_glist;				// Temporary GList
	GtkTreePath			*tmp_path;				// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise various things
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	if (num_layers - 2 <= selected_row)
	{
		// We're already at the bottom of the list or the background layer is selected, so return
		sound_beep();
		display_warning("ED40: This layer is already at the bottom\n");
		return;
	}

	// Get details of the layers we're moving around
	our_layer = g_list_nth(layer_pointer, selected_row);
	below_layer = g_list_nth(layer_pointer, selected_row + 1);

	// Move the row down one in the timeline widget
	gtk_list_store_move_before(((slide *) current_slide->data)->layer_store, ((layer *) below_layer->data)->row_iter, ((layer *) our_layer->data)->row_iter);

	// Move the row down one in the layer list
	tmp_glist = g_list_remove_link(layer_pointer, below_layer);
	layer_pointer = g_list_insert_before(layer_pointer, our_layer, below_layer->data);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

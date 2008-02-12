/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Move Layer Up toolbar button 
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "sound_beep.h"


void layer_move_up(void)
{
	// Local variables
	GList				*above_layer;				// The layer above the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GList				*our_layer;					// The selected layer
	guint				selected_row;				// Holds the row that is selected

	GtkTreeViewColumn	*tmp_column;				// Temporary column
	GList				*tmp_glist;					// Temporary GList
	GtkTreePath			*tmp_path;					// Temporary path


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

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	if (0 == selected_row)
	{
		// We're already at the top of the list, so return
		sound_beep();
		display_warning("Error ED41: This layer is already at the top\n");
		return;
	}

	// Get details of the layers we're moving around
	our_layer = g_list_nth(layer_pointer, selected_row);
	above_layer = g_list_nth(layer_pointer, selected_row - 1);

	// Check if the user has selected the Background layer, if so we return (ignoring this move request)
	if (TRUE == ((layer *) our_layer->data)->background)
	{
		// We're processing a background layer, so return
		gdk_beep();
		return;
	}

	// Move the row up one in the timeline widget
	gtk_list_store_move_before(((slide *) current_slide->data)->layer_store, ((layer *) our_layer->data)->row_iter, ((layer *) above_layer->data)->row_iter);

	// Move the row up one in the layer list
	tmp_glist = g_list_remove_link(layer_pointer, our_layer);
	layer_pointer = g_list_insert_before(layer_pointer, above_layer, our_layer->data);
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
 * Revision 1.6  2008/02/12 14:11:03  vapour
 * Updated to use the new background field in the layer structure.
 *
 * Revision 1.5  2008/02/04 16:56:18  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.4  2008/01/19 06:41:54  vapour
 * Tweaked an error message for clarity.
 *
 * Revision 1.3  2008/01/15 16:18:58  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

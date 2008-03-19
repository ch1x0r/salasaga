/*
 * $Id$
 *
 * Salasaga: Function called when the visibility field in the timeline widget is edited 
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"


void timeline_edited_visibility(GtkCellRendererToggle *selection, gchar *row, GtkListStore *store)
{
	// Local variables
	GtkTreeIter			iter;
	layer				*layer_data;
	GList				*layer_pointer;
	gboolean			visibility;


	// Only continue if we're getting called properly
	if (!gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(store), &iter, row))
		return;

	// Set up some pointers to make things easier
	layer_pointer = ((slide *) current_slide->data)->layers;

	// Work out which layer had its visibility toggle clicked
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_nth(layer_pointer, atoi(row));
	layer_data = layer_pointer->data;

	// * Ensure we're not trying to change the state of a background nor empty layer *
	if ((TYPE_EMPTY == layer_data->object_type) || (TRUE == layer_data->background))
	{
		// This is either an empty layer or background layer, so it can't be made invisible
		gdk_beep();
		return;
	}

	// Get current state of visibility
	visibility = gtk_cell_renderer_toggle_get_active(selection);

	// Toggle the layer visibility
	if (TRUE == visibility)
	{
		layer_data->visible = FALSE;
		gtk_list_store_set(store, &iter, TIMELINE_VISIBILITY, FALSE, -1);
	} else
	{
		layer_data->visible = TRUE;
		gtk_list_store_set(store, &iter, TIMELINE_VISIBILITY, TRUE, -1);
	}

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}

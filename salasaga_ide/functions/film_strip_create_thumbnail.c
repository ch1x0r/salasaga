/*
 * $Id$
 *
 * Salasaga: Creates a thumbnail for a given slide
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


void film_strip_create_thumbnail(slide *slide_data)
{
	// Local variables
	GdkPixbuf			*new_thumbnail;
	GtkTreeIter			old_iter;					// Iter used to select the film strip thumbnail
	GtkTreePath			*old_path;					// Path used to select the film strip thumbnail
	GtkImage			*old_thumbnail;				// The old thumbnail


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Get the iter for the presently selected thumbnail in the film strip
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &old_path, NULL);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(film_strip_store), &old_iter, old_path);

	// Get a backup pointer to the old thumbnail so we can free it
	old_thumbnail = slide_data->thumbnail;

	// Create the thumbnail for the slide from the backing store
	new_thumbnail = gdk_pixbuf_scale_simple(backing_store, preview_width, (guint) preview_width * 0.75, GDK_INTERP_TILES);
	slide_data->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(new_thumbnail)));

	// Replace the old thumbnail with the new thumbnail
	gtk_list_store_set(film_strip_store, &old_iter, 0, new_thumbnail, -1);

	// Free the old thumbnail
	// fixme2: Needs to be written
}

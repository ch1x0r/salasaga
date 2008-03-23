/*
 * $Id$
 *
 * Salasaga: Function to regenerate the film strip thumbnails
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "compress_layers.h"


void regenerate_film_strip_thumbnails()
{
	// Local variables
	GtkTreeIter			film_strip_iter;
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	GdkPixbuf			*new_thumbnail;
	gint				num_slides;
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gint				slide_counter, slide_position;
	GList				*this_slide;


	// Safety check
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	if (0 == num_slides)
	{
		// There aren't any slides in this project yet, so just return
		return;
	}

	// Get the path to the presently selected thumbnail in the film strip
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &new_path, NULL);

	// Remove the existing film strip thumbnails
	gtk_list_store_clear(GTK_LIST_STORE(film_strip_store));

	// Generate new thumbnails
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		// Create the thumbnail for the slide
		this_slide = g_list_nth(slides, slide_counter);
		new_thumbnail = compress_layers(this_slide, preview_width, (guint) preview_width * 0.75);
		((slide *) this_slide->data)->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(new_thumbnail)));

		// Add the thumbnail to the film strip
		gtk_list_store_append(film_strip_store, &film_strip_iter);
		gtk_list_store_set(film_strip_store, &film_strip_iter, 0, gtk_image_get_pixbuf(((slide *) this_slide->data)->thumbnail), -1);
	}

	// Reselect the thumbnail that was previously selected
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path
}

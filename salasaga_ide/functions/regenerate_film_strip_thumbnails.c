/*
 * $Id$
 *
 * Flame Project: Function to regenerate the film strip thumbnails
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
#include "compress_layers.h"


void regenerate_film_strip_thumbnails()
{
	// Local variables
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	gint				num_slides;
	gint				slide_counter, slide_position;
	GtkTreeIter			film_strip_iter;
	GdkPixbuf			*new_thumbnail;
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
	if (NULL == new_path)
	{
		// We couldn't get the existing path in order to reuse it, so we create a new one 
		slides = g_list_first(slides);
		slide_position = g_list_position(slides, current_slide);
		new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	}
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(film_strip_view), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(film_strip_view), new_path, NULL, TRUE, 0.5, 0.0);
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.8  2008/02/20 03:56:14  vapour
 * Updated to reuse an existing path if available, rather than recreating a new one each time.  Was a potential (small) memory leak.
 *
 * Revision 1.7  2008/02/19 18:04:03  vapour
 * Fixed a crashing bug caused by the film strip thumbnails being deselected every time they were regenerated.  Now the previously selected film strip thumbnail is reselected after they're regenerated.
 *
 * Revision 1.6  2008/02/04 17:09:58  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.5  2008/01/15 16:19:07  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.4  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.3  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:08  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:45  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

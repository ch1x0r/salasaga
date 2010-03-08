/*
 * $Id$
 *
 * Salasaga: Function to regenerate the film strip thumbnails
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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
#include "../layer/compress_layers.h"
#include "../preference/application_preferences.h"
#include "../preference/project_preferences.h"
#include "../time_line/time_line.h"
#include "../time_line/time_line_get_type.h"
#include "../time_line/time_line_get_cursor_position.h"
#include "../time_line/time_line_new.h"
#include "../time_line/time_line_set_stored_slide_duration.h"


void regenerate_film_strip_thumbnails()
{
	// Local variables
	gfloat				cursor_position;
	GtkTreeIter			film_strip_iter;
	GtkTreePath			*new_path;					// Path used to select the new film strip thumbnail
	gint				num_slides;
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	guint				preview_height;				// The height we calculate a film strip thumbnail should be
	gfloat				project_ratio;				// Ratio of project height to width
	gint				slide_counter;
	gint				slide_position;
	GList				*this_slide;
	slide				*this_slide_data;			// Points to the presently processing slide
	GdkPixbuf			*tmp_pixbuf;				// Used to convert from a pixmap to a pixbuf
	GdkPixmap			*tmp_pixmap;				// Used when converting from a pixmap to a pixbuf


	// Safety check
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	if (0 == num_slides)
	{
		// There aren't any slides in this project yet, so just return
		return;
	}

	// Get the path to the presently selected thumbnail in the film strip
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &new_path, NULL);

	// Remove the existing film strip thumbnails
	gtk_list_store_clear(GTK_LIST_STORE(get_film_strip_store()));

	// Generate new thumbnails
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		// Point to the desired slide data
		slides = g_list_first(slides);
		this_slide = g_list_nth(slides, slide_counter);
		this_slide_data = (slide *) this_slide->data;

		// If the present slide doesn't have a time line widget, then create one
		if (FALSE == IS_TIME_LINE(this_slide_data->timeline_widget))
		{
			// Construct the widget used to display the slide in the timeline
			this_slide_data->timeline_widget = time_line_new();
			time_line_set_stored_slide_duration(this_slide_data->timeline_widget, this_slide_data->duration);
		}

		// Get the current time line cursor position
		cursor_position = time_line_get_cursor_position(this_slide_data->timeline_widget);

		// Determine the proper thumbnail height
		project_ratio = (gfloat) get_project_height() / (gfloat) get_project_width();
		preview_height = get_preview_width() * project_ratio;

		// Create the thumbnail for the slide
		tmp_pixmap = compress_layers(this_slide, cursor_position, get_project_width(), get_project_height());
		if (NULL == tmp_pixmap)
		{
			return;
		}
		tmp_pixbuf = gdk_pixbuf_get_from_drawable(NULL, GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);
		this_slide_data->thumbnail = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_pixbuf), get_preview_width(), preview_height, GDK_INTERP_TILES);
		g_object_unref(GDK_PIXBUF(tmp_pixbuf));
		g_object_unref(GDK_PIXMAP(tmp_pixmap));

		// Add the thumbnail to the film strip
		gtk_list_store_append(get_film_strip_store(), &film_strip_iter);
		gtk_list_store_set(get_film_strip_store(), &film_strip_iter, 0, this_slide_data->thumbnail, -1);
	}

	// Reselect the thumbnail that was previously selected
	if (NULL != new_path)
		old_path = new_path;  // Make a backup of the old path, so we can free it
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, get_current_slide());
	new_path = gtk_tree_path_new_from_indices(slide_position, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, FALSE);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(get_film_strip_view()), new_path, NULL, TRUE, 0.5, 0.0);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path
}

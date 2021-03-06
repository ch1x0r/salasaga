/*
 * $Id$
 *
 * Salasaga: Creates a thumbnail for a given slide
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../preference/application_preferences.h"
#include "../preference/project_preferences.h"


void film_strip_create_thumbnail(slide *slide_data)
{
	// Local variables
	GString				*message;					// Used to construct message strings
	GdkPixbuf			*new_thumbnail;				//
	GtkTreeIter			old_iter;					// Iter used to select the film strip thumbnail
	GtkTreePath			*old_path;					// Path used to select the film strip thumbnail
	guint				preview_height;				// The height we calculate a film strip thumbnail should be
	gfloat				project_ratio;				// Ratio of project height to width
	GdkPixbuf			*tmp_pixbuf;				// Used to convert from a pixmap to a pixbuf


	// If no project is loaded then don't run this function
	if (NULL == get_current_slide())
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Get the iter for the presently selected thumbnail in the film strip
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(get_film_strip_view()), &old_path, NULL);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(get_film_strip_store()), &old_iter, old_path);

	// Determine the proper thumbnail height
	project_ratio = (gfloat) get_project_height() / (gfloat) get_project_width();
	preview_height = get_preview_width() * project_ratio;

	// Create the thumbnail for the slide from the working space pixmap
	tmp_pixbuf = gdk_pixbuf_get_from_drawable(NULL, GDK_PIXMAP(get_front_store()), NULL, 0, 0, 0, 0, -1, -1);
	if (NULL == tmp_pixbuf)
	{
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED369: %s", _("Error"), _("Couldn't create film strip thumbnail."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		gtk_tree_path_free(old_path);
		return;
	}
	new_thumbnail = gdk_pixbuf_scale_simple(GDK_PIXBUF(tmp_pixbuf), get_preview_width(), preview_height, GDK_INTERP_TILES);
	if (NULL == new_thumbnail)
	{
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED370: %s", _("Error"), _("Couldn't allocate memory for a new film strip thumbnail."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		gtk_tree_path_free(old_path);
		g_object_unref(GDK_PIXBUF(tmp_pixbuf));
		return;
	}
	g_object_unref(GDK_PIXBUF(tmp_pixbuf));

	// Replace the old film strip thumbnail with the new thumbnail
	gtk_list_store_set(get_film_strip_store(), &old_iter, 0, new_thumbnail, -1);

	// Free the memory used in this function
	gtk_tree_path_free(old_path);
}

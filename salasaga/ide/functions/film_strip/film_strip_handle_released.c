/*
 * $Id$
 *
 * Salasaga: Callback function called when the resizing of the film strip is completed 
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
#include "../preference/application_preferences.h"
#include "regenerate_film_strip_thumbnails.h"


gint film_strip_handle_released(GObject *paned, GParamSpec *pspec, gpointer data)
{
	// Check if we're in the middle of resizing the film strip
	if (TRUE == get_film_strip_being_resized())
	{
		// Set the new width of the film strip widget
		gtk_tree_view_column_set_fixed_width(GTK_TREE_VIEW_COLUMN(film_strip_column), get_preview_width());
		
		// Regenerate the film strip thumbnails at the new size
		regenerate_film_strip_thumbnails();

		// Set a toggle to indicate the film strip width changing has completed
		set_film_strip_being_resized(FALSE);
	}

	// Indicate to the calling routine that this function finished fine
	return FALSE;
}

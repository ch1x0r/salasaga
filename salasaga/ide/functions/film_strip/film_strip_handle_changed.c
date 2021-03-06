/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses moves the film strip handle 
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "../preference/application_preferences.h"


void film_strip_handle_changed(GObject *paned, GParamSpec *pspec, gpointer data)
{
	// Temporary variables
	GValue				*handle_size;						// The size of the dividing handle for the film strip
	gint				new_position;


	// Get the new position of the film strip seperator
	new_position = gtk_paned_get_position(GTK_PANED(paned));

	// Get the handle size of the film strip widget
	handle_size = g_new0(GValue, 1);
	g_value_init(handle_size, G_TYPE_INT);
	gtk_widget_style_get_property(GTK_WIDGET(get_main_area()), "handle-size", handle_size);
	new_position -= (g_value_get_int(handle_size) + 15);
	g_free(handle_size);

	// If the handle has moved, set the new thumbnail width in the application preferences
	if (new_position != get_preview_width())
	{
		// Set a toggle to indicate the film strip width is being changed
		set_film_strip_being_resized(TRUE);
		set_preview_width(new_position);
	}
}

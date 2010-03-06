/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses a new zoom level
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
#include "working_area/draw_workspace.h"


gint zoom_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Temporary variables
	gchar				*tmp_gchar;
	GString				*tmp_string;


	// Get the new zoom level
	tmp_string = g_string_new(NULL);
	tmp_gchar = gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector));
	g_string_printf(tmp_string, "%s", tmp_gchar);
	g_free(tmp_gchar);

	// Parse and store the new zoom level
	if ((0 == g_strcmp0("Fit to width", tmp_string->str)) || (0 == g_strcmp0(_("Fit to width"), tmp_string->str)))
	{
		// "Fit to width" is selected, so work out a new zoom level by figuring out how much space the widget really has
		//  (Look at the allocation of it's parent widget)
		//  Reduce the width calculated by 24 pixels (guessed) to give space for widget borders and such
		set_zoom((guint) (((float) (right_side->allocation.width - 24) / (float) project_width) * 100));
	} else
	{
		tmp_string = g_string_truncate(tmp_string, tmp_string->len - 1);
		set_zoom(atoi(tmp_string->str));
	}

	// Sanity check the zoom level.  Sometimes needed when using small project sizes.
	if (get_zoom() >= 400.0)
	{
		set_zoom(400.0);
	}

	// Free the memory allocated in this function
	g_string_free(tmp_string, TRUE);

	// Calculate and set the display size of the working area
	set_working_width((project_width * get_zoom()) / 100);
	set_working_height((project_height * get_zoom()) / 100);

	// Resize the drawing area so it draws properly
	gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), get_working_width(), get_working_height());

	// Free the existing front store for the workspace
	if (NULL != front_store)
	{
		g_object_unref(GDK_PIXMAP(front_store));
		front_store = NULL;
	}

	// Redraw the workspace area
	draw_workspace();

	// Indicate to the calling routine that this function finished fine
	return TRUE;
}

/*
 * $Id$
 *
 * Salasaga: Function called when the drawing area is resized
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
#include "../zoom_selector/zoom_selector_changed.h"


gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	static gint			old_width = 0;
	GtkWidget			*right_side_widget;		// Temporarily holds a pointer to the right side widget
	gchar				*tmp_gchar;
	GString				*tmp_string;


	// Check if the width of the drawing area has changed
	right_side_widget = get_right_side();
	if (old_width != right_side_widget->allocation.width)
	{
		// * The width has been changed *

		// If the required zoom level is "Fit to width", then recalculate the zoom and redraw the drawing area
		tmp_string = g_string_new(NULL);
		tmp_gchar = gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector));
		g_string_printf(tmp_string, "%s", tmp_gchar);
		g_free(tmp_gchar);

		// Parse and store the new zoom level
		if ((0 == g_strcmp0("Fit to width", tmp_string->str)) || (0 == g_strcmp0(_("Fit to width"), tmp_string->str)))
		{
			zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
		}

		old_width = right_side_widget->allocation.width;

		// Resize the drawing area so it draws properly
		gtk_widget_set_size_request(GTK_WIDGET(get_main_drawing_area()), get_working_width(), get_working_height());

		// Free the memory allocated in this function
		g_string_free(tmp_string, TRUE);
	}

	// Return FALSE to continue event propagation
	return FALSE;
}

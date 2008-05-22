/*
 * $Id$
 *
 * Salasaga: Function called when the drawing area is resized 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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
#include "../zoom_selector_changed.h"


gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	// Local variables
	static gint			old_width = 0;
	gint32				tmp_int;
	GString				*tmp_string;


	// Check if the width of the drawing area has changed
	if (old_width != right_side->allocation.width)
	{
		// * The width has been changed *
		
		// If the required zoom level is "Fit to width", then recalculate the zoom and redraw the drawing area
		tmp_string = g_string_new(NULL);
		g_string_printf(tmp_string, "%s", gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));
	
		// Parse and store the new zoom level
		tmp_int = g_ascii_strncasecmp(tmp_string->str, "F", 1);
		if (0 == tmp_int)
		{
			zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
		}

		old_width = right_side->allocation.width;

		// Resize the drawing area so it draws properly
		gtk_widget_set_size_request(GTK_WIDGET(main_drawing_area), working_width, working_height);

		// Free the memory allocated in this function
		g_string_free(tmp_string, TRUE);
	}

	// Return FALSE to continue event propagation
	return FALSE;
}

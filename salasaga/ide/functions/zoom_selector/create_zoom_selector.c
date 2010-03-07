/*
 * $Id$
 *
 * Salasaga: Function to create the zoom level selector 
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


GtkWidget *create_zoom_selector(gchar *initial_zoom)
{
	// Local variables
	gboolean			match_found;				// Simple toggle used to indiate the requested zoom has been found
	gint				match_at;					// If a zoom match was found, this contains where
	gint				num_zoom_items;				// The number of zoom options available
	guint				zoom_counter;				// Used as a simple counter
	gchar				*zoom_selector_array[] = { "400%", "300%", "200%", "150%", "100%", "75%", "50%", "25%", "10%", _("Fit to width") };  // The zoom options
	GtkWidget			*zoom_widget;				// ComboBox widget that gets returned


	// Initialise various things
	match_found = FALSE;
	match_at = -1;
	zoom_widget = gtk_combo_box_new_text();
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(zoom_widget), FALSE);

	// Construct the zoom selector widget
	num_zoom_items = sizeof(zoom_selector_array) / sizeof(zoom_selector_array[0]);
	for (zoom_counter = 0; zoom_counter < num_zoom_items; zoom_counter++)
	{
		// Add the zoom option to the widget
		gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_widget), zoom_selector_array[zoom_counter]);

		// Check if this zoom option is a match for the requested initial zoom
		if (0 == g_strcmp0(initial_zoom, zoom_selector_array[zoom_counter]))
		{
			match_found = TRUE;
			match_at = zoom_counter;
		} else {
			// Check if the zoom option is a match for the "Fit to width" string and it's localised version
			if ((0 == g_strcmp0("Fit to width", zoom_selector_array[zoom_counter])) || (0 == g_strcmp0(_("Fit to width"), zoom_selector_array[zoom_counter])))
			{
				if ((0 == g_strcmp0("Fit to width", initial_zoom)) || (0 == g_strcmp0(_("Fit to width"), initial_zoom)))
				{
					match_found = TRUE;
					match_at = zoom_counter;
				}
			}
		}
	}

	// If no match was found we add the requested zoom to the end of the list
	if (TRUE != match_found)
	{
		gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_widget), initial_zoom);
		match_at = num_zoom_items;  // Point to the new entry
	}

	// Select the chosen resolution as the default
	gtk_combo_box_set_active(GTK_COMBO_BOX(zoom_widget), match_at);

	// Return the resolution widget
	return zoom_widget;
}

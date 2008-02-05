/*
 * $Id$
 *
 * Flame Project: Function to create the output resolution selector 
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


GtkWidget *create_resolution_selector(ResolutionStructure *res_array, guint num_resolutions, guint initial_width, guint initial_height)
{
	// Local variables
	gboolean			match_found;				// Simple toggle used to indiate the requested resolution has been found
	gint				match_at;					// If a resolution match was found, this contains where
	guint				res_counter;				// Used as a simple counter
	GString				*res_string;				// Used for constructing resolution strings
	GtkWidget			*res_widget;				// ComboBox widget that gets returned


	// Initialise various things
	match_found = FALSE;
	match_at = -1;
	res_string = g_string_new(NULL);
	res_widget = gtk_combo_box_new_text();

	// Check if the requested resolution is present in the resolution array
	for (res_counter = 0; res_counter < num_resolutions; res_counter++)
	{
		if ((res_array + res_counter)->width == initial_width)
		{
			// There's a match on width, so check for a match on height
			if ((res_array + res_counter)->height == initial_height)
			{
				match_found = TRUE;
				match_at = res_counter;
			}
		}
	}

	// Construct the GtkComboBox from the supplied list of resolutions
	for (res_counter = 0; res_counter < num_resolutions; res_counter++)
	{
		g_string_printf(res_string, "%ux%u pixels", (res_array + res_counter)->width, (res_array + res_counter)->height);
		gtk_combo_box_append_text(GTK_COMBO_BOX(res_widget), res_string->str);
	}

	// If no match was found we add the requested resolution on to the end of the list
	if (TRUE != match_found)
	{
		g_string_printf(res_string, "%ux%u pixels", initial_width, initial_height);
		gtk_combo_box_append_text(GTK_COMBO_BOX(res_widget), res_string->str);
		match_at = num_resolutions;  // Point to the new entry
	}

	// Select the chosen resolution as the default
	gtk_combo_box_set_active(GTK_COMBO_BOX(res_widget), match_at);

	// Free memory allocated during this function
	g_string_free(res_string, TRUE);

	// Return the resolution widget
	return res_widget;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/02/05 06:22:28  vapour
 * Resolution selector widget now says pixels instead of px.
 *
 * Revision 1.3  2008/02/04 15:02:49  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.2  2008/01/15 16:19:07  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:17  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

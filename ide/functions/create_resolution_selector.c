/*
 * $Id$
 *
 * Salasaga: Function to create the output resolution selector
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
#include "../salasaga_types.h"
#include "../externs.h"


GtkWidget *create_resolution_selector(guint initial_width, guint initial_height)
{
	// Default output resolutions
	resolution_structure	res_array[] =
	{
		{ 1920, 1200 },
		{ 1920, 1080 },
		{ 1600, 1200 },
		{ 1280, 1024 },
		{ 1280, 720 },
		{ 1024, 768 },
		{ 800, 600 },
		{ 720, 480 },
		{ 640, 480 },
		{ 600, 400 },
		{ 528, 396 },
		{ 480, 120 },
		{ 352, 288 },
		{ 320, 240 },
		{ 176, 144 },
		{ 160, 120 },
		{ 128, 96 }
	};

	// Local variables
	gboolean			match_found;				// Simple toggle used to indiate the requested resolution has been found
	gint				match_at;					// If a resolution match was found, this contains where
	gint				num_resolutions = sizeof(res_array) / sizeof(res_array[0]);	// The number of resolution array items
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
		g_string_printf(res_string, "%ux%u %s", (res_array + res_counter)->width, (res_array + res_counter)->height, _("pixels"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(res_widget), res_string->str);
	}

	// If no match was found we add the requested resolution on to the end of the list
	if (TRUE != match_found)
	{
		g_string_printf(res_string, "%ux%u %s", initial_width, initial_height, _("pixels"));
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

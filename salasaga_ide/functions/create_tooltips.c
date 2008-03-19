/*
 * $Id$
 *
 * Salasaga: Recreate the tooltips for the slides 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"


void create_tooltips(void)
{
	// Local variables
	guint				counter;				// Used as a standard counter
	GString				*name_string;				// Used for generating a name string
	guint				num_slides;				// Number of slides in the whole slide list
	slide				*slide_data;				// Points to the actual data in the given slide


	// Initialise some things
	name_string = g_string_new(NULL);
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);

	// Loop through each slide, creating an appropriate tooltip for it
	for (counter = 0; counter < num_slides; counter++)
	{
		slide_data = g_list_nth_data(slides, counter);
		slide_data->tooltip = gtk_tooltips_new();
		if (NULL == slide_data->name)
		{
			// The user hasn't given a name to the slide, so use a default
			g_string_printf(name_string, "Slide %u", counter + 1);
			gtk_tooltips_set_tip(slide_data->tooltip, GTK_WIDGET(slide_data->thumbnail), name_string->str, NULL);
		} else
		{
			// The user has named the slide, so use the name
			gtk_tooltips_set_tip(slide_data->tooltip, GTK_WIDGET(slide_data->thumbnail), slide_data->name->str, NULL);
		}
	}
}

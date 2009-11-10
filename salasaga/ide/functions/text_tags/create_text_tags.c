/*
 * $Id$
 *
 * Salasaga: Creates the text tags that can be applied to text layers
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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


gboolean create_text_tags(void)
{
	// Local variables
	gint			loop_counter;
	gint			*int_ptr;


	// Create a global text tag table
	text_tags_table = gtk_text_tag_table_new();

	// Create all of the text tags responsible for setting the font face
	for (loop_counter = 0; loop_counter < FONT_COUNT; loop_counter++)
	{
		// Create the text tag itself
		text_tags_fonts[loop_counter] = gtk_text_tag_new(salasaga_font_names[loop_counter]);

		// Add properties to the text tag.  In this case, we add the font to be applied when the tag is applied
		g_object_set(text_tags_fonts[loop_counter], "font", salasaga_font_names[loop_counter], NULL);

		// Add a matching data item to this tag, pointing to the equivalent font in our loaded cairo font face array
		g_object_set_data(G_OBJECT(text_tags_fonts[loop_counter]), "array-font", cairo_font_face[loop_counter]);

		// Add a data item to this tag, indicating which font number in our list of fonts it is
		int_ptr = g_slice_new0(gint);
		*int_ptr = loop_counter;
		g_object_set_data(G_OBJECT(text_tags_fonts[loop_counter]), "font-num", int_ptr);

		// Add the font tag to the global text table
		gtk_text_tag_table_add(text_tags_table, text_tags_fonts[loop_counter]);
	}

	return TRUE;
}

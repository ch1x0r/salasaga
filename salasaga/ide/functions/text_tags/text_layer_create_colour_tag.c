/*
 * $Id$
 *
 * Salasaga: Function to create a foreground colour text layer text tag
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
#include "../global_functions.h"


GtkTextTag *text_layer_create_colour_tag(GdkColor *fg_colour)
{
	// Local variables
	GtkTextTag			*fg_colour_tag;			// Foreground colour text tag
	GString				*fg_colour_tag_name;	// Temporary string we assemble the desired tag name in


	// Initialisation
	fg_colour_tag_name = g_string_new(NULL);

	// Create the name of a text tag to match the desired colour
	g_string_printf(fg_colour_tag_name, "text fg colour #%u-%u-%u", fg_colour->red, fg_colour->green, fg_colour->blue);
	fg_colour_tag = gtk_text_tag_table_lookup(GTK_TEXT_TAG_TABLE(get_text_tags_table()), fg_colour_tag_name->str);
	if (NULL == fg_colour_tag)
	{
		// No text tag with the requested colour already exists in the tag table, so we create one
		fg_colour_tag = gtk_text_tag_new(fg_colour_tag_name->str);
		g_object_set(GTK_TEXT_TAG(fg_colour_tag), "foreground-gdk", fg_colour, NULL);

		// Add the new tag to the global text table
		gtk_text_tag_table_add(GTK_TEXT_TAG_TABLE(get_text_tags_table()), GTK_TEXT_TAG(fg_colour_tag));

		// Add the new colour tag to the linked list
		set_text_tags_fg_colour_slist(g_slist_prepend(get_text_tags_fg_colour_slist(), GTK_TEXT_TAG(fg_colour_tag)));
	}

	// Free the memory used in this function
	g_string_free(fg_colour_tag_name, TRUE);

	// Return the requested text tag
	return fg_colour_tag;
}

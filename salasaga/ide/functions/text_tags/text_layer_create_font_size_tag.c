/*
 * $Id$
 *
 * Salasaga: Function to create a font size text layer text tag
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


GtkTextTag *text_layer_create_font_size_tag(gdouble font_size)
{
	// Local variables
	GtkTextTag			*text_size_text_tag;	// Text size text tag
	GString				*tag_name_text_size;	// Temporary string we assemble the desired tag name in


	// Initialisation
	tag_name_text_size = g_string_new(NULL);

	// Create the name of a text tag to match the desired size
	g_string_printf(tag_name_text_size, "text size %.2f", font_size);

	// We replace any comma characters in the generated string with full stop characters, so
	// that transporting project files between users with different numeric locales works
	g_strcanon(tag_name_text_size->str, "tex siz0123456789", '.');

	text_size_text_tag = gtk_text_tag_table_lookup(GTK_TEXT_TAG_TABLE(get_text_tags_table()), tag_name_text_size->str);
	if (NULL == text_size_text_tag)
	{
		// No text tag with the requested size already exists in the tag table, so we create one
		text_size_text_tag = gtk_text_tag_new(tag_name_text_size->str);
		g_object_set(GTK_TEXT_TAG(text_size_text_tag), "size-points", font_size, NULL);

		// Add the new tag to the global text table
		gtk_text_tag_table_add(GTK_TEXT_TAG_TABLE(get_text_tags_table()), GTK_TEXT_TAG(text_size_text_tag));

		// Add the new size tag to the linked list
		set_text_tags_size_slist(g_slist_prepend(get_text_tags_size_slist(), GTK_TEXT_TAG(text_size_text_tag)));
	}

	// Free the memory used in this function
	g_string_free(tag_name_text_size, TRUE);

	// Return the requested text tag
	return text_size_text_tag;
}

/*
 * $Id:$
 *
 * Salasaga: Creates a duplicate of a given text buffer, preserving tags and so forth
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
#include "../salasaga_types.h"
#include "../externs.h"


GtkTextBuffer *gtk_text_buffer_duplicate(GtkTextBuffer *source_buffer)
{
	// Local variables
	GdkAtom				format_atom_dest;
	GdkAtom				format_atom_source;
	GtkTextBuffer		*new_text_buffer;
	guint8				*serialised_buffer;
	gsize				serialised_length;
	GtkTextIter			text_end;					// End position of text buffer
	GtkTextIter			text_start;					// Start position of text buffer


	// Create a new text buffer
	new_text_buffer = gtk_text_buffer_new(text_tags_table);

	// Register text buffers for gtk's internal rich text tagset format
	format_atom_source = gtk_text_buffer_register_serialize_tagset(source_buffer, "salasaga_text");
	format_atom_dest = gtk_text_buffer_register_deserialize_tagset(new_text_buffer, "salasaga_text");

	// Duplicate the contents of the existing text buffer into the new one
	gtk_text_buffer_get_bounds(source_buffer, &text_start, &text_end);
	serialised_buffer = gtk_text_buffer_serialize(source_buffer, source_buffer, format_atom_source, &text_start, &text_end, &serialised_length);

	// Deserialise the data into the new buffer
	gtk_text_buffer_get_start_iter(new_text_buffer, &text_start);
	gtk_text_buffer_deserialize(new_text_buffer, new_text_buffer, format_atom_dest, &text_start, serialised_buffer, serialised_length, NULL);

	// Free the serialised buffer
	g_free(serialised_buffer);

	// Unregister the serialisation formats
	gtk_text_buffer_unregister_deserialize_format(source_buffer, format_atom_source);
	gtk_text_buffer_unregister_deserialize_format(new_text_buffer, format_atom_dest);

	// Return the duplicated text buffer
	return new_text_buffer;
}

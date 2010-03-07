/*
 * $Id$
 *
 * Salasaga: Creates a duplicate of a given text buffer, preserving tags and so forth
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
#include "callbacks/text_layer_dialog_validate_buffer_tag_quantity.h"
#include "dialog/display_warning.h"


GtkTextBuffer *gtk_text_buffer_duplicate(GtkTextBuffer *source_buffer)
{
	// Local variables
	gchar				*conversion_buffer;			// Used when converting between unicode character types
	GtkTextIter			end_iter;
	GtkTextIter			end_iter_minus_one;
	gint				end_offset;
	gint				i;
	guint				loop_counter;
	GtkTextIter 		loop_iter;
	GtkTextBuffer		*new_text_buffer;
	guint				num_tags;
	GtkTextIter			source_buffer_end;
	GtkTextIter			source_buffer_start;
	gint				start_offset;
	GSList				*tag_list;
	GtkTextTag			*tag_ptr;
	gunichar			temp_char;
	GString				*temp_gstring;


	// Validate the tags in the source buffer
	text_layer_dialog_validate_buffer_tag_quantity(GTK_TEXT_BUFFER(source_buffer));

	// Initialise various things
	temp_gstring = g_string_new(NULL);

	// Create a new text buffer
	new_text_buffer = gtk_text_buffer_new(text_tags_table);

	// Get the bounds of the source buffer
	gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(source_buffer), &source_buffer_start, &source_buffer_end);
	gtk_text_iter_order(&source_buffer_start, &source_buffer_end);

	// Scan through the source text buffer one character at a time, getting the character and the tags that apply to it
	start_offset = gtk_text_iter_get_offset(&source_buffer_start);
	end_offset = gtk_text_iter_get_offset(&source_buffer_end);
	for (i = 0; i < end_offset; i++)
	{
		// Copy one character from the source text buffer to the new destination text buffer
		gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(source_buffer), &loop_iter, i);
		temp_char = gtk_text_iter_get_char(&loop_iter);
		conversion_buffer = g_ucs4_to_utf8(&temp_char, 1, NULL, NULL, NULL);
		if (NULL == conversion_buffer)
		{
			g_string_printf(temp_gstring, "%s ED441: %s", _("Error"), _("Could not convert unicode character from ucs4 to utf8."));
			display_warning(temp_gstring->str);
			continue;
		}

		// Validate the retrieved character
		if (TRUE != g_unichar_validate(temp_char))
		{
			// Something other than a unicode character was retrieved
			g_string_printf(temp_gstring, "%s ED442: %s", _("Error"), _("Invalid unicode character found in text."));
			display_warning(temp_gstring->str);
			continue;
		}
		gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER(new_text_buffer), conversion_buffer, -1);
		g_free(conversion_buffer);

		// Copy the tags from the character in the source buffer to the new character in the destination buffer
		tag_list = gtk_text_iter_get_tags(&loop_iter);
		num_tags = g_slist_length(tag_list);
		gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(new_text_buffer), &end_iter);
		end_iter_minus_one = end_iter;
		gtk_text_iter_backward_char(&end_iter_minus_one);
		for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
		{
			// Copy each tag from the source text buffer to the destination one
			tag_ptr = g_slist_nth_data(tag_list, loop_counter);
			gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(new_text_buffer), tag_ptr, &end_iter_minus_one, &end_iter);
		}
		g_slist_free(tag_list);
	}
	g_string_free(temp_gstring, TRUE);

	// Validate the tags in the duplicated buffer
	text_layer_dialog_validate_buffer_tag_quantity(GTK_TEXT_BUFFER(new_text_buffer));

	// Return the duplicated text buffer
	return new_text_buffer;
}

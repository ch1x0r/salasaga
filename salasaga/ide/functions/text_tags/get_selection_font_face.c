/*
 * $Id$
 *
 * Salasaga: Returns the font face of the selected text in a text buffer,
 *           or -1 if there are multiple font faces in the selection
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


gint get_selection_font_face(GtkTextBuffer *text_buffer)
{
	// Local variables
	GSList					*applied_tags;		// Receives a list of text tags applied at a position in a text buffer
	GtkTextIter				end_iter;			// Character iter pointing to the last character in the selection
	gint					font_face = -1;		// Used for calculating the font face of a character
	gint					*font_ptr;			// Used for calculating the font face of a character
	gint					loop_counter;		// Simple counter used in loops
	gint					num_tags;			// Receives the total number of tags applied to a text character
	gint					starting_font_face = -1;  // Used for calculating the font face of a character
	GtkTextIter				first_iter;			// Character iter used when looping through the selection
	GtkTextTag				*this_tag = NULL;	// Used in a loop for pointing to individual text tags


	// Retrieve the selection start and end iters
	gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &first_iter, &end_iter);
	gtk_text_iter_order(&first_iter, &end_iter);

	// If the start iter is not at the start of the text buffer, we move it back one character to get an accurate value
	if (FALSE == gtk_text_iter_is_start(&first_iter))
	{
		gtk_text_iter_backward_char(&first_iter);
	}

	// Unless the start and end iters are at the same place, move the end iter back one so we
	// get an accurate value
	if (FALSE == gtk_text_iter_equal(&first_iter, &end_iter))
	{
		gtk_text_iter_backward_char(&end_iter);
	}

	// Run through the tags at the first iter to determine which font face is applied
	applied_tags = gtk_text_iter_get_tags(&first_iter);
	num_tags = g_slist_length(applied_tags);
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		this_tag = g_slist_nth_data(applied_tags, loop_counter);
		font_ptr = g_object_get_data(G_OBJECT(this_tag), "font-num");
		if (NULL != font_ptr)
		{
			// * Only font face tags have the "font-num" property, so this is a font face tag *
			starting_font_face = *font_ptr;
		}
	}

	// Free the list of tags, as they're no longer needed
	g_slist_free(applied_tags);

	// Step through the text buffer character by character, checking if the font face has changed
	while (FALSE == gtk_text_iter_equal(&first_iter, &end_iter))
	{
		// Move forward one iter
		gtk_text_iter_forward_char(&first_iter);

		// Get the font face at this new iter
		applied_tags = gtk_text_iter_get_tags(&first_iter);
		num_tags = g_slist_length(applied_tags);
		for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
		{
			this_tag = g_slist_nth_data(applied_tags, loop_counter);
			font_ptr = g_object_get_data(G_OBJECT(this_tag), "font-num");
			if (NULL != font_ptr)
			{
				// * Only font face tags have the "font-num" property, so this is a font face tag *
				font_face = *font_ptr;
			}
		}

		// Free the list of tags, as they're no longer needed
		g_slist_free(applied_tags);

		// If the font face is different to the original one, then we short circuit the loop and return -1
		if (starting_font_face != font_face)
		{
			return -1;
		}
	}

	// We've reached the end of the selection, and the selection font face
	// has been the same the whole way.  We return that font face.
	return starting_font_face;
}

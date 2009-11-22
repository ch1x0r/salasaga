/*
 * $Id$
 *
 * Salasaga: Returns the font size of the selected text in a text buffer,
 *           or -1.0 if there are multiple font sizes in the selection
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


// Turn on C99 compatibility
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


gdouble get_selection_font_size(GtkTextBuffer *text_buffer, GtkTextView *text_view)
{
	// Local variables
	GtkTextIter				end_iter;
	gfloat					font_size;			// Used for calculating the font size of a character
	gint					font_size_int;		// Used for retrieving the size of a character in a text layer
	gfloat					iter_size;			// The size of the present character
	GtkTextAttributes		*text_attributes;	// Pointer to the attributes for a text layer character
	GtkTextIter				this_iter;


	// Retrieve the selection start and end iters
	gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &this_iter, &end_iter);

	// Determine the font size at the selection start iter
	text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));
	gtk_text_iter_get_attributes(&this_iter, text_attributes);
	font_size_int = pango_font_description_get_size(text_attributes->font);
	font_size = rint(font_size_int / PANGO_SCALE);

	// Unless the start and end iters are at the same place, move the end iter back one so we
	// get an accurate value
	if (FALSE == gtk_text_iter_equal(&this_iter, &end_iter))
	{
		gtk_text_iter_backward_char(&end_iter);
	}

	// Step through the text buffer character by character,
	// checking if the font size has changed
	while (FALSE == gtk_text_iter_equal(&this_iter, &end_iter))
	{
		// Move forward one iter
		gtk_text_iter_forward_char(&this_iter);

		// Get the font size at this new iter
		text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));
		gtk_text_iter_get_attributes(&this_iter, text_attributes);
		font_size_int = pango_font_description_get_size(text_attributes->font);
		iter_size = rint(font_size_int / PANGO_SCALE);

		// If the font size value is different to the original size,
		// then we short circuit the loop and return -1.0
		if (font_size != iter_size)
		{
			return -1.0;
		}
	}

	// We've reached the end of the selection, and the selection font size
	// has been the same the whole way.  We return that font size.
	return font_size;
}

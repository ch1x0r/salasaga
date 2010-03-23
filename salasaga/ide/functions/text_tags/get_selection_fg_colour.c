/*
 * $Id$
 *
 * Salasaga: Returns the foreground colour of the selected text in a text buffer,
 *           or grey if there are multiple foreground colours in the selection
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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


GdkColor *get_selection_fg_colour(GtkTextBuffer *text_buffer, GtkTextView *text_view)
{
	// Local variables
	GtkTextIter			end_iter;
	GdkColor			*fg_colour;				// We store the selected colour in this
	GdkColor			*iter_colour;			// Points to the foreground colour for an iter
	GdkColor			*start_colour;			// Points to the foreground colour for an iter
	GtkTextAppearance	*text_appearance;		// Only used to make pointers easier in the code
	GtkTextAttributes	*text_attributes;		// Pointer to the attributes for a text layer character
	GtkTextIter			this_iter;


	// Allocate memory to hold a foreground colour
	fg_colour = g_slice_new0(GdkColor);

	// Retrieve the selection start and end iters
	gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &this_iter, &end_iter);
	gtk_text_iter_order(&this_iter, &end_iter);

	// If the start iter is not at the start of the text buffer, we move it back one character to get an accurate value
	if (FALSE == gtk_text_iter_is_start(&this_iter))
	{
		gtk_text_iter_backward_char(&this_iter);
	}

	// Unless the start and end iters are at the same place, move the end iter back one so we
	// get an accurate value
	if (FALSE == gtk_text_iter_equal(&this_iter, &end_iter))
	{
		gtk_text_iter_backward_char(&end_iter);
	}

	// Determine the foreground colour at the selection start iter
	text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));
	gtk_text_iter_get_attributes(&this_iter, text_attributes);
	text_appearance = &(text_attributes->appearance);
	start_colour = &(text_appearance->fg_color);
	fg_colour->red = start_colour->red;
	fg_colour->green = start_colour->green;
	fg_colour->blue = start_colour->blue;

	// Step through the text buffer character by character,
	// checking if the foreground colour has changed
	while (FALSE == gtk_text_iter_equal(&this_iter, &end_iter))
	{
		// Move forward one iter
		gtk_text_iter_forward_char(&this_iter);

		// Get the attributes at this new iter
		gtk_text_iter_get_attributes(&this_iter, text_attributes);
		text_appearance = &(text_attributes->appearance);
		iter_colour = &(text_appearance->fg_color);

		// If any of the colour component values are different to the
		// original colour, then we short circuit the loop and return grey
		if ((fg_colour->red != iter_colour->red) ||
		   (fg_colour->green != iter_colour->green) ||
		   (fg_colour->blue != iter_colour->blue))
		{
			fg_colour->red = 60000;
			fg_colour->green = 60000;
			fg_colour->blue = 60000;
			return fg_colour;
		}
	}

	// We've reached the end of the selection, and the selection foreground colour has been
	// the same the whole way.  We return that foreground colour.
	return fg_colour;
}

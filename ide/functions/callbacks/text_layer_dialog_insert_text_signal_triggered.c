/*
 * $Id$
 *
 * Salasaga: Functions called when the "insert-text" signal is triggered in a text layer text view widget
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
#include "../text_tags/text_layer_create_font_size_tag.h"


// Variables shared between both functions in this source file
static gfloat our_font_size = 0.0;


// This callback function determines the desired font characteristics to be applied to newly inserted text
gboolean text_layer_dialog_insert_text_before(GtkTextBuffer *text_buffer, GtkTextIter *cursor_location, gchar *new_text, gint num_chars, text_dialog_widgets *text_widgets)
{
	// Local variables
	GtkTextIter				cursor_iter;
	GtkWidget				*fg_colour_button;
	GtkWidget				*font_face_widget;	// Points to the font face selection widget
	gint					font_size_int;		// Used for retrieving the size of a character in a text layer
	GtkWidget				*font_size_scale;
	GtkTextAttributes		*text_attributes;	// Pointer to the attributes for a text layer character
	GtkWidget				*text_view;


	// Initialisation
	fg_colour_button = text_widgets->font_fg_colour_button;
	font_face_widget = text_widgets->font_face_combo_box;
	font_size_scale = text_widgets->font_size_scale;
	text_view = text_widgets->text_view;

	// If the present cursor location is not at the end of the text buffer, we don't need to run the rest of the function
	if (TRUE == gtk_text_iter_is_end(cursor_location))
	{
		// Move the cursor back to just before the newly inserted text character
		cursor_iter = *cursor_location;
		gtk_text_iter_backward_chars(&cursor_iter, num_chars);

		// Determine the font foreground colour we should apply to the new text, and save it for the "after" function to use

		// Determine the font size we should apply to the new text, and save it for the "after" function to use

		// Determine the font size we should apply to the new text, and save it for the "after" function to use
		text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));
		gtk_text_iter_get_attributes(&cursor_iter, text_attributes);
		font_size_int = pango_font_description_get_size(text_attributes->font);
		our_font_size = rint(font_size_int / PANGO_SCALE);
	}

	return FALSE;
}


// This callback function applies the desired font characteristics to newly inserted text
gboolean text_layer_dialog_insert_text_after(GtkTextBuffer *text_buffer, GtkTextIter *cursor_location, gchar *new_text, gint num_chars, text_dialog_widgets *text_widgets)
{
	// Local variables
	GtkTextIter				selection_start;
	GtkTextTag				*text_size_text_tag;


	// Get the iter at the start of the text
	selection_start = *cursor_location;
	gtk_text_iter_backward_chars(&selection_start, num_chars);

	// * Apply the desired font size to the new text *

	// Create the name of a text tag to match the desired size
	text_size_text_tag = text_layer_create_font_size_tag(our_font_size);

	// Apply the font size to the selected text
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(text_size_text_tag), &selection_start, cursor_location);


	return FALSE;
}

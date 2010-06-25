/*
 * $Id$
 *
 * Salasaga: Functions called when the "insert-text" signal is triggered in a text layer text view widget
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
#include "../global_functions.h"
#include "../text_tags/text_layer_create_colour_tag.h"
#include "../text_tags/text_layer_create_font_size_tag.h"


// Variables shared between both functions in this source file
static GdkColor			our_fg_colour;
static gint				our_font_face = -1;
static gfloat			our_font_size = 0.0;


// This callback function determines the desired font characteristics to be applied to newly inserted text
gboolean text_layer_dialog_insert_text_before(GtkTextBuffer *text_buffer, GtkTextIter *cursor_location, gchar *new_text, gint num_chars, text_dialog_widgets *text_widgets)
{
	// Local variables
	GSList					*applied_tags;		// Receives a list of text tags applied at a position in a text buffer
	GtkTextIter				cursor_iter;
	GtkWidget				*fg_colour_button;
	gboolean				font_face_found;	// Used to track if we've found the font face
	GtkWidget				*font_face_widget;	// Points to the font face selection widget
	gint					*font_ptr;			// Used for calculating the font face of a character
	gint					font_size_int;		// Used for retrieving the size of a character in a text layer
	GtkWidget				*font_size_scale;
	gint					loop_counter;		// Simple counter used in loops
	gint					num_tags;			// Receives the total number of tags applied to a text character
	GdkColor				*start_colour;		// Points to the foreground colour for a text iter
	GtkTextAppearance		*text_appearance;	// Pointers to the text appearance attributes we're reading from
	GtkTextAttributes		*text_attributes;	// Pointer to the attributes for a text layer character
	GtkWidget				*text_view;
	GtkTextTag				*this_tag = NULL;	// Used in a loop for pointing to individual text tags


	// Initialisation
	cursor_iter = *cursor_location;
	fg_colour_button = text_widgets->font_fg_colour_button;
	font_face_widget = text_widgets->font_face_combo_box;
	font_size_scale = text_widgets->font_size_scale;
	text_view = text_widgets->text_view;

	// If the start iter is not at the start of the text buffer, we move it back one character to get an accurate value
	if (FALSE == gtk_text_iter_is_start(cursor_location))
	{
		// Move the cursor back to just before the newly inserted text character
		gtk_text_iter_backward_chars(&cursor_iter, num_chars);
	}

	// Determine the font foreground colour we should apply to the new text, and save it for the "after" function to use
	text_attributes = gtk_text_view_get_default_attributes(GTK_TEXT_VIEW(text_view));
	gtk_text_iter_get_attributes(&cursor_iter, text_attributes);

	text_appearance = &(text_attributes->appearance);
	start_colour = &(text_appearance->fg_color);
	our_fg_colour.red = start_colour->red;
	our_fg_colour.green = start_colour->green;
	our_fg_colour.blue = start_colour->blue;

	// Determine the font face we should apply to the new text, and save it for the "after" function to use
	applied_tags = gtk_text_iter_get_tags(&cursor_iter);
	num_tags = g_slist_length(applied_tags);
	font_face_found = FALSE;
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		this_tag = g_slist_nth_data(applied_tags, loop_counter);
		font_ptr = g_object_get_data(G_OBJECT(this_tag), "font-num");
		if (NULL != font_ptr)
		{
			// * Only font face tags have the "font-num" property, so this is a font face tag *
			our_font_face = *font_ptr;
			font_face_found = TRUE;
		}
	}
	g_slist_free(applied_tags);
	if (FALSE == font_face_found)
	{
		// Couldn't determine the font face to be applied to this text, so we use the default text layer font face
		our_font_face = get_default_text_font_face();
	}

	// Determine the font size we should apply to the new text, and save it for the "after" function to use
	font_size_int = pango_font_description_get_size(text_attributes->font);
	our_font_size = rint(font_size_int / PANGO_SCALE);

	return FALSE;
}


// This callback function applies the desired font characteristics to newly inserted text
gboolean text_layer_dialog_insert_text_after(GtkTextBuffer *text_buffer, GtkTextIter *cursor_location, gchar *new_text, gint num_chars, text_dialog_widgets *text_widgets)
{
	// Local variables
	GtkTextTag				*fg_colour_tag;
	gint					font_face_to_use;
	GtkTextIter				selection_start;
	GtkTextTag				*text_size_text_tag;


	// Get the iter at the start of the text
	selection_start = *cursor_location;
	gtk_text_iter_backward_chars(&selection_start, num_chars);

	// * Apply the desired foreground colour to the selected text *
	fg_colour_tag = text_layer_create_colour_tag(&our_fg_colour);
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(fg_colour_tag), &selection_start, cursor_location);

	// * Apply the desired font face to the new text *
	if (-1 == our_font_face)
	{
		font_face_to_use = get_default_text_font_face();
	} else
	{
		font_face_to_use = our_font_face;
	}
	gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(text_buffer), get_salasaga_font_name(font_face_to_use), &selection_start, cursor_location);

	// * Apply the desired font size to the new text *
	text_size_text_tag = text_layer_create_font_size_tag(our_font_size);
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(text_size_text_tag), &selection_start, cursor_location);

	// * Update the font face selector to reflect the font face used *
	// This is needed for (ie.) when there is no text yet in the buffer, so the font face selector is blank and the default font face is used
	gtk_combo_box_set_active(GTK_COMBO_BOX(text_widgets->font_face_combo_box), font_face_to_use);

	return FALSE;
}

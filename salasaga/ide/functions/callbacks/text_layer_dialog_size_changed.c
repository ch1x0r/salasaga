/*
 * $Id$
 *
 * Salasaga: Function called when the user changes the size in the text layer dialog
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


int text_layer_dialog_size_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	gfloat				font_size;
	GtkWidget			*font_size_button;
	guint				loop_counter;
	guint				num_tags;
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	GString				*tag_name_text_size;
	GtkTextBuffer		*text_buffer;
	gboolean			text_selected;
	GtkTextTag			*text_size_text_tag;
	GtkWidget			*text_view;


	// Initialisation
	font_size_button = text_widgets->font_size_spin_button;
	text_view = text_widgets->text_view;
	tag_name_text_size = g_string_new(NULL);
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// If no text is selected then we skip the rest of the function
	text_selected = gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &selection_start, &selection_end);
	if (FALSE == text_selected)
	{
		return FALSE;
	}

	// Remove all of the existing size tags for the selected text
	num_tags = g_slist_length(text_tags_size_slist);
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(text_buffer), g_slist_nth_data(text_tags_size_slist, loop_counter), &selection_start, &selection_end);
	}

	// Retrieve the requested font size from the size widget
	font_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(font_size_button));

	// Create the name of a text tag to match the desired size
	g_string_printf(tag_name_text_size, "text size %.2f", font_size);
	text_size_text_tag = gtk_text_tag_table_lookup(GTK_TEXT_TAG_TABLE(text_tags_table), tag_name_text_size->str);
	if (NULL == text_size_text_tag)
	{
		// No text tag with the requested size already exists in the tag table, so we create one
		text_size_text_tag = gtk_text_tag_new(tag_name_text_size->str);
		g_object_set(GTK_TEXT_TAG(text_size_text_tag), "size-points", font_size, NULL);

		// Add the new tag to the global text table
		gtk_text_tag_table_add(GTK_TEXT_TAG_TABLE(text_tags_table), GTK_TEXT_TAG(text_size_text_tag));

		// Add the new size tag to the linked list
		text_tags_size_slist = g_slist_prepend(text_tags_size_slist, GTK_TEXT_TAG(text_size_text_tag));
	}

	// Apply the font size to the selected text
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(text_size_text_tag), &selection_start, &selection_end);

	// Free the memory allocated in this function
	g_string_free(tag_name_text_size, TRUE);

	return FALSE;
}

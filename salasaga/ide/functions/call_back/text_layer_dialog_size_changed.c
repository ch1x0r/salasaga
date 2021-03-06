/*
 * $Id$
 *
 * Salasaga: Function called when the user changes the size in the text layer dialog
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
#include "../text_tags/text_layer_create_font_size_tag.h"
#include "text_layer_dialog_validate_buffer_tag_quantity.h"


gboolean text_layer_dialog_size_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	gdouble				font_size;
	GtkWidget			*font_size_scale;
	guint				loop_counter;
	guint				num_tags;
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	GtkTextBuffer		*text_buffer;
	gboolean			text_selected;
	GtkTextTag			*text_size_text_tag;
	GtkWidget			*text_view;


	// Initialisation
	font_size_scale = text_widgets->font_size_scale;
	text_view = text_widgets->text_view;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// If no text is selected then we skip the rest of the function
	text_selected = gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &selection_start, &selection_end);
	if (FALSE == text_selected)
	{
		return FALSE;
	}
	gtk_text_iter_order(&selection_start, &selection_end);

	// Retrieve the requested font size from the size widget
	font_size = gtk_range_get_value(GTK_RANGE(font_size_scale));

	// If no font size is selected, we skip the rest of this function
	if (-1.0 == font_size)
	{
		return FALSE;
	}

	// Remove all of the existing size tags for the selected text
	num_tags = g_slist_length(get_text_tags_size_slist());
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(text_buffer), g_slist_nth_data(get_text_tags_size_slist(), loop_counter), &selection_start, &selection_end);
	}

	// Create the name of a text tag to match the desired size
	text_size_text_tag = text_layer_create_font_size_tag(font_size);

	// Apply the font size to the selected text
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(text_size_text_tag), &selection_start, &selection_end);

	// Reset the colour scheme for the widget
	gtk_widget_modify_fg(GTK_WIDGET(font_size_scale), GTK_STATE_NORMAL, NULL);

	// Validate the correct # of tags are in place for all characters in the buffer
	text_layer_dialog_validate_buffer_tag_quantity(GTK_TEXT_BUFFER(text_buffer));

	return FALSE;
}

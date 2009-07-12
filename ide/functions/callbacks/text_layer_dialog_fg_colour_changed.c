/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses a different foreground colour in the text layer dialog
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
#include "text_layer_create_colour_tag.h"


int text_layer_dialog_fg_colour_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	GdkColor			*fg_colour;
	GtkWidget			*fg_colour_button;
	GtkTextTag			*fg_colour_tag;
	guint				loop_counter;
	guint				num_tags;
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	GtkTextBuffer		*text_buffer;
	gboolean			text_selected;
	GtkWidget			*text_view;


	// Initialisation
	fg_colour_button = text_widgets->font_fg_colour_button;
	text_view = text_widgets->text_view;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// If no text is selected then we skip the rest of the function
	text_selected = gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &selection_start, &selection_end);
	if (FALSE == text_selected)
	{
		return FALSE;
	}

	// Remove all of the existing colour tags for the selected text
	num_tags = g_slist_length(text_tags_fg_colour_slist);
	for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
	{
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(text_buffer), g_slist_nth_data(text_tags_fg_colour_slist, loop_counter), &selection_start, &selection_end);
	}

	// Retrieve the requested colour from the colour button
	fg_colour = g_slice_new0(GdkColor);
	gtk_color_button_get_color(GTK_COLOR_BUTTON(fg_colour_button), fg_colour);

	// Create the text tag for this foreground colour
	fg_colour_tag = text_layer_create_colour_tag(fg_colour);

	// Apply the foreground colour to the selected text
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_TAG(fg_colour_tag), &selection_start, &selection_end);

	return FALSE;
}

/*
 * $Id$
 *
 * Salasaga: Function called when the user changes the font in the text layer dialog
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


gboolean text_layer_dialog_font_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	gint				font_face_val;
	GtkWidget			*font_face_widget;
	gint				loop_counter;
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	GtkTextBuffer		*text_buffer;
	gboolean			text_selected;
	GtkWidget			*text_view;


	// Initialisation
	font_face_widget = text_widgets->font_face_combo_box;
	text_view = text_widgets->text_view;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// If no text is selected then we skip the rest of the function
	text_selected = gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER(text_buffer), &selection_start, &selection_end);
	if (FALSE == text_selected)
	{
		return FALSE;
	}

	// Retrieve the value selected in the font face drop down
	font_face_val = gtk_combo_box_get_active(GTK_COMBO_BOX(font_face_widget));

	// If no specific font is selected, we skip the rest of this function
	if (-1 == font_face_val)
	{
		return FALSE;
	}

	// Remove any existing font face tags from the selected text
	for (loop_counter = 0; loop_counter < FONT_COUNT; loop_counter++)
	{
		gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(text_buffer), text_tags_fonts[loop_counter], &selection_start, &selection_end);
	}

	// Apply the requested font face to the selected text
	gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(text_buffer), salasaga_font_names[font_face_val], &selection_start, &selection_end);

	return FALSE;
}

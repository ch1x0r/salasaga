/*
 * $Id$
 *
 * Salasaga: Function called when the user selects text in the text layer dialog
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
#include "../text_tags/get_selection_fg_colour.h"
#include "../text_tags/get_selection_font_face.h"
#include "../text_tags/get_selection_font_size.h"


gboolean text_layer_dialog_selection_changed(GtkWidget *calling_widget, GtkTextIter *text_iter, GtkTextMark *text_mark, text_dialog_widgets *text_widgets)
{
	// Local variables
	GdkColor			*fg_colour;
	GtkWidget			*fg_colour_button;
	GtkWidget			*font_face_widget;		// Points to the font face selection widget
	gdouble				font_size;				// Get selection font size
	GtkWidget			*font_size_scale;
	const gchar			*mark_name;				// Name of the GtkTextMark that was set
	GdkColor			temp_colour;			// Used to set the colour of the font size slider's text value
	GtkTextBuffer		*text_buffer;
	GtkWidget			*text_view;
	GtkStyle			*widget_style;			// Used to retrieve the colour properties for a widget


	// Initialisation
	fg_colour_button = text_widgets->font_fg_colour_button;
	font_face_widget = text_widgets->font_face_combo_box;
	font_size_scale = text_widgets->font_size_scale;
	text_view = text_widgets->text_view;
	text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));

	// Check if the name of the mark being set was "insert".  If not, we skip the rest of the function
	mark_name = gtk_text_mark_get_name(text_mark);
	if (NULL == mark_name)
	{
		// Marks without a name are very common and not what we're after
		return FALSE;
	}
	if (FALSE == g_str_equal(mark_name, "insert"))
	{
		// It wasn't the "insert" mark being set, so we skip the rest of the function
		return FALSE;
	}

	// Update the font widget with the selection font
	gtk_combo_box_set_active(GTK_COMBO_BOX(font_face_widget), get_selection_font_face(GTK_TEXT_BUFFER(text_buffer)));

	// Update the font size widget with the selection font size
	font_size = get_selection_font_size(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_VIEW(text_view));
	if (-1.0 != font_size)  // -1.0 in the return value is a flag to indicate mixed sizes
	{
		// Reset the colour scheme for the text value part of the font size slider
		gtk_widget_modify_fg(GTK_WIDGET(font_size_scale), GTK_STATE_NORMAL, NULL);

		// Set the value on the slider to the font size of the selected text
		gtk_range_set_value(GTK_RANGE(font_size_scale), font_size);
	}
	else
	{
		// Work out what the colour should be for greying out the font size slider
		widget_style = gtk_rc_get_style(GTK_WIDGET(font_size_scale));
		temp_colour = widget_style->mid[GTK_STATE_INSENSITIVE];

		// Grey out the text value part of the font size slider
		gtk_widget_modify_fg(GTK_WIDGET(font_size_scale), GTK_STATE_NORMAL, &temp_colour);
	}

	// Update the colour button with the selection colour
	fg_colour = get_selection_fg_colour(GTK_TEXT_BUFFER(text_buffer), GTK_TEXT_VIEW(text_view));
	gtk_color_button_set_color(GTK_COLOR_BUTTON(fg_colour_button), fg_colour);
	g_slice_free(GdkColor, fg_colour);  // We no longer need the colour value

	return FALSE;
}

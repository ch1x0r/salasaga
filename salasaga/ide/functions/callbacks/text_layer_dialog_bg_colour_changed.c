/*
 * $Id$
 *
 * Salasaga: Function called when the user chooses a different background colour in the text layer dialog
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


int text_layer_dialog_bg_colour_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	GdkColor				bg_colour;
	GtkWidget				*font_bg_colour_button;
	GtkWidget				*text_view;


	// Initialisation
	font_bg_colour_button = text_widgets->font_bg_colour_button;
	text_view = text_widgets->text_view;

	// Figure out which colour is selected in the widget
	gtk_color_button_get_color(GTK_COLOR_BUTTON(font_bg_colour_button), &bg_colour);

	// Set the new background colour for the text view widget
	gtk_widget_modify_base(GTK_WIDGET(text_view), GTK_STATE_NORMAL, &bg_colour);

	return FALSE;
}

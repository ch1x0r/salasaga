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


int text_layer_dialog_font_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets)
{
	// Local variables
	GtkWidget				*combo_box;
	PangoFontDescription	*font;
	gfloat					font_size;
	GtkWidget				*font_size_button;
	gint					gint_val;
	GString					*temp_gstring;
	GtkWidget				*text_view;


	// Initialisation
	combo_box = text_widgets->font_style_combo_box;
	font_size_button = text_widgets->font_size_spin_button;
	text_view = text_widgets->text_view;
	temp_gstring = g_string_new(NULL);

	// Figure out which values are selected in the combo box and font size box
	gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
	font_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(font_size_button));

	// Apply the font face to the text widget
	switch (gint_val)
	{
		case FONT_DEJAVU_SANS:
			g_string_assign(temp_gstring, "DejaVu Sans");
			break;

		case FONT_DEJAVU_SANS_B:
			g_string_assign(temp_gstring, "DejaVu Sans Bold");
			break;

		case FONT_DEJAVU_SANS_B_O:
			g_string_assign(temp_gstring, "DejaVu Sans Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_C:
			g_string_assign(temp_gstring, "DejaVu Sans Condensed");
			break;

		case FONT_DEJAVU_SANS_C_B:
			g_string_assign(temp_gstring, "DejaVu Sans Condensed Bold");
			break;

		case FONT_DEJAVU_SANS_C_B_O:
			g_string_assign(temp_gstring, "DejaVu Sans Condensed Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_C_O:
			g_string_assign(temp_gstring, "DejaVu Sans Condensed Oblique");
			break;

		case FONT_DEJAVU_SANS_EL:
			g_string_assign(temp_gstring, "DejaVu Sans Extra Light");
			break;

		case FONT_DEJAVU_SANS_MONO:
			g_string_assign(temp_gstring, "DejaVu Sans Mono");
			break;

		case FONT_DEJAVU_SANS_MONO_B:
			g_string_assign(temp_gstring, "DejaVu Sans Mono Bold");
			break;

		case FONT_DEJAVU_SANS_MONO_B_O:
			g_string_assign(temp_gstring, "DejaVu Sans Mono Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_MONO_O:
			g_string_assign(temp_gstring, "DejaVu Sans Mono Oblique");
			break;

		case FONT_DEJAVU_SANS_O:
			g_string_assign(temp_gstring, "DejaVu Sans Oblique");
			break;

		case FONT_DEJAVU_SERIF:
			g_string_assign(temp_gstring, "DejaVu Serif");
			break;

		case FONT_DEJAVU_SERIF_B:
			g_string_assign(temp_gstring, "DejaVu Serif Bold");
			break;

		case FONT_DEJAVU_SERIF_B_I:
			g_string_assign(temp_gstring, "DejaVu Serif Bold Italic");
			break;

		case FONT_DEJAVU_SERIF_C:
			g_string_assign(temp_gstring, "DejaVu Serif Condensed");
			break;

		case FONT_DEJAVU_SERIF_C_B:
			g_string_assign(temp_gstring, "DejaVu Serif Condensed Bold");
			break;

		case FONT_DEJAVU_SERIF_C_B_I:
			g_string_assign(temp_gstring, "DejaVu Serif Condensed Bold Italic");
			break;

		case FONT_DEJAVU_SERIF_C_I:
			g_string_assign(temp_gstring, "DejaVu Serif Condensed Italic");
			break;

		case FONT_DEJAVU_SERIF_I:
			g_string_assign(temp_gstring, "DejaVu Serif Italic");
			break;

		default:
			// The default is just plain Sans
			g_string_assign(temp_gstring, "DejaVu Sans");
			break;
	}

	// Apply the font face and size
	g_string_append_printf(temp_gstring, " %.2f", font_size);
	font = pango_font_description_from_string(temp_gstring->str);

	// Apply the font face to the text view
	gtk_widget_modify_font(text_view, font);

	// Free the memory allocated in this function
	g_string_free(temp_gstring, TRUE);

	return FALSE;
}

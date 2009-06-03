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


int text_layer_dialog_font_changed(GtkComboBox *combo_box, GtkWidget *text_view)
{
	// Local variables
	PangoFontDescription	*font;
	gint					gint_val;


	// Figure out which value is selected in the combo box
	gint_val = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));

	// Apply the font face to the text widget
	switch (gint_val)
	{
		case FONT_DEJAVU_SANS:
			font = pango_font_description_from_string("DejaVu Sans");
			break;

		case FONT_DEJAVU_SANS_B:
			font = pango_font_description_from_string("DejaVu Sans Bold");
			break;

		case FONT_DEJAVU_SANS_B_O:
			font = pango_font_description_from_string("DejaVu Sans Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_C:
			font = pango_font_description_from_string("DejaVu Sans Condensed");
			break;

		case FONT_DEJAVU_SANS_C_B:
			font = pango_font_description_from_string("DejaVu Sans Condensed Bold");
			break;

		case FONT_DEJAVU_SANS_C_B_O:
			font = pango_font_description_from_string("DejaVu Sans Condensed Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_C_O:
			font = pango_font_description_from_string("DejaVu Sans Condensed Oblique");
			break;

		case FONT_DEJAVU_SANS_EL:
			font = pango_font_description_from_string("DejaVu Sans Extra Light");
			break;

		case FONT_DEJAVU_SANS_MONO:
			font = pango_font_description_from_string("DejaVu Sans Mono");
			break;

		case FONT_DEJAVU_SANS_MONO_B:
			font = pango_font_description_from_string("DejaVu Sans Mono Bold");
			break;

		case FONT_DEJAVU_SANS_MONO_B_O:
			font = pango_font_description_from_string("DejaVu Sans Mono Bold Oblique");
			break;

		case FONT_DEJAVU_SANS_MONO_O:
			font = pango_font_description_from_string("DejaVu Sans Mono Oblique");
			break;

		case FONT_DEJAVU_SANS_O:
			font = pango_font_description_from_string("DejaVu Sans Oblique");
			break;

		case FONT_DEJAVU_SERIF:
			font = pango_font_description_from_string("DejaVu Serif");
			break;

		case FONT_DEJAVU_SERIF_B:
			font = pango_font_description_from_string("DejaVu Serif Bold");
			break;

		case FONT_DEJAVU_SERIF_B_I:
			font = pango_font_description_from_string("DejaVu Serif Bold Italic");
			break;

		case FONT_DEJAVU_SERIF_C:
			font = pango_font_description_from_string("DejaVu Serif Condensed");
			break;

		case FONT_DEJAVU_SERIF_C_B:
			font = pango_font_description_from_string("DejaVu Serif Condensed Bold");
			break;

		case FONT_DEJAVU_SERIF_C_B_I:
			font = pango_font_description_from_string("DejaVu Serif Condensed Bold Italic");
			break;

		case FONT_DEJAVU_SERIF_C_I:
			font = pango_font_description_from_string("DejaVu Serif Condensed Italic");
			break;

		case FONT_DEJAVU_SERIF_I:
			font = pango_font_description_from_string("DejaVu Serif Italic");
			break;

		default:
			// The default is just plain Sans
			font = pango_font_description_from_string("DejaVu Sans");
			break;
	}

	// Apply the font face to the text view
	gtk_widget_modify_font(text_view, font);

	return FALSE;
}

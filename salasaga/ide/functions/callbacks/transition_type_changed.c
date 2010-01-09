/*
 * $Id$
 *
 * Salasaga: Function called when the user changes the transition type in a dialog box
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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


gboolean transition_type_changed(GtkWidget *calling_widget, transition_widgets *calling_widgets)
{
	// Local variables
	gint				active_type;


	// Check if the type of the duration selector widget is Immediate or not
	active_type = gtk_combo_box_get_active(GTK_COMBO_BOX(calling_widgets->transition_type));
	if (0 == active_type)
	{
		// The transition is Immediate, so we disable the slider
		gtk_widget_set_sensitive(GTK_WIDGET(calling_widgets->transition_duration_label), FALSE);
		gtk_widget_set_sensitive(GTK_WIDGET(calling_widgets->transition_duration_widget), FALSE);
	} else
	{
		// The transition is not Immediate, so we enable the slider
		gtk_widget_set_sensitive(GTK_WIDGET(calling_widgets->transition_duration_label), TRUE);
		gtk_widget_set_sensitive(GTK_WIDGET(calling_widgets->transition_duration_widget), TRUE);
	}

	return FALSE;
}

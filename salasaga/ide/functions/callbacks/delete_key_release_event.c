/*
 * $Id$
 *
 * Salasaga: Function called when the user presses a key while the film strip has focus
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

// GDK include for the delete key code
#include <gdk/gdkkeysyms.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../slide/slide_delete.h"


void film_strip_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	// Temporary variables
	gint	check_val;


	// Only do this function if we have a front store available and a project loaded
	if ((NULL == front_store) || (FALSE == project_active))
	{
		return;
	}

	// Double check if the event is a key release
	if (GDK_KEY_RELEASE != event->type)
	{
		return;
	}

	// Check if the delete key is the one pressed
	if (GDK_Delete != event->keyval)
	{
		return;
	}

	// Make sure there were no modifier were keys pressed at the same time as the delete key
	check_val = GDK_SHIFT_MASK;
	check_val = event->state & GDK_SHIFT_MASK;
	if (0 != check_val)  // Shift key
	{
		return;
	}

	check_val = GDK_CONTROL_MASK;
	check_val = event->state & GDK_CONTROL_MASK;
	if (0 != check_val)  // Control key
	{
		return;
	}

	// * Disabling the check for the shift key for now, as the NUMLOCK key on my keyboard trips it for some unknown reason *
//	check_val = GDK_META_MASK;
//	check_val = event->state & GDK_META_MASK;
//	if (0 != check_val)  // Alt key
//	{
//		return;
//	}

	// If so, then call the function that removes the present slide from the workspace
	slide_delete();

	return;
}

/*
 * $Id$
 *
 * Salasaga: Function called when the user presses a key in some widgets
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

// GDK include for the delete key code
#include <gdk/gdkkeysyms.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../layer/layer_delete.h"
#include "../other/widget_focus.h"
#include "../slide/slide_delete.h"


gboolean delete_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	// Temporary variables
	gint	check_val;

	// Only do this function if we have a front store available and a project loaded
	if ((NULL == get_front_store()) || (FALSE == get_project_active()))
	{
		return TRUE;
	}

	// Double check if the event is a key release
	if (GDK_KEY_RELEASE != event->type)
	{
		return TRUE;
	}

	// Check if the delete key is the one pressed
	if (GDK_Delete != event->keyval)
	{
		return TRUE;
	}

	// Make sure there were no modifier were keys pressed at the same time as the delete key
	check_val = GDK_SHIFT_MASK;
	check_val = event->state & GDK_SHIFT_MASK;
	if (0 != check_val)  // Shift key
	{
		return TRUE;
	}

	check_val = GDK_CONTROL_MASK;
	check_val = event->state & GDK_CONTROL_MASK;
	if (0 != check_val)  // Control key
	{
		return TRUE;
	}

	// * Disabling the check for the Alt key for now, as the NUMLOCK key on my keyboard trips it for some unknown reason *
//	check_val = GDK_META_MASK;
//	check_val = event->state & GDK_META_MASK;
//	if (0 != check_val)  // Alt key
//	{
//		return FALSE;
//	}

	// If so, then call the function that removes the present slide or layer from the workspace
	switch (get_delete_focus())
	{
		case FOCUS_SLIDE:
			slide_delete();
			break;

		case FOCUS_LAYER:
			layer_delete();
			break;

		default:
			break;
	}

	return TRUE;
}

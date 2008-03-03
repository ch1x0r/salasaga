/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Slide -> Reset slide name from the top menu 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 */


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "create_tooltips.h"


void slide_name_reset(void)
{
	// Frees the string used for storing the present slide name, and sets it to NULL to mark as 'unset'
	if (NULL != ((slide *) current_slide->data)->name)
	{
		g_string_free(((slide *) current_slide->data)->name, TRUE);
		((slide *) current_slide->data)->name = NULL;
	}

	// Recreate the film strip tooltips
	create_tooltips();

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Slide name reset");
	gdk_flush();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.6  2008/03/03 03:04:16  vapour
 * Added status bar feedback message.
 *
 * Revision 1.5  2008/02/04 17:16:21  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.4  2008/01/25 14:21:45  vapour
 * Updated to not free the memory for the existing string if there isn't one.
 *
 * Revision 1.3  2008/01/15 16:19:03  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.2  2007/10/06 11:37:24  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

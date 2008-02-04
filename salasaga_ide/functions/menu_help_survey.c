/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Help -> Survey from the top menu 
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

// Gnome includes
#include <libgnome/gnome-url.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void menu_help_survey(void)
{
#ifndef _WIN32  // Non-windows check
	// Launch a browser window of the Flame Project survey
	gnome_url_show ("http://www.flameproject.org/go/survey", NULL);
#endif
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2008/02/04 17:06:45  vapour
 *  + Removed unnecessary includes.
 *
 * Revision 1.2  2008/01/15 16:19:02  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Flame Project: Function to sound a beep (on user error, etc)
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void sound_beep(void)
{
	// Generate a beep
	gdk_beep();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.7  2008/02/04 08:39:13  vapour
 * Updated to just call the gdk beep function.  Theres a good chance we might be able to just remove this function and replace all references to it with gdk beep directly.
 *
 * Revision 1.6  2008/02/04 03:19:09  vapour
 * Updated to use the gdk sound beep function instead of using our own sound.  Will hopefully work out-of-the-box on Win32 as well.
 *
 * Revision 1.5  2008/01/19 07:03:12  vapour
 * Updated to use better directory separator.
 *
 * Revision 1.4  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:00  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

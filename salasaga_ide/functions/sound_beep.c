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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void sound_beep(void)
{
#ifndef _WIN32
	// Local variables
	GString				*tmp_gstring;			// Temporary GString
	
	// fixme4: Need a non-gnome way of playing sound too, for windows
	tmp_gstring = g_string_new(NULL);
	g_string_assign(tmp_gstring, icon_path->str);
	g_string_printf(tmp_gstring, g_build_path(G_DIR_SEPARATOR_S, tmp_gstring->str, "..", "..", "sounds", "generic.wav", NULL));  // Gee this is an ugly approach 
	gnome_sound_play(tmp_gstring->str);

	// Free local variables
	g_string_free(tmp_gstring, TRUE);
#endif
}


/*
 * History
 * +++++++
 * 
 * $Log$
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

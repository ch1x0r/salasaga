/*
 * $Id$
 *
 * Flame Project: Function to detect collisions between a given coordinate and a GList of boundary boxes
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


// Function to detect collisions between a given coordinate and a GList of boundary boxes
GList *detect_collisions(GList *collision_list, gdouble mouse_x, gdouble mouse_y)
{
	// Local variables
	guint				count_int;
	guint				num_boundaries;

	boundary_box			*boundary;


	// Only do this if we've been given a list of boundary boxes
	if (NULL == boundary_list)
	{
		return NULL;
	}

	// Check if there are any boundaries to detect collisions with
	boundary_list = g_list_first(boundary_list);
	num_boundaries = g_list_length(boundary_list);
	if (0 == num_boundaries)
	{
		//  No boundaries given, so return
		return NULL;
	}

	// Yes there are boundaries to check
	for (count_int = 0; count_int < num_boundaries; count_int++)
	{
		boundary_list = g_list_first(boundary_list);
		boundary = g_list_nth_data(boundary_list, count_int);
		if (TRUE == gdk_region_point_in(boundary->region_ptr, (guint) mouse_x, (guint) mouse_y))
		{
			// Collision detected, so add it to the collision list
			collision_list = g_list_first(collision_list);
			collision_list = g_list_append(collision_list, boundary);
		}
	}

	return collision_list;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.4  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:40:11  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Flame Project: Function that redraws the timeline 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void draw_timeline(void)
{
	// Local variables
	slide				*slide_pointer;				// Points to the presently processing slide

	GList				*tmp_glist;				// Temporary GList


	// If the slide doesn't have a timeline widget constructed for it yet, then make one
	slide_pointer = (slide *) current_slide->data;
	if (NULL == slide_pointer->timeline_widget)
	{
		// Construct the widget used to display the slide in the timeline
		construct_timeline_widget(slide_pointer);
	}

	// Remove the present table from the timeline area, this will also reduce it's reference count by one
	tmp_glist = gtk_container_get_children(GTK_CONTAINER(time_line_container));
	if (NULL != tmp_glist)
	{
		// Remove timeline widget from the container
		gtk_container_remove(GTK_CONTAINER(time_line_container), GTK_WIDGET(tmp_glist->data));
	}

	// Increase the reference count for the timeline widget, so it's not destroyed when this function is next called and it's removed
	g_object_ref(slide_pointer->timeline_widget);

	// Add the timeline widget to the onscreen timeline area
	gtk_container_add(GTK_CONTAINER(time_line_container), GTK_WIDGET(slide_pointer->timeline_widget));

	// Show all of the widgets in the timeline
	gtk_widget_show_all(GTK_WIDGET(time_line_container));
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:13  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

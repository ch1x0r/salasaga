/*
 * $Id$
 *
 * Salasaga: Function that redraws the timeline 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "widgets/time_line.h"


void draw_timeline(void)
{
	// Local variables
	gboolean			return_code_gbool;			// Receives boolean return code
	slide				*slide_pointer;				// Points to the presently processing slide
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist


	// If the slide doesn't have a timeline widget constructed for it yet, then make one
	slide_pointer = (slide *) current_slide->data;
	if (NULL == slide_pointer->timeline_widget)
	{
		// Construct the widget used to display the slide in the timeline
		slide_pointer->timeline_widget = time_line_new();

		// Check if the timeline area already has children
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(time_line_container));
		if (NULL != tmp_glist)
		{
			// Increase the reference count for the timeline widget, so it's not destroyed when it's removed from the container
			g_object_ref(GTK_WIDGET(tmp_glist->data));

			// Remove timeline widget from the container
			gtk_container_remove(GTK_CONTAINER(time_line_container), GTK_WIDGET(tmp_glist->data));
		}
		g_list_free(tmp_glist);

		// Add the timeline widget to the onscreen timeline area
		gtk_container_add(GTK_CONTAINER(time_line_container), GTK_WIDGET(slide_pointer->timeline_widget));

	} else
	{
		// Regenerate the images in the time line
		return_code_gbool = time_line_regenerate_images(slide_pointer->timeline_widget);
		if (FALSE == return_code_gbool)
			display_warning("Error ED359: Could not regenerate the time line images");
	}
	
	// Show all of the widgets in the timeline
	gtk_widget_show_all(GTK_WIDGET(time_line_container));
	if (NULL != slide_pointer->timeline_widget->window)
		gdk_window_invalidate_rect(slide_pointer->timeline_widget->window, &slide_pointer->timeline_widget->allocation, TRUE);
}

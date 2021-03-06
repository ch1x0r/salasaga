/*
 * $Id$
 *
 * Salasaga: Function that redraws the timeline
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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
#include "../global_functions.h"
#include "../dialog/display_warning.h"
#include "../time_line/time_line.h"
#include "../time_line/time_line_get_type.h"
#include "../time_line/time_line_new.h"
#include "../time_line/time_line_regenerate_widget.h"
#include "../time_line/time_line_set_stored_slide_duration.h"
#include "time_line_internal_make_widgets.h"

void draw_timeline(void)
{
	// Local variables
	GString				*message;					// Used to construct message strings
	gboolean			return_code_gbool;			// Receives boolean return code
	slide				*slide_pointer;				// Points to the presently processing slide
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist
	TimeLinePrivate		*priv;						// For holding the Timeline private structure object

	//static GdkGC		*this_gc = NULL;

	// If the slide doesn't have a timeline widget constructed for it yet, then make one

	slide_pointer = get_current_slide_data();
	if (FALSE == IS_TIME_LINE(slide_pointer->timeline_widget))
	{
		// Construct the widget used to display the slide in the timeline
		slide_pointer->timeline_widget = time_line_new();
		time_line_set_stored_slide_duration(slide_pointer->timeline_widget, slide_pointer->duration);
	} else
	{
		// This slide already has a widget, so regenerate it
		return_code_gbool = time_line_regenerate_widget(slide_pointer->timeline_widget);
		if (FALSE == return_code_gbool)
		{
			// For some unknown reason, this time widget isn't able to be regenerated
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED359: %s", _("Error"), _("Could not regenerate the time line widget."));
			display_warning(message->str);
			g_string_free(message, TRUE);

			// Return hopefully without doing more damage
			return;
		}
	}

	// Check if the timeline area already has children
	tmp_glist = gtk_container_get_children(GTK_CONTAINER(get_time_line_container()));
	if (NULL != tmp_glist)
	{
		// Increase the reference count for the timeline widget, so it's not destroyed when it's removed from the container
		g_object_ref(GTK_WIDGET(slide_pointer->timeline_widget));

		// Remove timeline widget from the container
		gtk_container_remove(GTK_CONTAINER(get_time_line_container()), GTK_WIDGET(tmp_glist->data));
	}
	g_list_free(tmp_glist);

	// Add the timeline widget to the onscreen timeline area

	// Getting the private Time Line Instance for various parameters
	priv = TIME_LINE_GET_PRIVATE(slide_pointer->timeline_widget);

	gtk_container_add(GTK_CONTAINER(get_time_line_container()), GTK_WIDGET(slide_pointer->timeline_widget));
	gtk_widget_show_all(GTK_WIDGET(get_time_line_container()));

	if (NULL != GTK_WIDGET(priv->bot_right_evb)->window)
		gdk_window_invalidate_rect(GTK_WIDGET(priv->bot_right_evb)->window, &GTK_WIDGET(priv->bot_right_evb)->allocation, TRUE);
	if (NULL != GTK_WIDGET(priv->bot_left_evb)->window)
		gdk_window_invalidate_rect(GTK_WIDGET(priv->bot_left_evb)->window, &GTK_WIDGET(priv->bot_left_evb)->allocation, TRUE);
}

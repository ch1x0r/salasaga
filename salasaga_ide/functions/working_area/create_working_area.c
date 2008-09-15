/*
 * $Id$
 *
 * Salasaga: Function to create the working area 
 * 
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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
#include "../../externs.h"
#include "working_area_button_press_event.h"
#include "working_area_button_release_event.h"
#include "working_area_expose_event.h"
#include "working_area_motion_notify_event.h"


GtkWidget *create_working_area(GtkWidget *working_frame)
{
	// Purpose: To create the working area
	//
	// Inputs: Takes a pointer to an empty GtkWidget
	//
	// Returns: A pointer to the working area GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *working;
	//		working = create_working_area(working);
	//

	// Create the top working area widget
	// fixme3: How about we remove the scroll bars and use Alt+mouse to drag/move instead?
	working_frame = gtk_scrolled_window_new(NULL, NULL);

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(working_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// Create an aspect frame inside the working area's scrolled window
	working = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1, TRUE);
	gtk_frame_set_shadow_type(GTK_FRAME(working), GTK_SHADOW_NONE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(working_frame), GTK_WIDGET(working));

	// Create the drawing area inside the aspect frame
	main_drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(working), GTK_WIDGET(main_drawing_area));

	// Hook up the signal handlers to the working area
	g_signal_connect(main_drawing_area, "button_press_event", G_CALLBACK(working_area_button_press_event), NULL);
	g_signal_connect(main_drawing_area, "button_release_event", G_CALLBACK(working_area_button_release_event), NULL);
	g_signal_connect(main_drawing_area, "expose_event", G_CALLBACK(working_area_expose_event), NULL);
	g_signal_connect(main_drawing_area, "motion_notify_event", G_CALLBACK(working_area_motion_notify_event), NULL);

	// Ensure we get the signals we want
	gtk_widget_set_events(main_drawing_area, gtk_widget_get_events(main_drawing_area)
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_BUTTON1_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK);

	// Display the working area
	gtk_widget_show_all(GTK_WIDGET(working_frame));

	return working_frame;
}

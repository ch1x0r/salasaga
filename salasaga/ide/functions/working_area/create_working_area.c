/*
 * $Id$
 *
 * Salasaga: Function to create the working area
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
#include "../call_back/delete_key_release_event.h"
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

	// Create an alignment widget, to center the slide in the working area
	GtkWidget *working = gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(working_frame), GTK_WIDGET(working));

	// Create the main drawing area widget
	set_main_drawing_area(gtk_drawing_area_new());
	gtk_widget_set_can_focus(GTK_WIDGET(get_main_drawing_area()), TRUE);
	gtk_container_add(GTK_CONTAINER(working), GTK_WIDGET(get_main_drawing_area()));

	// Hook up signal handlers for mouse events to the working area
	g_signal_connect(get_main_drawing_area(), "button_press_event", G_CALLBACK(working_area_button_press_event), NULL);
	g_signal_connect(get_main_drawing_area(), "button_release_event", G_CALLBACK(working_area_button_release_event), NULL);
	g_signal_connect(get_main_drawing_area(), "expose_event", G_CALLBACK(working_area_expose_event), NULL);
	g_signal_connect(get_main_drawing_area(), "motion_notify_event", G_CALLBACK(working_area_motion_notify_event), NULL);

	// Add a signal handler to the working area, to be called whenever a key is pressed while it is in focus
	g_signal_connect(get_main_drawing_area(), "key-release-event", G_CALLBACK(delete_key_release_event), NULL);

	// Ensure we get the signals we want
	gtk_widget_set_events(get_main_drawing_area(), gtk_widget_get_events(get_main_drawing_area())
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_BUTTON1_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK
		| GDK_KEY_RELEASE_MASK);

	// Display the working area
	gtk_widget_show_all(GTK_WIDGET(working_frame));

	return working_frame;
}

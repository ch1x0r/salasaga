/*
 * $Id$
 *
 * Salasaga: Function to do initial creation of the GDK window (realisation)
 *
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "time_line.h"
#include "time_line_get_type.h"


void time_line_realise(GtkWidget *widget)
{
	// Local variables
	guint				attribute_mask;
	GdkWindowAttr		window_attributes;


	// Safety checks
	g_return_if_fail(NULL != widget);
	g_return_if_fail(IS_TIME_LINE(widget));

	// Set the widget state to "realized"
	GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);

	// Set up the GdkWindow attributes
	window_attributes.colormap = gtk_widget_get_colormap(widget);
	window_attributes.event_mask = gtk_widget_get_events(widget);
	window_attributes.event_mask |= GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK;
	window_attributes.height = widget->allocation.height;
	window_attributes.visual = gtk_widget_get_visual(widget);
	window_attributes.wclass = GDK_INPUT_OUTPUT;
	window_attributes.width = widget->allocation.width;
	window_attributes.window_type = GDK_WINDOW_CHILD;
	window_attributes.x = widget->allocation.x;
	window_attributes.y = widget->allocation.y;
	attribute_mask = GDK_WA_COLORMAP | GDK_WA_VISUAL | GDK_WA_X | GDK_WA_Y;

	// Create the GdkWindow instance
	widget->window = gdk_window_new(widget->parent->window, &window_attributes, attribute_mask);
	widget->style = gtk_style_attach(widget->style, widget->window);
	gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);

	// Ensure GTK forwards events to our widget
	gdk_window_set_user_data(widget->window, TIME_LINE(widget));

	// Display the GdkWindow
	gdk_window_show(widget->window);
}

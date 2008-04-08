/*
 * $Id$
 *
 * Salasaga: Functions for the time line widget
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "time_line.h"

// Macros
#define TIME_LINE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TIME_LINE_TYPE, TimeLinePrivate))

// Private structures
typedef struct _TimeLinePrivate TimeLinePrivate;
struct _TimeLinePrivate
{
	GdkPixbuf			*cached_bg_image;			// Cache of the timeline background image, for drawing upon
	gboolean			cached_bg_valid;			// Flag for whether the timeline background cache image is valid
	gfloat				cursor_position;			// Where in the slide the cursor is positioned (in seconds or part thereof)
	GdkPixmap			*display_buffer;			// The rendered version of the timeline
};


// Function definitions
static gint time_line_expose(GtkWidget *widget, GdkEventExpose *event)
{
	// Local variables
	GdkSegment			lines[4];					// Holds the lines used for drawing
	gint				left, right, top, bottom;	// Holds the line positions


	// Safety check
	g_return_val_if_fail(widget != NULL || event != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Calculate where we are going to draw the lines
	left = 5;
	right = widget->allocation.width - 5;
	top = 5;
	bottom = widget->allocation.height - 5;
	lines[0].x1 = left;
	lines[0].y1 = top;
	lines[0].x2 = right;
	lines[0].y2 = top;
	lines[1].x1 = right;
	lines[1].y1 = top;
	lines[1].x2 = right;
	lines[1].y2 = bottom;
	lines[2].x1 = right;
	lines[2].y1 = bottom;
	lines[2].x2 = left;
	lines[2].y2 = bottom;
	lines[3].x1 = left;
	lines[3].y1 = bottom;
	lines[3].x2 = left;
	lines[3].y2 = top;

	// Wipe the exposed area
	// fixme2: We'll probably update this to refresh from the cached bitmap instead (if its valid) 
	gdk_window_clear_area(widget->window, 0, 0, widget->allocation.width, widget->allocation.height);

	// Draw on the GdkWindow to show this function works
	gdk_draw_segments(widget->window, widget->style->fg_gc[widget->state], lines, 4);

	return TRUE;
}

static void time_line_realise(GtkWidget *widget)
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

static void time_line_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	// Safety check
	g_return_if_fail(allocation != NULL || widget != NULL);
	g_return_if_fail(TIME_LINE(widget));

	// Set the widget position and size
	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget))
	{
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, allocation->width, allocation->height);
	}
}

static void time_line_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
	// Safety check
	g_return_if_fail(requisition != NULL || widget != NULL);
	g_return_if_fail(TIME_LINE(widget));

	// Set the minimum widget size
	// fixme2: Just pulled these initial sizes out of the air
	requisition->width = 150;
	requisition->height = 20;
}

static void time_line_class_init(TimeLineClass *klass)
{
	// Local variables
	GtkWidgetClass		*widget_class;


	// Set the widget functions to be used
	widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->expose_event = time_line_expose;
	widget_class->realize = time_line_realise;
	widget_class->size_allocate = time_line_size_allocate;
	widget_class->size_request = time_line_size_request;

	// Add TimeLinePrivate to the class
	g_type_class_add_private(klass, sizeof(TimeLinePrivate));
}

static void time_line_init(TimeLine *time_line)
{
	// Local variables
	TimeLinePrivate			*priv = TIME_LINE_GET_PRIVATE(time_line);


	// Initialise variable defaults
	priv->cached_bg_image = NULL;
	priv->cached_bg_valid = FALSE;
}

GType time_line_get_type(void)
{
	// Local variables
	static GType		this_type = 0;


	// If we haven't already registered this type, then we do so
	if (0 == this_type)
	{
		static const GTypeInfo this_info =
		{
			sizeof(TimeLineClass),
			NULL,
			NULL,
			(GClassInitFunc) time_line_class_init,
			NULL,
			NULL,
			sizeof(TimeLine),
			0,
			(GInstanceInitFunc) time_line_init,
			NULL
		};
		this_type = g_type_register_static(GTK_TYPE_DRAWING_AREA, "TimeLine", &this_info, 0);
	}

	return this_type;
}

GtkWidget* time_line_new()
{
	return GTK_WIDGET(g_object_new(time_line_get_type(), NULL));
}

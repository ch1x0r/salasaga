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
#include "../display_warning.h"
#include "time_line.h"

// fixme2: Pulled these initial sizes out of the air, they should probably be revisited
#define WIDGET_MINIMUM_HEIGHT	150
#define WIDGET_MINIMUM_WIDTH	500

// * Macros *
#define TIME_LINE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TIME_LINE_TYPE, TimeLinePrivate))

// * Private structures *
typedef struct _TimeLinePrivate TimeLinePrivate;
struct _TimeLinePrivate
{
	GdkPixmap			*cached_bg_image;			// Cache of the timeline background image, for drawing upon
	gboolean			cached_bg_valid;			// Flag for whether the timeline background cache image is valid
	gfloat				cursor_position;			// Where in the slide the cursor is positioned (in seconds or part thereof)
	GdkPixmap			*display_buffer;			// The rendered version of the timeline
};


// * Internal function declarations *
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height);


// * Function definitions *

// Function to return the number of the presently selected timeline layer, or -1 if none
gint time_line_get_selected_layer_num(void)
{
	// Stub function for now
	return 1;
}

// Function to set the presently selected timeline layer to the given one
gboolean time_line_set_selected_layer_num(gint selected_row)
{
	// Stub function for now
	return TRUE;
}

// Function to set the presently selected timeline layer to the background one
gboolean time_line_set_selected_layer_to_bg(void)
{
	// Stub function for now
	return TRUE;
}

// Function to do the actual drawing of the timeline widget onscreen
static gint time_line_expose(GtkWidget *widget, GdkEventExpose *event)
{
	// Local variables
	TimeLinePrivate		*priv;
	static GdkGC		*this_gc = NULL;
	TimeLine			*this_time_line;


	// Safety check
	g_return_val_if_fail(widget != NULL || event != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have a display buffer to refresh from
	if (NULL == priv->display_buffer)
	{
		return TRUE;
	}

	// Create a graphic context if we don't have one already
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Refresh the invalidated area from the local cached version
	gdk_draw_drawable(GDK_DRAWABLE(widget->window), GDK_GC(this_gc),
		GDK_PIXMAP(priv->display_buffer),
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);

	return TRUE;
}

// Function to do initial creation of the GDK window (realisation)
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
	// Local variables
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Safety check
	g_return_if_fail(allocation != NULL || widget != NULL);
	g_return_if_fail(TIME_LINE(widget));

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Re-create the timeline background image and display buffer at the new size
	time_line_internal_create_images(priv, allocation->width, allocation->height);

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
	requisition->width = WIDGET_MINIMUM_WIDTH;
	requisition->height = WIDGET_MINIMUM_HEIGHT;
}

// Class initialiser
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

// Function to create the cached time line background image, and its display buffer 
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height)
{
	// Local variables
	static GdkGC		*bg_image_gc = NULL;
	GdkColormap			*colourmap = NULL;			// Colormap used for drawing
	static GdkGC		*display_buffer_gc = NULL;
	gint				left, right, top, bottom;	// Holds the line positions
	GdkSegment			lines[4];					// Holds the lines used for drawing


	// If we already have a background image, we free it
	if (NULL != priv->cached_bg_image)
	{
		g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
		priv->cached_bg_image = NULL;
	}

	// Create the colourmap
	colourmap = gdk_colormap_get_system();

	// Create the background image
	priv->cached_bg_image = gdk_pixmap_new(NULL, width, height, colourmap->visual->depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->cached_bg_image), GDK_COLORMAP(colourmap));
	if (NULL == priv->cached_bg_image)
	{
		// Creating the background image didn't work
		display_warning("Error ED358: Couldn't create the time line background image!");
		return FALSE;
	}

	// Create a graphic context for the timeline background cache image if we don't have one already
	if (NULL == bg_image_gc)
	{
		bg_image_gc = gdk_gc_new(GDK_DRAWABLE(priv->cached_bg_image));
	}

	// Initialise the background image
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE, 0, 0, width, height);

	// Flag that we now have a valid background cache image
	priv->cached_bg_valid = TRUE;

	// If we already have a display buffer, we free it
	if (NULL != priv->display_buffer)
	{
		g_object_unref(GDK_PIXMAP(priv->display_buffer));
		priv->display_buffer = NULL;
	}

	// Create the display buffer
	priv->display_buffer = gdk_pixmap_new(NULL, width, height, colourmap->visual->depth);
	if (NULL == priv->cached_bg_image)
	{
		// Creating the display buffer image didn't work
		display_warning("Error ED357: Couldn't create the time line display buffer image!");
		return FALSE;
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer), GDK_COLORMAP(colourmap));

	// Create a graphic context for the display buffer image if we don't have one already
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Copy the timeline background image to the display buffer
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc),
		GDK_PIXMAP(priv->cached_bg_image),
		0, 0,
		0, 0,
		width, height);

	// Draw a border around the edge of the widget, to show it's possible
	left = 5;
	right = width - 5;
	top = 5;
	bottom = height - 5;
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
	gdk_gc_set_function(GDK_GC(display_buffer_gc), GDK_INVERT);
	gdk_draw_segments(priv->display_buffer, GDK_GC(display_buffer_gc), lines, 4);
	gdk_gc_set_function(GDK_GC(display_buffer_gc), GDK_COPY);

	return TRUE;
}

// Instance initialiser
static void time_line_init(TimeLine *time_line)
{
	// Local variables
	TimeLinePrivate		*priv;


	// Initialise variable defaults
	priv = TIME_LINE_GET_PRIVATE(time_line);
	priv->cached_bg_valid = FALSE;
	priv->display_buffer = NULL;

	// Call our internal time line function to create the cached background image and display buffer
	time_line_internal_create_images(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);
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

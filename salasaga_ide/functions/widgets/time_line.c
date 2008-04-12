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


// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../display_warning.h"
#include "../draw_handle_box.h"
#include "../draw_timeline.h"
#include "../draw_workspace.h"
#include "../layer_edit.h"
#include "time_line.h"

// fixme2: Pulled these initial sizes out of the air, they should probably be revisited
#define WIDGET_MINIMUM_HEIGHT	150
#define WIDGET_MINIMUM_WIDTH	500

#define ADJUSTMENTS_X	85
#define ADJUSTMENTS_Y	2
#define ADJUSTMENTS_SIZE	10

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
	gboolean			drag_active;				// Tracks whether we have an active mouse drag or not
	gint				guide_line_end;				// The pixel number of the ending guide line
	gint				guide_line_start;			// The pixel number of the starting guide line
	gint				left_border_width;			// Number of pixels in the left border (layer name) area
	gint				pixels_per_second;			// Number of pixels used to display each second
	gint				row_height;					// Number of pixels in each layer row
	gint				selected_layer_num;			// The number of the selected layer
	gfloat				stored_slide_duration;		// The original duration of the slide in seconds
	gint				stored_x;
	gint				stored_y;
	gint				top_border_height;			// Number of pixels in the top border (cursor) area
};


// * Internal function declarations *
gboolean time_line_internal_draw_guide_line(GtkWidget *widget, gint pixel_num);
gboolean time_line_internal_draw_layer_duration(TimeLinePrivate *priv, gint layer_number);
gboolean time_line_internal_draw_layer_info(TimeLinePrivate *priv);
gboolean time_line_internal_draw_layer_name(TimeLinePrivate *priv, gint layer_number);
gboolean time_line_internal_initialise_bg_image(TimeLinePrivate *priv, gint width, gint height);
gboolean time_line_internal_initialise_display_buffer(TimeLinePrivate *priv, gint new_width, gint new_height);
gboolean time_line_internal_invalidate_layer_area(GtkWidget *widget, gint layer_number);
gboolean time_line_internal_redraw_bg_area(TimeLinePrivate *priv, gint x1, gint y1, gint width, gint height);
gboolean time_line_internal_redraw_layer_bg(TimeLinePrivate *priv, gint layer_number);
void time_line_internal_draw_selection_highlight(TimeLinePrivate *priv, gint width);

// * Function definitions *

// Function to return the number of the presently selected timeline layer, or -1 if none
// fixme3: Should this be done as a property instead?
gint time_line_get_selected_layer_num(GtkWidget *widget)
{
	// Local variables
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Safety check
	if (NULL == widget)
	{
		return -1;
	}
	if (FALSE == IS_TIME_LINE(widget))
	{
		return -1;
	}

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Return the internal variable, as requested
	return priv->selected_layer_num;
}

// Function to set the presently selected timeline layer to the given one
// fixme3: Should this be done as a property instead?
gboolean time_line_set_selected_layer_num(GtkWidget *widget, gint selected_row)
{
	// Local variables
	gint				height;
	GtkAllocation		new_allocation;
	GtkAllocation		old_allocation;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;
	gint				x1;
	gint				x2;
	gint				y1;
	gint				y2;


	// Safety check
	if (NULL == widget)
	{
		return -1;
	}
	if (FALSE == IS_TIME_LINE(widget))
	{
		return -1;
	}

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > GTK_WIDGET(widget)->allocation.height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = GTK_WIDGET(widget)->allocation.height;
	}
	if (WIDGET_MINIMUM_WIDTH > GTK_WIDGET(widget)->allocation.width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = GTK_WIDGET(widget)->allocation.width;
	}

	// * Restore the background underneath the existing selection box *

	// Calculate the corner points
	x1 = 0;
	y1 = priv->top_border_height + 1 + (priv->selected_layer_num * priv->row_height);
	x2 = width;
	y2 = priv->row_height - 2;

	// Restore top line segment
	old_allocation.x = x1;
	old_allocation.y = y1;
	old_allocation.width = x2;
	old_allocation.height = 1;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore bottom line segment
	old_allocation.y = y1 + y2;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore left line segment
	old_allocation.y = y1;
	old_allocation.width = 1;
	old_allocation.height = y2;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore right line segment
	old_allocation.x = width - 1;
	time_line_internal_redraw_bg_area(priv, old_allocation.x, old_allocation.y, old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Set the internal variable, as requested
	priv->selected_layer_num = selected_row;

	// Draw a selection box around the newly selected layer
	time_line_internal_draw_selection_highlight(priv, width);

	// * Refresh the widget where the new lines were just drawn  *

	// Update the corner points
	y1 = priv->top_border_height + 1 + (selected_row * priv->row_height);

	// Refresh top line segment
	new_allocation.x = x1;
	new_allocation.y = y1;
	new_allocation.width = x2;
	new_allocation.height = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh bottom line segment
	new_allocation.y = y1 + y2;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh left line segment
	new_allocation.y = y1;
	new_allocation.width = 1;
	new_allocation.height = y2;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh right line segment
	new_allocation.x = width - 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	return TRUE;
}

// Function to do the actual drawing of the timeline widget onscreen
static gint time_line_expose(GtkWidget *widget, GdkEventExpose *event)
{
	// Local variables
	gint				height;
	TimeLinePrivate		*priv;
	static GdkGC		*this_gc = NULL;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	g_return_val_if_fail(widget != NULL || event != NULL, FALSE);
	g_return_val_if_fail(IS_TIME_LINE(widget), FALSE);

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > GTK_WIDGET(widget)->allocation.height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = GTK_WIDGET(widget)->allocation.height;
	}
	if (WIDGET_MINIMUM_WIDTH > GTK_WIDGET(widget)->allocation.width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = GTK_WIDGET(widget)->allocation.width;
	}

	// Ensure we have a display buffer to refresh from
	if (NULL == priv->display_buffer)
	{
		// Create the display buffer
		time_line_internal_initialise_display_buffer(priv, width, height);

		// Draw the layer information
		time_line_internal_draw_layer_info(priv);

		// Highlight the selected row
		time_line_internal_draw_selection_highlight(priv, width);
	}

	// Create a graphic context if we don't have one already
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
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
	gint				height;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	g_return_if_fail(allocation != NULL || widget != NULL);
	g_return_if_fail(TIME_LINE(widget));

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > allocation->height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = allocation->height;
	}
	if (WIDGET_MINIMUM_WIDTH > allocation->width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = allocation->width;
	}

	// Create the background buffer
	time_line_internal_initialise_bg_image(priv, width, height);

	// Create the display buffer
	time_line_internal_initialise_display_buffer(priv, width, height);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv);

	// Highlight the selected row
	time_line_internal_draw_selection_highlight(priv, width);

	// Set the widget position and size
	widget->allocation = *allocation;
	if (GTK_WIDGET_REALIZED(widget))
	{
		gdk_window_move_resize(widget->window, allocation->x, allocation->y, width, height);
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

// Function to redraw the images for an already existing
gboolean time_line_regenerate_images(GtkWidget *widget)
{
	// Local variables
	gint				height;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	gint				width;


	// Safety check
	if (NULL == widget)
	{
		return FALSE;
	}
	if (FALSE == IS_TIME_LINE(widget))
	{
		return FALSE;
	}

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > GTK_WIDGET(widget)->allocation.height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = GTK_WIDGET(widget)->allocation.height;
	}
	if (WIDGET_MINIMUM_WIDTH > GTK_WIDGET(widget)->allocation.width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = GTK_WIDGET(widget)->allocation.width;
	}

	// If the cached background image isn't valid, we need to recreate it
	if (FALSE == priv->cached_bg_valid)
	{
		time_line_internal_initialise_bg_image(priv, width, height);
	}

	// Recreate the display buffer
	time_line_internal_initialise_display_buffer(priv, width, height);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv);

	// Highlight the selected row
	time_line_internal_draw_selection_highlight(priv, width);

	return TRUE;
}

// Function to highlight the layer in a specific row
void time_line_internal_draw_selection_highlight(TimeLinePrivate *priv, gint width)
{
	// Local variables
	const GdkColor		colour_red = {0, 65535, 0, 0 };
	gint8				dash_list[2] = { 3, 3 };
	static GdkGC		*display_buffer_gc = NULL;
	gint				selected_row;
	gint				x1, y1, x2, y2;


	// Create a graphic context for the display buffer image if we don't have one already
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Draw the selection
	selected_row = priv->selected_layer_num;
	x1 = 0;
	y1 = priv->top_border_height + 1 + (selected_row * priv->row_height);
	x2 = width - 1;
	y2 = priv->row_height - 2;
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_red);
	gdk_gc_set_line_attributes(GDK_GC(display_buffer_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_dashes(GDK_GC(display_buffer_gc), 1, dash_list, 2);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE, x1, y1, x2, y2);
}

// Function to draw a horizontal guide line directly on the time line widget
gboolean time_line_internal_draw_guide_line(GtkWidget *widget, gint pixel_num)
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static gint8		dash_list[2] = { 3, 3 };
	static GdkGC		*widget_gc = NULL;
	GtkAllocation		guide_area;					// The area we need to invalidate
	gint				pixmap_height;				// Height of the pixmap in pixels
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Safety check
	if (NULL == widget)
		return FALSE;
	if (FALSE == IS_TIME_LINE(widget))
		return FALSE;

	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(widget->window), GDK_COLORMAP(colourmap));
	}
	if (NULL == widget_gc)
	{
		widget_gc = gdk_gc_new(GDK_DRAWABLE(widget->window));
	}
	gdk_drawable_get_size(GDK_DRAWABLE(widget->window), NULL, &pixmap_height);

	// Set the height related variables
	guide_area.x = pixel_num;
	guide_area.y = 0;
	guide_area.height = pixmap_height;
	guide_area.width = 1;

	// Draw guide line
	gdk_gc_set_rgb_fg_color(GDK_GC(widget_gc), &colour_black);
	gdk_gc_set_line_attributes(GDK_GC(widget_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_dashes(GDK_GC(widget_gc), 1, dash_list, 2);
	gdk_draw_line(GDK_DRAWABLE(widget->window), GDK_GC(widget_gc),
			guide_area.x,
			priv->top_border_height + 1,
			guide_area.x,
			guide_area.height);

	return TRUE;
}

// Function to draw the layer duration onto the display buffer
gboolean time_line_internal_draw_layer_duration(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	const GdkColor		colour_fade = {0, (160 << 8), (160 << 8), (190 << 8) };
	const GdkColor		colour_fully_visible = {0, (200 << 8), (200 << 8), (230 << 8) };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static GdkGC		*display_buffer_gc = NULL;
	gint				layer_height;
	layer				*layer_data;
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				layer_width;
	gint				layer_x;
	gint				layer_y;


	// Initialisation
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer), GDK_COLORMAP(colourmap));
	}
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Select the layer we're working with
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_data = g_list_nth_data(layer_pointer, layer_number);

	// Set the height related variables
	layer_y = priv->top_border_height + (layer_number * priv->row_height) + 2;
	layer_height = priv->row_height - 3;

	// Check if there's a fade in transition for this layer
	if (TRANS_LAYER_FADE == layer_data->transition_in_type)
	{
		// Draw the fade in
		layer_x = priv->left_border_width + (layer_data->start_time * priv->pixels_per_second) + 1;
		layer_width = (layer_data->transition_in_duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fade);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE,
				layer_x, layer_y, layer_width, layer_height - 1);

		// Draw the fully visible duration
		layer_x = priv->left_border_width + ((layer_data->start_time + layer_data->transition_in_duration) * priv->pixels_per_second) + 1;
		layer_width = (layer_data->duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE,
				layer_x, layer_y, layer_width - 1, layer_height - 1);
	} else
	{
		// There's no fade in transition for this layer
		layer_x = priv->left_border_width + (layer_data->start_time * priv->pixels_per_second) + 1;
		layer_width = (layer_data->duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE,
				layer_x, layer_y, layer_width - 1, layer_height - 1);
	}

	// Check if there's a fade out transition for this layer
	if (TRANS_LAYER_FADE == layer_data->transition_out_type)
	{
		// Draw the fade out
		layer_x += (layer_data->duration * priv->pixels_per_second);
		layer_width = (layer_data->transition_out_duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fade);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x, layer_y, layer_width, layer_height);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE,
				layer_x - 1, layer_y, layer_width, layer_height - 1);
	}

	return TRUE;
}

// Function to draw the layer information onto the display buffer
gboolean time_line_internal_draw_layer_info(TimeLinePrivate *priv)
{
	// Local variables
	gint				loop_counter;				// Simple counter used in loops
	gint				num_layers;					// The number of layers in the select slide


	// Draw the layer names and durations
	num_layers = ((slide *) current_slide->data)->num_layers;
	for (loop_counter = 0; loop_counter < num_layers; loop_counter++)
	{
		time_line_internal_draw_layer_name(priv, loop_counter);
		time_line_internal_draw_layer_duration(priv, loop_counter);
	}

	return TRUE;
}

// Function to draw the layer name onto the display buffer
gboolean time_line_internal_draw_layer_name(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	const GdkColor		colour_black = {0, 0, 0, 0 };
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static GdkGC		*display_buffer_gc = NULL;
	static PangoContext *font_context = NULL;
	static PangoFontDescription  *font_description = NULL;
	static PangoLayout	*font_layout = NULL;
	layer				*layer_data;
	GList				*layer_pointer;				// Points to the layers in the selected slide


	// Initialisation
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
		gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer), GDK_COLORMAP(colourmap));
	}
	if (NULL == font_context)
	{
		font_context = gdk_pango_context_get();
	}
	if (NULL == font_layout)
	{
		font_layout = pango_layout_new(font_context);
	}
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}
	if (NULL == font_description)
	{
		font_description = pango_font_description_from_string("Sans");
		pango_layout_set_font_description(font_layout, font_description);
	}

	// Retrieve the layer name string
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_data = g_list_nth_data(layer_pointer, layer_number);
	pango_layout_set_text(font_layout, layer_data->name->str, -1);

	// Draw the text string
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
	gdk_draw_layout(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), 5, ((layer_number + 1) * priv->row_height) - 3, font_layout);

	return TRUE;
}

// Function to invalidate the widget area onscreen that's occupied by a specific row
gboolean time_line_internal_invalidate_layer_area(GtkWidget *widget, gint layer_number)
{
	// Local variables
	GtkAllocation		layer_area;
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;


	// Initialisation
	this_time_line = TIME_LINE(widget);
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Set the height related variables
	layer_area.x = 0;
	layer_area.y = priv->top_border_height + (layer_number * priv->row_height) + 2;
	layer_area.height = priv->row_height - 3;
	layer_area.width = widget->allocation.width;

	// Invalidate the selected area
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &layer_area, TRUE);

	return TRUE;
}

// Function to create the cached time line background image
gboolean time_line_internal_initialise_bg_image(TimeLinePrivate *priv, gint width, gint height)
{
	// Local variables
	static GdkGC		*bg_image_gc = NULL;
	const GdkColor		colour_antique_white_2 = {0, (238 << 8), (223 << 8), (204 << 8) };
	const GdkColor		colour_black = {0, 0, 0, 0 };
	const GdkColor		colour_left_bg_first = {0, (255 << 8), (250 << 8), (240 << 8) };
	const GdkColor		colour_left_bg_second = {0, (253 << 8), (245 << 8), (230 << 8) };
	const GdkColor		colour_main_first = {0, 65535, 65535, 65535 };
	const GdkColor		colour_main_second = {0, 65000, 65000, 65000 };
	const GdkColor		colour_old_lace = {0, (253 << 8), (245 << 8), (230 << 8) };
	const GdkColor		colour_white = {0, 65535, 65535, 65535 };
	GdkColormap			*colourmap = NULL;			// Colormap used for drawing
	gint8				dash_list[2] = { 3, 3 };
	gint				existing_bg_height;			// Height in pixels of an existing pixmap
	gint				existing_bg_width;			// Width in pixels of an existing pixmap
	gboolean			flip_flop = FALSE;			// Used to alternate between colours
	PangoContext		*font_context;
	PangoFontDescription  *font_description;
	PangoLayout			*font_layout;
	gint				font_width;
	gint				loop_counter;				// Simple counter used in loops
	gint				loop_counter2;				// Simple counter used in loops
	gint				loop_max;
	gint				loop_max2;
	GString				*seconds_number;


	// Initialisation
	colourmap = gdk_colormap_get_system();
	font_context = gdk_pango_context_get();
	font_layout = pango_layout_new(font_context);
	g_object_unref(font_context);
	seconds_number = g_string_new(NULL);

	// If we already have a background image, we check if we can re-use it
	if (NULL != priv->cached_bg_image)
	{
		// Retrieve the size of the existing cached background image
		gdk_drawable_get_size(GDK_PIXMAP(priv->cached_bg_image), &existing_bg_width, &existing_bg_height);

		// If we have an existing cached background image of the correct height and width, we re-use it
		if ((existing_bg_width == width) && (existing_bg_height == height))
		{
			return TRUE;
		}

		// The existing cached image isn't usable, so we free it and proceed with creating a new one
		g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
		priv->cached_bg_image = NULL;
		priv->cached_bg_valid = FALSE;
	}

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

	// Draw the background of the top border area
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_old_lace);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE, 0, 0, width, priv->top_border_height);
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), 0, priv->top_border_height, width, priv->top_border_height);

	// Draw the minus symbol
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_white);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE, ADJUSTMENTS_X, ADJUSTMENTS_Y, ADJUSTMENTS_SIZE, ADJUSTMENTS_SIZE);
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), FALSE, ADJUSTMENTS_X, ADJUSTMENTS_Y, ADJUSTMENTS_SIZE, ADJUSTMENTS_SIZE);
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
			ADJUSTMENTS_X + 2, ADJUSTMENTS_Y + 5, ADJUSTMENTS_X + 9, ADJUSTMENTS_Y + 5);  // Horizontal line

	// Draw the plus symbol
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_white);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE, ADJUSTMENTS_X + 15, ADJUSTMENTS_Y, ADJUSTMENTS_SIZE, ADJUSTMENTS_SIZE);
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), FALSE, ADJUSTMENTS_X + 15, ADJUSTMENTS_Y, ADJUSTMENTS_SIZE, ADJUSTMENTS_SIZE);
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
			ADJUSTMENTS_X + 17, ADJUSTMENTS_Y + 5, ADJUSTMENTS_X + 24, ADJUSTMENTS_Y + 5);  // Horizontal line
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
			ADJUSTMENTS_X + 20, ADJUSTMENTS_Y + 2, ADJUSTMENTS_X + 20, ADJUSTMENTS_Y + 9);  // Vertical line

	// Horizontal alternating background rows
	loop_max = height / priv->row_height;
	for (loop_counter = 0; loop_counter <= loop_max; loop_counter++)
	{
		if (TRUE == (flip_flop = !flip_flop))
		{
			// Background for left side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_left_bg_first);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								0, priv->top_border_height + 1 + (loop_counter * priv->row_height), priv->left_border_width, ((loop_counter + 1) * priv->row_height));

			// Background for right side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_main_first);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								priv->left_border_width, priv->top_border_height + 1 + (loop_counter * priv->row_height), width, ((loop_counter + 1) * priv->row_height));
		} else
		{
			// Alternative colour background for left side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_left_bg_second);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								0, priv->top_border_height + 1 + (loop_counter * priv->row_height), priv->left_border_width, ((loop_counter + 1) * priv->row_height));

			// Alternative colour background for right side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_main_second);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								priv->left_border_width, priv->top_border_height + 1 + (loop_counter * priv->row_height), width, ((loop_counter + 1) * priv->row_height));
		}
	}

	// Draw the seconds markings
	font_description = pango_font_description_from_string("Sans, 10px");
	pango_layout_set_font_description(font_layout, font_description);
	loop_max = width / priv->pixels_per_second;
	for (loop_counter = 0; loop_counter <= loop_max; loop_counter++)
	{
		// In the top border area
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
		gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
						priv->left_border_width + (loop_counter * priv->pixels_per_second),
						priv->top_border_height - 5,
						priv->left_border_width + (loop_counter * priv->pixels_per_second),
						priv->top_border_height - 1);

		// The numbers themselves
		g_string_printf(seconds_number, "%ds", loop_counter);
		pango_layout_set_text(font_layout, seconds_number->str, -1);
		pango_layout_get_size(font_layout, &font_width, NULL);
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
		gdk_draw_layout(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
				priv->left_border_width + (loop_counter * priv->pixels_per_second) - ((font_width / PANGO_SCALE) / 2), -2, font_layout);

		// In the main time line area
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_antique_white_2);
		gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_gc_set_dashes(GDK_GC(bg_image_gc), 1, dash_list, 2);
		gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
						priv->left_border_width + (loop_counter * priv->pixels_per_second),
						priv->top_border_height + 1,
						priv->left_border_width + (loop_counter * priv->pixels_per_second),
						height);
	}
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	g_string_free(seconds_number, TRUE);

	// Draw the horizontal layer components
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_antique_white_2);
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	loop_max = height / priv->row_height;
	for (loop_counter = 1; loop_counter <= loop_max; loop_counter++)
	{
		// The half second markings
		loop_max2 = width / priv->pixels_per_second;
		for (loop_counter2 = 0; loop_counter2 <= loop_max2; loop_counter2++)
		{
			gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
							priv->left_border_width + (priv->pixels_per_second >> 1) + (loop_counter2 * priv->pixels_per_second),
							priv->top_border_height + (loop_counter * priv->row_height) - 3,
							priv->left_border_width + (priv->pixels_per_second >> 1) + (loop_counter2 * priv->pixels_per_second),
							priv->top_border_height + (loop_counter * priv->row_height));
		}

		// The solid rows
		gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
						0,
						priv->top_border_height + (loop_counter * priv->row_height),
						width - 1,
						priv->top_border_height + (loop_counter * priv->row_height));
	}

	// Draw the left border area
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), priv->left_border_width, priv->top_border_height, priv->left_border_width, height);

	// Flag that we now have a valid background cache image
	priv->cached_bg_valid = TRUE;

	return TRUE;
}

// Function to create the display buffer
gboolean time_line_internal_initialise_display_buffer(TimeLinePrivate *priv, gint new_width, gint new_height)
{
	// Local variables
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	gint				existing_bg_height;			// Height in pixels of an existing pixmap
	gint				existing_bg_width;			// Width in pixels of an existing pixmap
	gint				height;
	gint				width;


	// Initialisation
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
	}

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > new_height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = new_height;
	}
	if (WIDGET_MINIMUM_WIDTH > new_width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = new_width;
	}

	// If we already have a display buffer, we check if we can reuse it
	if (NULL != priv->display_buffer)
	{
		// Retrieve the size of the existing cached display buffer
		gdk_drawable_get_size(GDK_PIXMAP(priv->display_buffer), &existing_bg_width, &existing_bg_height);

		// If the existing display buffer is not of the same height and width, we discard it
		if ((existing_bg_width != width) || (existing_bg_height != height))
		{
			// The existing display buffer is not of the same height and width
			g_object_unref(GDK_PIXMAP(priv->display_buffer));
			priv->display_buffer = gdk_pixmap_new(NULL, width, height, colourmap->visual->depth);
			if (NULL == priv->cached_bg_image)
			{
				// Couldn't allocate memory for a new display buffer
				display_warning("Error ED360: Couldn't create the time line display buffer image!");
				return FALSE;
			}
		}
	} else
	{
		// Create the display buffer
		priv->display_buffer = gdk_pixmap_new(NULL, width, height, colourmap->visual->depth);
		if (NULL == priv->cached_bg_image)
		{
			// Couldn't allocate memory for a new display buffer
			display_warning("Error ED357: Couldn't create the time line display buffer image!");
			return FALSE;
		}
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer), GDK_COLORMAP(colourmap));

	// Copy the timeline background image to the display buffer
	time_line_internal_redraw_bg_area(priv, 0, 0, width, height);

	return TRUE;
}

// Function to refresh an area of the display buffer from the cached background image
gboolean time_line_internal_redraw_bg_area(TimeLinePrivate *priv, gint x1, gint y1, gint width, gint height)
{
	static GdkGC		*display_buffer_gc = NULL;


	// Initialisation
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Ensure the background image we're about to use is valid
	if (TRUE != priv->cached_bg_valid)
	{
		// It's not, so recreate the timeline background image
		time_line_internal_initialise_bg_image(priv, width, height);
	}

	// Refresh the display buffer for the desired area
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc),
			GDK_PIXMAP(priv->cached_bg_image), x1, y1, x1, y1, width, height);

	return TRUE;	
}

// Function to refresh the area of the display buffer covered by a layer, from the cached background image
gboolean time_line_internal_redraw_layer_bg(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	gint				layer_height;
	gint				layer_width;
	gint				layer_x;
	gint				layer_y;


	// Set the height related variables
	layer_x = 0;
	layer_y = priv->top_border_height + (layer_number * priv->row_height) + 2;
	layer_height = priv->row_height - 3;
	layer_width = -1;

	// Refresh the display buffer for the selected layer
	time_line_internal_redraw_bg_area(priv, layer_x, layer_y, layer_width, layer_height);

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
	priv->drag_active = FALSE;
	priv->selected_layer_num = 0;
	priv->stored_x = 0;
	priv->stored_y = 0;
	priv->guide_line_start = 0;
	priv->guide_line_end = 0;

	// Store the slide duration
	priv->stored_slide_duration = ((slide *) current_slide->data)->duration;

	// fixme3: These would probably be good as properties
	priv->left_border_width = 120;
	priv->pixels_per_second = 60;
	priv->row_height = 20;
	priv->top_border_height = 15;

	// Call our internal time line function to create the cached background image
	time_line_internal_initialise_bg_image(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);

	// Call our internal function to create the display buffer
	time_line_internal_initialise_display_buffer(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv);

	// Select the highlighted layer
	time_line_internal_draw_selection_highlight(priv, WIDGET_MINIMUM_WIDTH);
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

// Callback function for when the user drags the mouse button on the time line widget
void timeline_widget_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	layer				*background_layer_data;		// Data for the background layer
	GdkModifierType		button_state;				// Mouse button states
	gint				current_row;				// The presently selected row
	gint				distance_moved;				// Number of pixels the row has been scrolled by horizontally
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GtkAllocation		area;						// Rectangular area
	GList				*layer_above;				// The layer above the selected one
	GList				*layer_below;				// The layer below the selected one
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	gint				new_row;					// The row the mouse is over
	GList				*selected_layer;			// The selected layer
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	gfloat				time_moved;					// Number of seconds the row is being adjusted by
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist


	// Safety checks
	if (NULL == widget)
	{
		return;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// It's probably the child of the called widget that we need to get data from
	if (FALSE == IS_TIME_LINE(widget))
	{
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(widget));
		if (NULL == tmp_glist)
			return;
		if (FALSE == IS_TIME_LINE(tmp_glist->data))
		{
			g_list_free(tmp_glist);
			return;
		}

		// The child is the TimeLine widget
		this_time_line = TIME_LINE(tmp_glist->data);
	} else
	{
		// This is a time line widget
		this_time_line = TIME_LINE(widget);		
	}

	// Initialisation
	this_slide_data = ((slide *) current_slide->data);
	layer_pointer = this_slide_data->layers;
	end_row = this_slide_data->num_layers -1;
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	current_row = priv->selected_layer_num;

	// Work out which row the mouse is over in the timeline area
	new_row = floor((event->y - priv->top_border_height) / priv->row_height);

	// Retrieve the layer data for the selected row
	layer_pointer = g_list_first(layer_pointer);
	this_layer_data = g_list_nth_data(layer_pointer, current_row);

	// Calculate the present end time of the layer (in seconds)
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Check if we're not already dragging
	if (FALSE == priv->drag_active)
	{
		// If the user is trying to drag outside the valid layers, ignore this event
		if ((0 > new_row) || (new_row >= end_row) || (current_row >= end_row) || (0 > current_row))
			return;

		// We're commencing a drag, so note this
		priv->drag_active = TRUE;

		// Store the mouse coordinates so we know where to drag from
		if (event->x < priv->left_border_width)
		{
			priv->stored_x = priv->left_border_width;
		} else
		{
			priv->stored_x = event->x;
		}
		priv->stored_y = event->y;

		// Remove the old guide lines
		area.x = priv->guide_line_start;
		area.y = 0;
		area.height = GTK_WIDGET(this_time_line)->allocation.height;
		area.width = 1;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*
		area.x = priv->guide_line_end;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*

		// Store the position of the new guide lines so we know where to refresh
		priv->guide_line_start = priv->left_border_width + (this_layer_data->start_time * priv->pixels_per_second);
		priv->guide_line_end = priv->left_border_width + (end_time * priv->pixels_per_second);

		// Draw the new guide lines
		time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_start);
		time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_end);
	} else
	{
		// Clamp the row number to acceptable bounds
		new_row = CLAMP(new_row, 0, end_row - 1);

		// * We are already dragging, so check if the selected row should be moved vertically *
		if (current_row > new_row)
		{
			// * The layer is being moved vertically upwards *

			// Get details of the layers we're moving around
			selected_layer = g_list_nth(layer_pointer, current_row);
			layer_above = g_list_nth(layer_pointer, new_row);

			// Move the row up one in the layer list
			layer_pointer = g_list_remove_link(layer_pointer, selected_layer);
			layer_pointer = g_list_insert_before(layer_pointer, layer_above, selected_layer->data);
			this_slide_data->layers = layer_pointer;

			// Refresh the timeline display of the old row
			time_line_internal_redraw_layer_bg(priv, current_row);
			time_line_internal_draw_layer_name(priv, current_row);
			time_line_internal_draw_layer_duration(priv, current_row);

			// Refresh the timeline display of the new row
			time_line_internal_redraw_layer_bg(priv, new_row);
			time_line_internal_draw_layer_name(priv, new_row);
			time_line_internal_draw_layer_duration(priv, new_row);

			// Update the selected row
			time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

			// Tell the window system to update the new widget areas onscreen
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), new_row);
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);

			// Update the workspace area
			draw_workspace();
		}
		if (current_row < new_row)
		{
			// * The layer is being moved vertically downwards *

			// Get details of the layers we're moving around
			selected_layer = g_list_nth(layer_pointer, current_row);
			layer_below = g_list_nth(layer_pointer, new_row);

			// Move the row down one in the layer list
			layer_pointer = g_list_remove_link(layer_pointer, layer_below);
			layer_pointer = g_list_insert_before(layer_pointer, selected_layer, layer_below->data);
			this_slide_data->layers = layer_pointer;

			// Refresh the timeline display of the old row
			time_line_internal_redraw_layer_bg(priv, current_row);
			time_line_internal_draw_layer_name(priv, current_row);
			time_line_internal_draw_layer_duration(priv, current_row);

			// Refresh the timeline display of the new row
			time_line_internal_redraw_layer_bg(priv, new_row);
			time_line_internal_draw_layer_name(priv, new_row);
			time_line_internal_draw_layer_duration(priv, new_row);

			// Update the selected row
			time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

			// Tell the window system to update the new widget areas onscreen
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), new_row);
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);

			// Update the workspace area
			draw_workspace();
		}

		// * Check if the row should be moved horizontally *
		if ((priv->stored_x != mouse_x) && (mouse_x > priv->left_border_width))
		{
			// Check if the layer is being moved to the right
			if (priv->stored_x > mouse_x)
			{
				// Calculate the time and distance travelled
				mouse_x = CLAMP(mouse_x, priv->left_border_width, GTK_WIDGET(this_time_line)->allocation.width);
				distance_moved = priv->stored_x - mouse_x;
				time_moved = ((gfloat) distance_moved) / priv->pixels_per_second;

				// Update the layer data with the new timing
				if (0 > (this_layer_data->start_time - time_moved))
				{
					end_time = end_time - this_layer_data->start_time;
					this_layer_data->start_time = 0;
				} else
				{
					this_layer_data->start_time -= time_moved;
					end_time -= time_moved;
				}

				// Update the stored position of the row in the widget
				priv->stored_x = mouse_x;

				// Refresh the timeline display of the row
				time_line_internal_redraw_layer_bg(priv, current_row);
				time_line_internal_draw_layer_name(priv, current_row);
				time_line_internal_draw_layer_duration(priv, current_row);

				// Tell the window system to update the current row area onscreen
				time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);
			}

			// Check if the layer is being moved to the left
			if (priv->stored_x < mouse_x)
			{
				// Calculate the time and distance travelled
				mouse_x = CLAMP(mouse_x, priv->left_border_width, GTK_WIDGET(this_time_line)->allocation.width);
				distance_moved = mouse_x - priv->stored_x;
				time_moved = ((gfloat) distance_moved) / priv->pixels_per_second;

				// Update the layer data with the new timing
				this_layer_data->start_time += time_moved;
				end_time += time_moved;

				// Update the stored position of the row in the widget
				priv->stored_x = mouse_x;

				// Refresh the timeline display of the row
				time_line_internal_redraw_layer_bg(priv, current_row);
				time_line_internal_draw_layer_name(priv, current_row);
				time_line_internal_draw_layer_duration(priv, current_row);

				// Tell the window system to update the current row area onscreen
				time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), current_row);
			}
		}

		// Ensure the background layer end is kept correct 
		background_layer_data = g_list_nth_data(layer_pointer, end_row);
		if (background_layer_data->duration != priv->stored_slide_duration)
		{
			background_layer_data->duration = priv->stored_slide_duration;

			// Refresh the timeline display of the background layer
			area.x = priv->stored_slide_duration * priv->pixels_per_second;
			area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
			area.height = priv->row_height - 3;
			area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
			time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
			time_line_internal_draw_layer_duration(priv, end_row);

			// Refresh the newly drawn widget area
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}

		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
			// The new end time is longer than the slide duration, so update the slide and background layer to match
			this_slide_data->duration = end_time;
			background_layer_data->duration = end_time;

			// Refresh the timeline display of the background layer
			area.x = priv->stored_slide_duration * priv->pixels_per_second;
			area.y = priv->top_border_height + (end_row * priv->row_height) + 2;
			area.height = priv->row_height - 3;
			area.width = GTK_WIDGET(this_time_line)->allocation.width - area.x;
			time_line_internal_redraw_bg_area(priv, area.x, area.y, area.width, area.height);
			time_line_internal_draw_layer_duration(priv, end_row);

			// Refresh the newly drawn widget area
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}

		// Remove the old guide lines
		area.x = priv->guide_line_start;
		area.y = 0;
		area.height = GTK_WIDGET(this_time_line)->allocation.height;
		area.width = 1;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*
		area.x = priv->guide_line_end;
		gtk_widget_draw(GTK_WIDGET(widget), &area);  // Yes, this is deprecated, but it *works*

		// Update the guide line positions so we know where to refresh
		priv->guide_line_start = priv->left_border_width + (this_layer_data->start_time * priv->pixels_per_second);
		priv->guide_line_end = priv->left_border_width + (end_time * priv->pixels_per_second);

		// Draw the updated guide lines
		time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_start);
		time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_end);
	}

	return;
}

// Callback function for when the user presses the mouse button on the time line widget
void timeline_widget_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				new_row;					// Used to determine the row clicked upon
	TimeLinePrivate		*priv;
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	TimeLine			*this_time_line;
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist


	// Safety check
	if (NULL == widget)
	{
		return;
	}

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}

	// It's probably the child of the called widget that we need to get data from
	if (FALSE == IS_TIME_LINE(widget))
	{
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(widget));
		if (NULL == tmp_glist)
			return;
		if (FALSE == IS_TIME_LINE(tmp_glist->data))
		{
			g_list_free(tmp_glist);
			return;
		}

		// The child is the TimeLine widget
		this_time_line = TIME_LINE(tmp_glist->data);
	} else
	{
		// This is a time line widget
		this_time_line = TIME_LINE(widget);		
	}

	// Initialisation
	priv = TIME_LINE_GET_PRIVATE(this_time_line);
	this_slide_data = ((slide *) current_slide->data);
	layer_pointer = this_slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);

	// Check if this button press is in the top border area
	if ((ADJUSTMENTS_Y <= event->y) && (ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= event->y)
	{
		// * It's in the correct range *
		
		// For now, we just ignore the button click
		return;
	}

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();
		return;
	}

	// Figure out which row the user has selected in the timeline area
	new_row = floor((event->y - priv->top_border_height) / priv->row_height);

	// Ensure the user clicked on a valid row
	if (0 > new_row)
		return;  // Too low, the user didn't click on a valid row
	if (this_slide_data->num_layers <= new_row)
		return;  // Too high, the user didn't click on a valid row

	// The user clicked on a valid row, so update the selection
	time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), new_row);

	// Calculate the present end time of the layer (in seconds)
	this_layer_data = g_list_nth_data(layer_pointer, new_row);
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;

	// Store the guide line positions so we know where to refresh
	priv->guide_line_start = priv->left_border_width + (this_layer_data->start_time * priv->pixels_per_second);
	priv->guide_line_end = priv->left_border_width + (end_time * priv->pixels_per_second);

	// Draw guide lines
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_start);
	time_line_internal_draw_guide_line(GTK_WIDGET(this_time_line), priv->guide_line_end);

	// Draw a handle box around the newly selected row in the time line area
	draw_handle_box();
}

// Callback function for when the user releases the mouse button on the time line widget
void timeline_widget_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	layer				*background_layer_data;		// Data for the background layer
	GdkModifierType		button_state;				// Mouse button states
	gint				end_row;					// Number of the last layer in this slide
	gfloat				end_time;					// The end time in seconds of the presently selected layer
	GtkAllocation		area;						// Area covered by an individual guide line
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				mouse_x;					// Mouse x position
	gint				mouse_y;					// Mouse x position
	TimeLinePrivate		*priv;
	gboolean			return_code_gbool;			// Receives boolean return codes
	layer				*this_layer_data;			// Data for the presently selected layer
	slide				*this_slide_data;			// Data for the presently selected slide
	TimeLine			*this_time_line;
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist


	// Safety check
	if (NULL == widget)
	{
		return;
	}

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so we return
		return;
	}

	// Find out where the mouse is positioned, and which buttons and modifier keys are down (active)
	gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

	// It's probably the child of the called widget that we need to get data from
	if (FALSE == IS_TIME_LINE(widget))
	{
		tmp_glist = gtk_container_get_children(GTK_CONTAINER(widget));
		if (NULL == tmp_glist)
			return;
		if (FALSE == IS_TIME_LINE(tmp_glist->data))
		{
			g_list_free(tmp_glist);
			return;
		}

		// The child is the TimeLine widget
		this_time_line = TIME_LINE(tmp_glist->data);
	} else
	{
		// This is a time line widget
		this_time_line = TIME_LINE(widget);		
	}

	// Initialisation
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	// Check if this button release is within the vertical range of the adjustment buttons
	if ((ADJUSTMENTS_Y <= mouse_y) && (ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= mouse_y)
	{
		// * It's in the correct range *

		// Check if this button release is for the minus button
		if ((ADJUSTMENTS_X <= mouse_x) && ((ADJUSTMENTS_X + ADJUSTMENTS_SIZE) >= mouse_x))
		{
			// Sanity check
			if (priv->pixels_per_second >= 96)
			{
				// We're already at the acceptable scaling limit, so beep then return
				gdk_beep();
				return;
			}

			// Adjust the number of pixels per second
			priv->pixels_per_second = priv->pixels_per_second * 2;
			g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
			priv->cached_bg_image = NULL;

			// Regenerate the timeline images with the new pixel scale
			return_code_gbool = time_line_internal_initialise_bg_image(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED361: Couldn't recreate time line background image");
				return;
			}
			return_code_gbool = time_line_internal_initialise_display_buffer(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED362: Couldn't recreate time line display buffer");
				return;
			}
			return_code_gbool = time_line_internal_draw_layer_info(priv);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED363: Couldn't redraw the time line layer information");
				return;
			}
			area.x = 0;
			area.y = 0;
			area.width = widget->allocation.width;
			area.height = widget->allocation.height;
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}

		// Check if this button release is for the plus button
		if ((ADJUSTMENTS_X + 15 <= mouse_x) && ((ADJUSTMENTS_X + 15 + ADJUSTMENTS_SIZE) >= mouse_x))
		{
			// Sanity check
			if (priv->pixels_per_second <= 24)
			{
				// We're already at the acceptable scaling limit, so beep then return
				gdk_beep();
				return;
			}

			// Adjust the number of pixels per second
			priv->pixels_per_second = priv->pixels_per_second / 2;
			g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
			priv->cached_bg_image = NULL;

			// Regenerate the timeline images with the new pixel scale
			return_code_gbool = time_line_internal_initialise_bg_image(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED364: Couldn't recreate time line background image");
				return;
			}
			return_code_gbool = time_line_internal_initialise_display_buffer(priv, widget->allocation.width, widget->allocation.height);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED365: Couldn't recreate time line display buffer");
				return;
			}
			return_code_gbool = time_line_internal_draw_layer_info(priv);
			if (FALSE == return_code_gbool)
			{
				display_warning("Error ED366: Couldn't redraw the time line layer information");
				return;
			}
			area.x = 0;
			area.y = 0;
			area.width = widget->allocation.width;
			area.height = widget->allocation.height;
			gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
		}
	}

	// Remove guide lines from the widget
	area.x = priv->guide_line_start;
	area.y = 0;
	area.height = GTK_WIDGET(this_time_line)->allocation.height;
	area.width = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);
	area.x = priv->guide_line_end;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &area, TRUE);

	// Check if this mouse release matches a drag 
	if (TRUE == priv->drag_active)
	{
		// Note that the drag has finished
		priv->drag_active = FALSE;

		// Calculate the end time of the layer (in seconds)
		this_slide_data = (slide *) current_slide->data;
		layer_pointer = this_slide_data->layers;
		layer_pointer = g_list_first(layer_pointer);
		this_layer_data = g_list_nth_data(layer_pointer, priv->selected_layer_num);
		end_time = this_layer_data->start_time + this_layer_data->duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			end_time += this_layer_data->transition_in_duration;
		if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
			end_time += this_layer_data->transition_out_duration;

		// Check if the new end time is longer than the slide duration
		if (end_time > priv->stored_slide_duration)
		{
			// The new slide duration is longer than the old one, so update the slide and background layer to match
			this_slide_data->duration = priv->stored_slide_duration = end_time;
			end_row = this_slide_data->num_layers - 1;
			background_layer_data = g_list_nth_data(layer_pointer, end_row);
			background_layer_data->duration = priv->stored_slide_duration;

			// Refresh the timeline display of the background layer
			time_line_internal_redraw_layer_bg(priv, end_row);
			time_line_internal_draw_layer_name(priv, end_row);
			time_line_internal_draw_layer_duration(priv, end_row);
			time_line_internal_invalidate_layer_area(GTK_WIDGET(this_time_line), end_row);
		}
	}
}

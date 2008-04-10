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
#include "../layer_edit.h"
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
	gint				left_border_width;			// Number of pixels in the left border (layer name) area
	gint				pixels_per_second;			// Number of pixels used to display each second
	gint				row_height;					// Number of pixels in each layer row
	gint				selected_layer_num;			// The number of the selected layer
	gint				top_border_height;			// Number of pixels in the top border (cursor) area
};


// * Internal function declarations *
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height);
gboolean time_line_internal_draw_layer_duration(TimeLinePrivate *priv, gint layer_number);
gboolean time_line_internal_draw_layer_info(TimeLinePrivate *priv, gint width, gint height);
gboolean time_line_internal_draw_layer_name(TimeLinePrivate *priv, gint layer_number);
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
	static GdkGC		*display_buffer_gc = NULL;
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

	// * Restore the background around the existing selection box *

	// Ensure the background image we're about to use is valid
	if (TRUE != priv->cached_bg_valid)
	{
		// It's not, so recreate the timeline background image and display buffer at the new size
		time_line_internal_create_images(priv, width, height);
	}

	// Create a graphic context for the display buffer image if we don't have one already
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// * Restore the display buffer for the old (unselecting) layer *

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
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), GDK_PIXMAP(priv->cached_bg_image),
		old_allocation.x, old_allocation.y,
		old_allocation.x, old_allocation.y,
		old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore bottom line segment
	old_allocation.x = x1;
	old_allocation.y = y1 + y2;
	old_allocation.width = x2;
	old_allocation.height = 1;
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), GDK_PIXMAP(priv->cached_bg_image),
		old_allocation.x, old_allocation.y,
		old_allocation.x, old_allocation.y,
		old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore left line segment
	old_allocation.x = x1;
	old_allocation.y = y1;
	old_allocation.width = 1;
	old_allocation.height = y2;
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), GDK_PIXMAP(priv->cached_bg_image),
		old_allocation.x, old_allocation.y,
		old_allocation.x, old_allocation.y,
		old_allocation.width, old_allocation.height);
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &old_allocation, TRUE);

	// Restore right line segment
	old_allocation.x = width - 1;
	old_allocation.y = y1;
	old_allocation.width = 1;
	old_allocation.height = y2;
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), GDK_PIXMAP(priv->cached_bg_image),
		old_allocation.x, old_allocation.y,
		old_allocation.x, old_allocation.y,
		old_allocation.width, old_allocation.height);
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
	new_allocation.x = x1;
	new_allocation.y = y1 + y2;
	new_allocation.width = x2;
	new_allocation.height = 1;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh left line segment
	new_allocation.x = x1;
	new_allocation.y = y1;
	new_allocation.width = 1;
	new_allocation.height = y2;
	gdk_window_invalidate_rect(GTK_WIDGET(widget)->window, &new_allocation, TRUE);

	// Refresh right line segment
	new_allocation.x = width - 1;
	new_allocation.y = y1;
	new_allocation.width = 1;
	new_allocation.height = y2;
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
		// Draw the layer information
		time_line_internal_draw_layer_info(priv, width, height);

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
	time_line_internal_create_images(priv, width, height);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv, width, height);

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
		time_line_internal_create_images(priv, width, height);
	}

	// Draw the layer information
	time_line_internal_draw_layer_info(priv, width, height);

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

// Function to draw the layer duration onto the display buffer
gboolean time_line_internal_draw_layer_duration(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
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
				layer_x,
				layer_y,
				layer_width,
				layer_height);

		// Draw the fully visible duration
		layer_x = priv->left_border_width + ((layer_data->start_time + layer_data->transition_in_duration) * priv->pixels_per_second) + 1;
		layer_width = (layer_data->duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x,
				layer_y,
				layer_width,
				layer_height);
	} else
	{
		// There's no fade in transition for this layer
		layer_x = priv->left_border_width + (layer_data->start_time * priv->pixels_per_second) + 1;
		layer_width = (layer_data->duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fully_visible);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x,
				layer_y,
				layer_width,
				layer_height);
	}

	// Check if there's a fade out transition for this layer
	if (TRANS_LAYER_FADE == layer_data->transition_out_type)
	{
		// Draw the fade out
		layer_x += (layer_data->duration * priv->pixels_per_second);
		layer_width = (layer_data->transition_out_duration * priv->pixels_per_second);
		gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_fade);
		gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), TRUE,
				layer_x,
				layer_y,
				layer_width,
				layer_height);
	}

	return TRUE;
}

// Function to draw the layer information onto the display buffer
gboolean time_line_internal_draw_layer_info(TimeLinePrivate *priv, gint new_width, gint new_height)
{
	// Local variables
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	static GdkGC		*display_buffer_gc = NULL;
	gint				existing_bg_height;			// Height in pixels of an existing pixmap
	gint				existing_bg_width;			// Width in pixels of an existing pixmap
	gint				height;
	gint				loop_counter;				// Simple counter used in loops
	gint				num_layers;					// The number of layers in the select slide
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

	// Create a graphic context for the display buffer image if we don't have one already
	if (NULL == display_buffer_gc)
	{
		display_buffer_gc = gdk_gc_new(GDK_DRAWABLE(priv->display_buffer));
	}

	// Ensure the background image we're about to use is valid
	if (TRUE != priv->cached_bg_valid)
	{
		// It's not, so recreate the timeline background image and display buffer at the new size
		time_line_internal_create_images(priv, width, height);
	}

	// Copy the timeline background image to the display buffer
	gdk_draw_drawable(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc),
			GDK_PIXMAP(priv->cached_bg_image), 0, 0, 0, 0, width, height);

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

// Function to create the cached time line background image, and its display buffer 
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height)
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

	// Draw the top border area
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_old_lace);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE, 0, 0, width, priv->top_border_height);
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), 0, priv->top_border_height, width, priv->top_border_height);

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

// Instance initialiser
static void time_line_init(TimeLine *time_line)
{
	// Local variables
	TimeLinePrivate		*priv;


	// Initialise variable defaults
	priv = TIME_LINE_GET_PRIVATE(time_line);
	priv->cached_bg_valid = FALSE;
	priv->display_buffer = NULL;
	priv->selected_layer_num = 0;

	// fixme3: These would probably be good as properties
	priv->left_border_width = 120;
	priv->pixels_per_second = 60;
	priv->row_height = 20;
	priv->top_border_height = 15;

	// Call our internal time line function to create the cached background image and display buffer
	time_line_internal_create_images(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);

	// Draw the layer information
	time_line_internal_draw_layer_info(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);

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

// Callback function for when the user presses the mouse button on the time line widget
void timeline_widget_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	gfloat				mouse_y;					// Used to determine the row clicked upon
	TimeLinePrivate		*priv;
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

	// Check if this was a double mouse click.  If it was, open an edit dialog
	if (GDK_2BUTTON_PRESS == event->type)
	{
		// Open an edit dialog
		layer_edit();
		return;
	}

	// In this particular case, it's probably the child of the called widget that we need to get data from
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

	// Figure out which row the user has selected in the timeline area
	mouse_y = floor((event->y - priv->top_border_height) / priv->row_height);

	// Ensure the user clicked on a valid row
	if (0 > mouse_y)
		return;  // Too low, the user didn't click on a valid row
	if (((slide *) current_slide->data)->num_layers <= mouse_y)
		return;  // Too high, the user didn't click on a valid row

	// The user clicked on a valid row, so update the selection
	time_line_set_selected_layer_num(GTK_WIDGET(this_time_line), mouse_y);

	// Draw a handle box around the newly selected row in the time line area
	draw_handle_box();
}

// Callback function for when the user releases the mouse button on the time line widget
void timeline_widget_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// Local variables
	TimeLinePrivate		*priv;
	TimeLine			*this_time_line;
	GList				*tmp_glist;					// Is given a list of child widgets, if any exist


	// Safety check
	if (NULL == widget)
	{
		return;
	}

	// In this particular case, it's probably the child of the called widget that we need to get data from
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

	// Check for primary mouse button
	if (1 != event->button)
	{
		// Not a primary mouse, so return
		return;
	}

	// Initialisation
	priv = TIME_LINE_GET_PRIVATE(this_time_line);

	/* Other button release stuff goes here */
}

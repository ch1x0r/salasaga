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
	gint				left_border_width;			// Number of pixels in the left border (layer name) area
	gint				pixels_per_second;			// Number of pixels used to display each second
	gint				row_height;					// Number of pixels in each layer row
	gint				selected_layer_num;			// The number of the selected layer
	gint				top_border_height;			// Number of pixels in the top border (cursor) area
};


// * Internal function declarations *
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height);
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

	// Set the internal variable, as requested
	priv->selected_layer_num = selected_row;

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

// Function to highlight the layer in a specific row
void time_line_internal_draw_selection_highlight(TimeLinePrivate *priv, gint width)
{
	// Local variables
	GdkColor			colour_red = {0, 65535, 0, 0 };
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
	y2 = ((selected_row + 1) * priv->row_height) - 1;
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_red);
	gdk_gc_set_line_attributes(GDK_GC(display_buffer_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
	gdk_gc_set_dashes(GDK_GC(display_buffer_gc), 1, dash_list, 2);
	gdk_draw_rectangle(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), FALSE, x1, y1, x2, y2);
}

// Function to create the cached time line background image, and its display buffer 
gboolean time_line_internal_create_images(TimeLinePrivate *priv, gint width, gint height)
{
	// Local variables
	static GdkGC		*bg_image_gc = NULL;
	GdkColor			colour_antique_white_2 = {0, (238 << 8), (223 << 8), (204 << 8) };
	GdkColor			colour_black = {0, 0, 0, 0 };
	GdkColor			colour_fade = {0, (160 << 8), (160 << 8), (190 << 8) };
	GdkColor			colour_fully_visible = {0, (200 << 8), (200 << 8), (230 << 8) };
	GdkColor			colour_left_bg_first = {0, (255 << 8), (250 << 8), (240 << 8) };
	GdkColor			colour_left_bg_second = {0, (253 << 8), (245 << 8), (230 << 8) };
	GdkColor			colour_main_first = {0, 65535, 65535, 65535 };
	GdkColor			colour_main_second = {0, 65000, 65000, 65000 };
	GdkColor			colour_old_lace = {0, (253 << 8), (245 << 8), (230 << 8) };
	GdkColormap			*colourmap = NULL;			// Colormap used for drawing
	gint8				dash_list[2] = { 3, 3 };
	static GdkGC		*display_buffer_gc = NULL;
	gboolean			flip_flop = FALSE;			// Used to alternate between colours
	PangoContext		*font_context;
	PangoFontDescription  *font_description;
	gint				font_height;
	PangoLayout			*font_layout;
	gint				font_width;
	layer				*layer_data;
	gint				layer_height;
	GList				*layer_pointer;				// Points to the layers in the selected slide
	gint				layer_width;
	gint				layer_x;
	gint				layer_y;
	gint				loop_counter;				// Simple counter used in loops
	gint				loop_counter2;				// Simple counter used in loops
	gint				loop_max;
	gint				loop_max2;
	gint				num_layers;					// The number of layers in the select slide
	GString				*seconds_number;


	// If we already have a background image, we free it
	if (NULL != priv->cached_bg_image)
	{
		g_object_unref(GDK_PIXMAP(priv->cached_bg_image));
		priv->cached_bg_image = NULL;
	}

	// Initialisation
	colourmap = gdk_colormap_get_system();
	font_context = gdk_pango_context_get();
	font_layout = pango_layout_new(font_context);
	g_object_unref(font_context);
	seconds_number = g_string_new(NULL);

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


	// * Draw initial objects on the background image *

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
		pango_layout_get_size(font_layout, &font_width, &font_height);
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

	// * Display buffer image stuff now *

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

	// Display the layer names
	font_description = pango_font_description_from_string("Sans");
	pango_layout_set_font_description(font_layout, font_description);
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	num_layers = ((slide *) current_slide->data)->num_layers;
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
	for (loop_counter = 0; loop_counter < num_layers; loop_counter++)
	{
		layer_data = g_list_nth_data(layer_pointer, loop_counter);
		pango_layout_set_text(font_layout, layer_data->name->str, -1);
		gdk_draw_layout(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), 5, (loop_counter + 1) * priv->row_height, font_layout);
	}

	// Draw the layer durations
	for (loop_counter = 0; loop_counter < num_layers; loop_counter++)
	{
		// Select the layer we're working with 
		layer_data = g_list_nth_data(layer_pointer, loop_counter);

		layer_y = priv->top_border_height + (loop_counter * priv->row_height) + 2;
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
			// There's no transition in for this layer
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
	}

	// Select the highlighted layer
	time_line_internal_draw_selection_highlight(priv, width);

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

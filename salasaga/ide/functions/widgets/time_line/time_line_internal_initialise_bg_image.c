/*
 * $Id$
 *
 * Salasaga: Create the cached time line background image
 *
 * Copyright (C) 2005-2009 Justin Clift <justin@salasaga.org>
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
#include "../../../salasaga_types.h"
#include "../../../externs.h"
#include "time_line.h"
#include "time_line_get_left_border_width.h"
#include "../../dialog/display_warning.h"


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
	GdkColormap			*colourmap = NULL;			// Colourmap used for drawing
	gint8				dash_list[2] = { 3, 3 };
	gint				existing_bg_height;			// Height in pixels of an existing pixmap
	gint				existing_bg_width;			// Width in pixels of an existing pixmap
	gboolean			flip_flop = FALSE;			// Used to alternate between colours
	PangoContext		*font_context;
	PangoFontDescription  *font_description;
	PangoLayout			*font_layout;
	gint				font_width;
	gint				left_border;
	gint				loop_counter;				// Simple counter used in loops
	gint				loop_counter2;				// Simple counter used in loops
	gint				loop_max;
	gint				loop_max2;
	GString				*message;					// Used to construct message strings
	GString				*seconds_number;


	// Initialisation
	colourmap = gdk_colormap_get_system();
	font_context = gdk_pango_context_get();
	font_layout = pango_layout_new(font_context);
	g_object_unref(font_context);
	seconds_number = g_string_new(NULL);
	left_border = time_line_get_left_border_width(priv);

	// If we already have a background image, we check if we can re-use it
	if (NULL != priv->cached_bg_image)
	{
		// Retrieve the size of the existing cached background image
		gdk_drawable_get_size(GDK_PIXMAP(priv->cached_bg_image), &existing_bg_width, &existing_bg_height);

		// If we have an existing cached background image of the correct height and width, we re-use it
		if ((existing_bg_width == width) && (existing_bg_height == height))
		{
			g_string_free(seconds_number, TRUE);
			g_object_unref(font_layout);
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
		message = g_string_new(NULL);
		g_string_printf(message, "%s ED358: %s", _("Error"), _("Couldn't create the time line background image."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		g_string_free(seconds_number, TRUE);
		g_object_unref(font_layout);
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
								0, priv->top_border_height + 1 + (loop_counter * priv->row_height), left_border, ((loop_counter + 1) * priv->row_height));

			// Background for right side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_main_first);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								left_border, priv->top_border_height + 1 + (loop_counter * priv->row_height), width, ((loop_counter + 1) * priv->row_height));
		} else
		{
			// Alternative colour background for left side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_left_bg_second);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								0, priv->top_border_height + 1 + (loop_counter * priv->row_height), left_border, ((loop_counter + 1) * priv->row_height));

			// Alternative colour background for right side
			gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_main_second);
			gdk_draw_rectangle(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), TRUE,
								left_border, priv->top_border_height + 1 + (loop_counter * priv->row_height), width, ((loop_counter + 1) * priv->row_height));
		}
	}

	// Draw the seconds markings
	font_description = pango_font_description_from_string("Sans, 10px");
	pango_layout_set_font_description(font_layout, font_description);
	loop_max = width / time_line_get_pixels_per_second();
	for (loop_counter = 0; loop_counter <= loop_max; loop_counter++)
	{
		// In the top border area
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
		gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
						left_border + (loop_counter * time_line_get_pixels_per_second()),
						priv->top_border_height - 5,
						left_border + (loop_counter * time_line_get_pixels_per_second()),
						priv->top_border_height - 1);

		// The numbers themselves
		g_string_printf(seconds_number, "%ds", loop_counter);
		pango_layout_set_text(font_layout, seconds_number->str, -1);
		pango_layout_get_size(font_layout, &font_width, NULL);
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_black);
		gdk_draw_layout(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
				left_border + (loop_counter * time_line_get_pixels_per_second()) - ((font_width / PANGO_SCALE) / 2), -2, font_layout);

		// In the main time line area
		gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_antique_white_2);
		gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_gc_set_dashes(GDK_GC(bg_image_gc), 1, dash_list, 2);
		gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
						left_border + (loop_counter * time_line_get_pixels_per_second()),
						priv->top_border_height + 1,
						left_border + (loop_counter * time_line_get_pixels_per_second()),
						height);
	}
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	pango_font_description_free(font_description);
	g_object_unref(font_layout);
	g_string_free(seconds_number, TRUE);

	// Draw the horizontal layer components
	gdk_gc_set_rgb_fg_color(GDK_GC(bg_image_gc), &colour_antique_white_2);
	gdk_gc_set_line_attributes(GDK_GC(bg_image_gc), 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	loop_max = height / priv->row_height;
	for (loop_counter = 1; loop_counter <= loop_max; loop_counter++)
	{
		// The half second markings
		loop_max2 = width / time_line_get_pixels_per_second();
		for (loop_counter2 = 0; loop_counter2 <= loop_max2; loop_counter2++)
		{
			gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc),
							left_border + (time_line_get_pixels_per_second() >> 1) + (loop_counter2 * time_line_get_pixels_per_second()),
							priv->top_border_height + (loop_counter * priv->row_height) - 3,
							left_border + (time_line_get_pixels_per_second() >> 1) + (loop_counter2 * time_line_get_pixels_per_second()),
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
	gdk_draw_line(GDK_DRAWABLE(priv->cached_bg_image), GDK_GC(bg_image_gc), left_border, priv->top_border_height, left_border, height);

	// Flag that we now have a valid background cache image
	priv->cached_bg_valid = TRUE;

	return TRUE;
}

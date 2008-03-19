/*
 * $Id$
 *
 * Salasaga: Creates a horizontal image showing the duration of a slide  
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"


GdkPixbuf *create_timeline_slider(GdkPixbuf *output_pixbuf, gint total_width, gint total_height, gint duration_start, gint duration_width)
{
	// Local variables
	GdkColormap			*drawable_colormap;			// Used for adjusting the system colormap
	GdkDrawable			*layer_drawable;			// Points to slider images as they're created
	GdkGC				*layer_graphics_context;		// Used for working with graphics contexts
	GdkColor			slider_bg;				// Backgroud color of sliders
	GdkColor			slider_fg;				// Foreground color of sliders


	// Set up the colors used for drawing the slider
	drawable_colormap = gdk_colormap_get_system();
	if (NULL == drawable_colormap) display_warning("Error ED87: No colormap exists in system\n");
	slider_bg.pixel = 0;
	slider_bg.red = 32000;
	slider_bg.green = 32000;
	slider_bg.blue = 32000;
	if (TRUE != gdk_colormap_alloc_color(GDK_COLORMAP(drawable_colormap), &slider_bg, FALSE, TRUE))
	{
		printf("Error ED88: Unable to slider background color\n");
	}
	slider_fg.pixel = 0;
	slider_fg.red = 48000;
	slider_fg.green = 48000;
	slider_fg.blue = 48000;
	if (TRUE != gdk_colormap_alloc_color(GDK_COLORMAP(drawable_colormap), &slider_fg, FALSE, TRUE))
	{
		printf("Error ED89: Unable to set slider internal color\n");
	}

	// * Create a GdkDrawable displaying the duration of the layer in the slide *
	layer_drawable = gdk_pixmap_new(NULL, total_width, total_height, 24);
	gdk_drawable_set_colormap(GDK_DRAWABLE(layer_drawable), GDK_COLORMAP(drawable_colormap));
	layer_graphics_context = gdk_gc_new(GDK_DRAWABLE(layer_drawable));
	gdk_gc_set_fill(layer_graphics_context, GDK_SOLID);
	gdk_gc_set_line_attributes(layer_graphics_context, 1, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);

	// Draw the background of the slider as grey
	gdk_gc_set_rgb_fg_color(layer_graphics_context, &slider_bg);
	gdk_draw_rectangle(GDK_DRAWABLE(layer_drawable), layer_graphics_context, TRUE, 0, 0, total_width, total_height);

	// Set the contrast color for the duration slider
	gdk_gc_set_rgb_fg_color(layer_graphics_context, &slider_fg);
	gdk_draw_rectangle(GDK_DRAWABLE(layer_drawable), layer_graphics_context, TRUE, duration_start, 2, duration_width, total_height - 4);

	// Convert drawable to GdkPixbuf
	output_pixbuf = gdk_pixbuf_get_from_drawable(output_pixbuf, layer_drawable, NULL, 0, 0, 0, 0, -1, -1);
	if (NULL == output_pixbuf) display_warning("Error ED86: Could not allocate GdkPixbuf\n");

	return output_pixbuf;
}

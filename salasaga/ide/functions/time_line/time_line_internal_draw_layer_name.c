/*
 * $Id$
 *
 * Salasaga: Draw the layer name onto the display buffer
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
#include "../global_functions.h"
#include "time_line.h"


gboolean time_line_internal_draw_layer_name(TimeLinePrivate *priv, gint layer_number)
{
	// Local variables
	GdkRectangle		clip_region;				// Used as a clip mask region
	const GdkColor		colour_black = {0, 0, 0, 0 };
	static GdkColormap	*colourmap = NULL;			// Colourmap used for drawing
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
	layer_pointer = get_current_slide_layers_pointer();
	layer_pointer = g_list_first(layer_pointer);
	layer_data = g_list_nth_data(layer_pointer, layer_number);
	pango_layout_set_text(font_layout, layer_data->name->str, -1);

	// Set a clip mask
	clip_region.x = 0;
	clip_region.y = priv->top_border_height + (layer_number * priv->row_height);
	clip_region.width = priv->left_border_width - 1;
	clip_region.height = priv->row_height;
	gdk_gc_set_clip_rectangle(GDK_GC(display_buffer_gc), &clip_region);

	// Draw the text string
	gdk_gc_set_rgb_fg_color(GDK_GC(display_buffer_gc), &colour_black);
	gdk_draw_layout(GDK_DRAWABLE(priv->display_buffer), GDK_GC(display_buffer_gc), 5, ((layer_number + 1) * priv->row_height) - 3, font_layout);

	return TRUE;
}

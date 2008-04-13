/*
 * $Id$
 *
 * Salasaga: Function to process the data for a layer, adding it to the main image being built 
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

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
#include "draw_highlight_box.h"


void compress_layers_inner(layer *this_layer_data, GdkPixbuf *tmp_pixbuf, gfloat time_position)
{
	// Local variables
	gfloat					end_time;				// Time in seconds of the layer objects finish time
	gint					finish_x;				// X position at the layer objects finish time
	gint					finish_y;				// Y position at the layer objects finish time 
	PangoFontDescription	*font_description;		// Pango font description used for text rendering
	GdkGC					*graphics_context;		// GDK graphics context
	gint					height;					//
	GdkScreen				*output_screen;			//
	PangoContext			*pango_context;			// Pango context used for text rendering
	gint					pango_height;			// Height of the Pango layout
	PangoLayout				*pango_layout;			// Pango layout used for text rendering
	PangoMatrix				pango_matrix = PANGO_MATRIX_INIT;  // Required for positioning the pango layout
	PangoRenderer			*pango_renderer;		// Pango renderer
	gint					pango_width;			// Width of the Pango layout
	gint					pixbuf_height;			// Height of the backing pixbuf
	gint					pixbuf_width;			// Width of the backing pixbuf
	gfloat					scaled_height_ratio;	// Used to calculate a vertical scaling ratio 
	gfloat					scaled_width_ratio;		// Used to calculate a horizontal scaling ratio
	gfloat					start_time;				// Time in seconds of the layer objects start time
	gint					start_x;				// X position at the layer objects start time
	gint					start_y;				// Y position at the layer objects start time
	GtkTextIter				text_end;				// The end position of the text buffer
	GtkTextIter				text_start;				// The start position of the text buffer
	gint					time_alpha = 255;		// Alpha value to use at our desired point in time (defaulting to 255 = fall opacity)
	gfloat					time_offset;
	gint					time_x;					// Unscaled X position of the layer at our desired point in time
	gint					time_y;					// Unscaled Y position of the layer at our desired point in time
	gfloat					time_diff;				// Used when calculating the object position at the desired point in time
	gfloat					time_scale;				// Used when calculating the object alpha value at the desired point in time
	GdkColormap				*tmp_colormap;			// Temporary colormap
	GdkPixmap				*tmp_pixmap;			// GDK Pixmap
	gint					width;					//
	gint					x_diff;					// Used when calculating the object position at the desired point in time
	gint					x_offset;				// X coordinate of the object at the desired point in time
	gfloat					x_scale;				// Used when calculating the object position at the desired point in time
	gint					y_diff;					// Used when calculating the object position at the desired point in time
	gint					y_offset;				// Y coordinate of the object at the desired point in time
	gfloat					y_scale;				// Used when calculating the object position at the desired point in time


	// Is this layer invisible, or is it a background layer?
	if ((FALSE == this_layer_data->visible) || (TRUE == this_layer_data->background))
	{
		// We don't need to process this layer
		return;
	}

	// Simplify pointers
	finish_x = this_layer_data->x_offset_finish;
	finish_y = this_layer_data->y_offset_finish;
	start_time = this_layer_data->start_time;
	start_x = this_layer_data->x_offset_start;
	start_y = this_layer_data->y_offset_start;

	// If the layer isn't visible at the requested time, we don't need to proceed
	end_time = this_layer_data->start_time + this_layer_data->duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
		end_time += this_layer_data->transition_in_duration;
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
		end_time += this_layer_data->transition_out_duration;
	if ((time_position < start_time) || (time_position > end_time))
	{
		return;
	}

	// Calculate how far into the layer movement we are
	time_offset = time_position - start_time;
	time_diff = end_time - start_time;
	x_diff = finish_x - start_x;
	x_scale = (((gfloat) x_diff) / time_diff);
	time_x = start_x + (x_scale * time_offset);
	y_diff = finish_y - start_y;
	y_scale = (((gfloat) y_diff) / time_diff);
	time_y = start_y + (y_scale * time_offset);

	// If the time position is during a transition, we need to determine the correct alpha value
	if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
	{
		// There is a transition in, so work out if the time position is during it
		end_time = start_time + this_layer_data->transition_in_duration;
		if ((time_position > start_time) && (time_position < end_time))
		{
			// The time position is during a transition in
			time_diff = end_time - start_time;
			time_scale = time_offset / time_diff;
			time_alpha = CLAMP(roundf(time_scale * 256), 0, 255);
		}
	}
	if (TRANS_LAYER_NONE != this_layer_data->transition_out_type)
	{
		// There is a transition out, so work out if the time position is during it
		if (TRANS_LAYER_NONE != this_layer_data->transition_in_type)
			start_time += this_layer_data->transition_in_duration;
		start_time += this_layer_data->duration;
		time_offset = time_position - start_time;
		end_time = start_time + this_layer_data->transition_out_duration;
		if ((time_position > start_time) && (time_position < end_time))
		{
			// The time position is during a transition out
			time_diff = end_time - start_time;
			time_scale = time_offset / time_diff;
			time_alpha = CLAMP(roundf(256 - (time_scale * 256)), 0, 255);
		}
	}

	// Calculate the height and width scaling values for the requested layer size
	pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
	pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
	scaled_height_ratio = (gfloat) pixbuf_height / (gfloat) project_height;
	scaled_width_ratio = (gfloat) pixbuf_width / (gfloat) project_width;

	// Determine the type of layer we're dealing with (we ignore background layers)
	switch (this_layer_data->object_type)
	{
		case TYPE_GDK_PIXBUF:

			// * We're processing an image layer *

			// Calculate how much of the source image will fit onto the backing pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_image *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_image *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the image onto the backing pixbuf
			gdk_pixbuf_composite(((layer_image *) this_layer_data->object_data)->image_data,	// Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			scaled_width_ratio,				// Scale X
			scaled_height_ratio,			// Scale Y
			GDK_INTERP_TILES,				// Scaling type
			time_alpha);					// Alpha

			// All done
			return;

		case TYPE_MOUSE_CURSOR:

			// * Composite the mouse pointer image onto the backing pixmap *

			// If the mouse pointer image wasn't able to be loaded then we skip this layer, as the compositing wont work with it
			if (NULL == mouse_ptr_pixbuf)
			{
				return;
			}

			// Calculate how much of the source image will fit onto the backing pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_mouse *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_mouse *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the mouse pointer onto the backing pixbuf
			gdk_pixbuf_composite(mouse_ptr_pixbuf,  // Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			scaled_width_ratio,				// Scale X
			scaled_height_ratio,			// Scale Y
			GDK_INTERP_TILES,				// Scaling type
			time_alpha);					// Alpha
			return;

		case TYPE_EMPTY:

			// Empty layer, just return
			return;

		case TYPE_TEXT:

			// * Draw the text layer *

			// Prepare the text
			pango_context = gdk_pango_context_get();
			pango_layout = pango_layout_new(pango_context);
			font_description = pango_context_get_font_description(pango_context);
			pango_font_description_set_size(font_description,
				PANGO_SCALE * ((layer_text *) this_layer_data->object_data)->font_size * scaled_height_ratio);
			pango_font_description_set_family(font_description, "Bitstream Vera Sans");  // "Sans", "Serif", "Monospace"
			pango_font_description_set_style(font_description, PANGO_STYLE_NORMAL);
			pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
			pango_font_description_set_weight(font_description, PANGO_WEIGHT_NORMAL);
			pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);
			gtk_text_buffer_get_bounds(((layer_text *) this_layer_data->object_data)->text_buffer, &text_start, &text_end);
			pango_layout_set_markup(pango_layout,
				gtk_text_buffer_get_text(((layer_text *) this_layer_data->object_data)->text_buffer, &text_start, &text_end, FALSE),  // Point to the text in the object buffer
				-1);	  // Auto calculate string length

			// * Draw a background for the text layer *

			// Work out how big the rendered text will be
			pango_layout_get_size(pango_layout, &pango_width, &pango_height);

			// Calculate how much of the background will fit onto the backing pixmap
			x_offset = (time_x - 10) * scaled_width_ratio;
			y_offset = (time_y - 2) * scaled_height_ratio;
			width = (pango_width / PANGO_SCALE) + (20 * scaled_width_ratio);
			height = (pango_height / PANGO_SCALE) + (4 * scaled_height_ratio);
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset - 1;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset - 1;
			}
			if (x_offset < 0)
			{
				x_offset = 0;
			}
			if (y_offset < 0)
			{
				y_offset = 0;
			}

			// Store the calculated width and height so we can do collision detection in the event handler
			((layer_text *) this_layer_data->object_data)->rendered_width = width / scaled_width_ratio;
			((layer_text *) this_layer_data->object_data)->rendered_height = height / scaled_height_ratio;

			// Draw the text background box (or as much as will fit)
			draw_highlight_box(tmp_pixbuf, x_offset, y_offset, width, height,
			0xFFFFCCFF,						// Fill color - light yellow
			0x000000FF,						// Border color - black
			time_alpha);

			// Turn the pixbuf into a pixmap
			tmp_colormap = gdk_colormap_get_system();
			tmp_pixmap = gdk_pixmap_new(NULL, gdk_pixbuf_get_width(GDK_PIXBUF(tmp_pixbuf)), gdk_pixbuf_get_height(GDK_PIXBUF(tmp_pixbuf)), tmp_colormap->visual->depth);
			gdk_drawable_set_colormap(GDK_DRAWABLE(tmp_pixmap), GDK_COLORMAP(tmp_colormap));
			gdk_draw_pixbuf(GDK_PIXMAP(tmp_pixmap), NULL, GDK_PIXBUF(tmp_pixbuf), 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			graphics_context = gdk_gc_new(GDK_PIXMAP(tmp_pixmap));

			// Set the color of the text
			output_screen = gdk_drawable_get_screen(GDK_DRAWABLE(tmp_pixmap));
			pango_renderer = gdk_pango_renderer_get_default(GDK_SCREEN(output_screen));
			gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(pango_renderer), GDK_DRAWABLE(tmp_pixmap));
			gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(pango_renderer), graphics_context);
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, &(((layer_text *) this_layer_data->object_data)->text_color));

			// Position the text
			pango_matrix_translate(&pango_matrix, time_x * scaled_width_ratio, time_y * scaled_height_ratio);
			pango_context_set_matrix(pango_context, &pango_matrix);

			// Render the text onto the pixmap
			pango_renderer_draw_layout(PANGO_RENDERER(pango_renderer), PANGO_LAYOUT(pango_layout), 0, 0);

			// Unset the renderer, as its shared
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, NULL);
			gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(pango_renderer), NULL);
			gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(pango_renderer), NULL);

			// Copy the pixmap back onto the backing pixbuf
			gdk_pixbuf_get_from_drawable(GDK_PIXBUF(tmp_pixbuf), GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);	

			// Free the memory allocated but no longer needed
			g_object_unref(PANGO_LAYOUT(pango_layout));
			g_object_unref(GDK_GC(graphics_context));
			if (NULL != tmp_pixmap) g_object_unref(GDK_PIXMAP(tmp_pixmap));

			break;

		case TYPE_HIGHLIGHT:

			// * Draw the highlight *

			// 0x00FF0040 : Good fill color
			// 0x00FF00CC : Good border color

			// Calculate how much of the highlight will fit onto the backing pixmap
			x_offset = time_x * scaled_width_ratio;
			y_offset = time_y * scaled_height_ratio;
			width = ((layer_highlight *) this_layer_data->object_data)->width * scaled_width_ratio;
			height = ((layer_highlight *) this_layer_data->object_data)->height * scaled_height_ratio;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the highlight box (or as much as will fit)
			draw_highlight_box(tmp_pixbuf, x_offset, y_offset, width, height,
			0x00FF0040,						// Fill color
			0x00FF00CC,						// Border color
			time_alpha);

			break;		

		default:
			display_warning("Error ED33: Unknown layer type");		
	}
}

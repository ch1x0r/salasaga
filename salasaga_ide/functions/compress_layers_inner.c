/*
 * $Id$
 *
 * Flame Project: Function to process the data for a layer, adding it to the main image being built 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void compress_layers_inner(gpointer element, gpointer user_data)
{
	// Local variables
	GdkGC				*graphics_context;                      // GDK graphics context
	GdkScreen			*output_screen;                         //
	layer				*layer_pointer;                         // Points to the data for this layer

	PangoContext            	*pango_context;                         // Pango context used for text rendering
	PangoFontDescription            *font_description;                      // Pango font description used for text rendering
	PangoLayout			*pango_layout;                          // Pango layout used for text rendering
	gint				pango_height;                           // Height of the Pango layout
	gint				pango_width;                            // Width of the Pango layout

	GtkTextIter			text_start;				// The start position of the text buffer
	GtkTextIter			text_end;				// The end position of the text buffer

	gint				pixbuf_width;                           // Width of the backing pixbuf
	gint				pixbuf_height;                          // Height of the backing pixbuf
	gint				x_offset;				//
	gint				y_offset;				//
	gint				width;					//
	gint				height;					//

	GdkColormap			*tmp_colormap;                          // Temporary colormap
	gint				tmp_int;				// Temporary integer
	GdkPixbuf			*tmp_pixbuf;                            // GDK Pixbuf
	GdkPixmap			*tmp_pixmap;                            // GDK Pixmap

	PangoMatrix                     pango_matrix = PANGO_MATRIX_INIT;       // Required for positioning the pango layout
	PangoRenderer                   *pango_renderer;                        // Pango renderer


	// Initialise various pointers
	layer_pointer = element;
	tmp_pixbuf = user_data;  // The backing pixbuf

	// Determine the type of layer we're dealing with (we ignore background layers)
	switch (layer_pointer->object_type)
	{
		case TYPE_GDK_PIXBUF:
			// * Image *

			// Determine if we're processing the background image (which we ignore) or an image layer
			tmp_int = g_ascii_strncasecmp(layer_pointer->name->str, "Background", 10);
			if (0 == tmp_int)
			{
				// We're processing a background layer, so return
				return;
			}

			// * We're processing an image layer *

			// Calculate how much of the source image will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_image *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_image *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_image *) layer_pointer->object_data)->width;
			height = ((layer_image *) layer_pointer->object_data)->height;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the image onto the backing pixbuf
			gdk_pixbuf_composite(((layer_image *) layer_pointer->object_data)->image_data,	// Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			1, 1,							// Scale factor (1 == no scale)
			GDK_INTERP_NEAREST,					// Scaling type
			255);							// Alpha
			user_data = (GdkPixbuf *) tmp_pixbuf;

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
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_mouse *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_mouse *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_mouse *) layer_pointer->object_data)->width;
			height = ((layer_mouse *) layer_pointer->object_data)->height;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the mouse pointer onto the backing pixbuf
			gdk_pixbuf_composite(mouse_ptr_pixbuf,			// Source pixbuf
			tmp_pixbuf,						// Destination pixbuf
			x_offset,						// X offset
			y_offset,						// Y offset
			width,							// Width
			height,							// Height
			x_offset,						// Source offsets
			y_offset,						// Source offsets
			1, 1,							// Scale factor (1 == no scale)
			GDK_INTERP_NEAREST,					// Scaling type
			255);							// Alpha
			user_data = (GdkPixbuf *) tmp_pixbuf;
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
				PANGO_SCALE * ((layer_text *) layer_pointer->object_data)->font_size);
			pango_font_description_set_family(font_description, "Bitstream Vera Sans");  // "Sans", "Serif", "Monospace"
			pango_font_description_set_style(font_description, PANGO_STYLE_NORMAL);
			pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
			pango_font_description_set_weight(font_description, PANGO_WEIGHT_NORMAL);
			pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);
			gtk_text_buffer_get_bounds(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end);
			pango_layout_set_markup(pango_layout,
				gtk_text_buffer_get_text(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end, FALSE),  // Point to the text in the object buffer
				-1);	  // Auto calculate string length

			// * Draw a background for the text layer *

			// Work out how big the rendered text will be
			pango_layout_get_size(pango_layout, &pango_width, &pango_height);

			// Calculate how much of the background will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_text *) layer_pointer->object_data)->x_offset_start - 10;
			y_offset = ((layer_text *) layer_pointer->object_data)->y_offset_start - 2;
			width = (pango_width / PANGO_SCALE) + 20;
			height = (pango_height / PANGO_SCALE) + 4;
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
			((layer_text *) layer_pointer->object_data)->rendered_width = width;
			((layer_text *) layer_pointer->object_data)->rendered_height = height;

			// Draw the highlight box (or as much as will fit)
			draw_highlight_box(tmp_pixbuf, x_offset, y_offset, width, height,
			0xFFFFCCFF,						// Fill color - light yellow
			0x000000FF);						// Border color - black

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
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, &(((layer_text *) layer_pointer->object_data)->text_color));

			// Position the text
			pango_matrix_translate(&pango_matrix,
				((layer_text *) layer_pointer->object_data)->x_offset_start,
				((layer_text *) layer_pointer->object_data)->y_offset_start);
			pango_context_set_matrix(pango_context, &pango_matrix);

			// Render the text onto the pixmap
			pango_renderer_draw_layout(PANGO_RENDERER(pango_renderer), PANGO_LAYOUT(pango_layout), 0, 0);

			// Unset the renderer, as its shared
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, NULL);
			gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(pango_renderer), NULL);
			gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(pango_renderer), NULL);

			// Copy the pixmap back onto the backing pixbuf
			gdk_pixbuf_get_from_drawable(GDK_PIXBUF(tmp_pixbuf), GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);	
			user_data = GDK_PIXBUF(tmp_pixbuf);

			// Free the memory allocated but no longer needed
			g_object_unref(pango_layout);
			g_object_unref(graphics_context);
			if (NULL != tmp_pixmap) g_object_unref(tmp_pixmap);

			break;

		case TYPE_HIGHLIGHT:
			// * Draw the highlight *

			// 0x00FF0040 : Good fill color
			// 0x00FF00CC : Good border color

			// Calculate how much of the highlight will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_highlight *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_highlight *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_highlight *) layer_pointer->object_data)->width;
			height = ((layer_highlight *) layer_pointer->object_data)->height;
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
			0x00FF00CC);						// Border color

			break;		

		default:
			display_warning("ED33: Unknown layer type\n");		
	}
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:16  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

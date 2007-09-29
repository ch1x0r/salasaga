/*
 * $Id$
 *
 * Flame Project: Function to convert a slide structure into an output file
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
#include <gtk/gtk.h>

#ifndef _WIN32
	// Non-windows code
	#include <gconf/gconf.h>
	#include <libgnome/libgnome.h>
#else
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"


void menu_export_svg_animation_slide(gpointer element, gpointer user_data)
{
	// Local variables
	gchar				*base64_string;			// Pointer to an Base64 string
	gchar				*encoded_string;		// Pointer to an URI encoded Base64 string
	GError				*error = NULL;			// Pointer to error return structure
	GString				*file_name = NULL;		// Used to construct the file name of the output file
	guint				finish_frame;			// The finish frame in which the object appears
	gint				layer_counter;			// Simple counter
	layer				*layer_data;			// Points to the presently processing layer
	GList				*layer_pointer;			// Points to the layer structure in the slide
	gint				num_layers;				// Number of layers in the slide
	gint				num_text_lines;				// Number of text lines in a layer
	guint				object_type;			// The type of object in each layer
	gchar				*pixbuf_buffer;			// Gets given a pointer to a compressed jpeg image
	gsize				pixbuf_size;			// Gets given the size of a compressed jpeg image
	GIOStatus			return_value;			// Return value used in most GIOChannel functions
	slide				*slide_pointer;			// Points to the present slide
	guint				start_frame;			// The first frame in which the object appears
	GString				*string_to_write = NULL;// Holds SVG data to be written out
	GtkTextIter			text_end;				// The end position of the text buffer
	gint				text_lines_counter;			// Counter used when processing text
	GtkTextIter			text_start;				// The start position of the text buffer
	gfloat				time_end;				// The second (or part of) in the animation, in which this slide disappears
	gfloat				time_start;				// The second (or part of) in the animation, in which this slide appears
	gfloat				x_scale;				// Width scale factor for the scene
	gfloat				y_scale;				// Height scale factor for the scene

	// Pango and related variables used while processing text layers
	PangoContext            	*pango_context;                         // Pango context used for text rendering
	PangoFontDescription            *font_description;                      // Pango font description used for text rendering
	gfloat				font_size;				// Size of the text font when rendered
	PangoLayout			*pango_layout;                          // Pango layout used for text rendering
	gint				pango_height;                           // Height of the Pango layout
	gint				pango_width;                            // Width of the Pango layout
	GtkTextBuffer			*pango_text_slice_buffer;
	GString				*text_slice_buffer;
	GString				*text_output_buffer;

	gboolean			tmp_bool;				// Temporary boolean value
	gfloat				tmp_gfloat;				// Temporary gfloat
	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;			// Temporary GString
	gint				tmp_int;				// Temporary integer
	GdkPixbuf			*tmp_pixbuf;			// Temporary GDK pixbuf


	// Initialise some things
	slide_pointer = element;
	time_start = export_time_counter;
	time_end = 0;
	tmp_gstring = g_string_new(NULL);
	x_scale = (gfloat) output_width / project_width;
	y_scale = (gfloat) output_height / project_height;

	// Determine how many seconds this slide lasts for
	layer_pointer = g_list_first((GList *) slide_pointer->layers);
	num_layers = g_list_length(layer_pointer);
	for (layer_counter = num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first((GList *) slide_pointer->layers);
		layer_data = g_list_nth_data(layer_pointer, layer_counter);

		// If the present layer lasts longer than the longest known thus far, adopt the present layers version
		if (layer_data->finish_frame > time_end)
			time_end = layer_data->finish_frame;
	}

	// * At this point, time_end should contain the highest "finish frame" that was in any of the layers *
	
	// Convert time_end into seconds
	time_end = (time_end / frames_per_second) + time_start;

	// * For each layer, write it to the output file *
	layer_pointer = g_list_first((GList *) slide_pointer->layers);
	num_layers = g_list_length(layer_pointer);
	for (layer_counter = num_layers - 1; layer_counter >= 0; layer_counter--)
	{
		layer_pointer = g_list_first((GList *) slide_pointer->layers);
		layer_data = g_list_nth_data(layer_pointer, layer_counter);

		// Create some useful pointers
		start_frame = layer_data->start_frame;
		finish_frame = layer_data->finish_frame;
		object_type = layer_data->object_type;

		// Determine the type of object present in the layer
		switch (object_type)
		{
			case TYPE_EMPTY:
				// fixme3: Needs writing
				break;

			case TYPE_GDK_PIXBUF:
				// * We're processing a GDK pixbuf *

				// Resize the pixbuf to the output resolution
				tmp_pixbuf = gdk_pixbuf_scale_simple(GDK_PIXBUF(((layer_image *) layer_data->object_data)->image_data), output_width, output_height, GDK_INTERP_BILINEAR);
				if (NULL == tmp_pixbuf)
				{
					// * Something went wrong when scaling down the pixbuf *

					// Display the warning message using our function
					if (NULL == slide_pointer->name)
					{
						g_string_printf(tmp_gstring, "Error ED13: An error '%s' occured when scaling a slide", error->message);
					} else
					{
						g_string_printf(tmp_gstring, "Error ED13: An error '%s' occured when scaling slide '%s'", error->message, slide_pointer->name->str);
					}
					display_warning(tmp_gstring->str);

					// Free the memory allocated in this function
					g_string_free(tmp_gstring, TRUE);
					g_error_free(error);
					return;
				}

				// Convert the compressed image into jpeg data
				tmp_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(tmp_pixbuf),
								&pixbuf_buffer,  // Will come back filled out with location of jpeg data
								&pixbuf_size,  // Will come back filled out with size of jpeg data
								"jpeg",
								&error,
								"quality",
								"100",
								NULL);
				if (FALSE == tmp_bool)
				{
					// Something went wrong when encoding the image to jpeg format
					display_warning("ED51: Something went wrong when encoding a slide to jpeg format");

					// Free the memory allocated in this function
					if (NULL != tmp_pixbuf) g_object_unref(tmp_pixbuf);
					g_string_free(tmp_gstring, TRUE);
					g_error_free(error);
					return;
				}

				// Base64 encode the image data
				base64_encode(pixbuf_buffer, pixbuf_size, &base64_string);

				// URI encode the Base64 data
				uri_encode_base64(base64_string, strlen(base64_string), &encoded_string);

				// Create the temporary string to put everything into
				string_to_write = g_string_new(NULL);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// We handle background images differently to other image types
				tmp_int = g_ascii_strncasecmp(layer_data->name->str, "Background", 10);
				if (0 == tmp_int)
				{
					// * We're processing a background layer *

					// Create a string to write to the output svg file
					g_string_append_printf(string_to_write, "<image id=\"%s-background\" width=\"%upx\" height=\"%upx\" opacity=\"0.0\" xlink:href=\"data:image/jpeg;base64,%s\">\n",
						layer_data->name->str,
						output_width,
						output_height,
						encoded_string);

					// Animate the image SVG properties so the background is only visible for as long as the slide
					g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n</image>\n",
						1 + time_start,
						time_end - time_start);
				} else
				{
					// * We're not processing a background layer *

					// Create a string to write to the output svg file
					g_string_append_printf(string_to_write, "<image id=\"%s-standard_image\" width=\"%.4fpx\" height=\"%.4fpx\" x=\"%.4fpx\" y=\"%.4fpx\" opacity=\"0.0\" xlink:href=\"data:image/jpeg;base64,%s\">\n",
						layer_data->name->str,
						x_scale * ((layer_image *) layer_data->object_data)->width,
						y_scale * ((layer_image *) layer_data->object_data)->height,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_start,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_start,
						encoded_string);

					// Animate the image SVG properties to fade it in over 1 second
					g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
						1 + time_start + (layer_data->start_frame / frames_per_second));

					// Animate the image SVG properties so it keeps visible after faded in
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						(((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2);

					// Animate the image SVG properties so they fade out over 1 second
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
						1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

					// Animate the SVG properties to move it to it's destination location
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_start,
						x_scale * ((layer_image *) layer_data->object_data)->x_offset_finish);
					g_string_append_printf(string_to_write,
						"\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_start,
						y_scale * ((layer_image *) layer_data->object_data)->y_offset_finish);

					// Remove the image from display after it's no longer needed
					g_string_append_printf(string_to_write,
						"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</image>\n",
						1 + time_start + (layer_data->finish_frame / frames_per_second));
				}

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				// Free the allocated memory
				if (NULL != tmp_pixbuf) g_object_unref(tmp_pixbuf);
				g_free(encoded_string);
				g_free(base64_string);
				break;

			case TYPE_HIGHLIGHT:
				// We're processing a highlight layer
				string_to_write = g_string_new(NULL);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// Add the SVG tag, but ensure the highlight box starts out invisible
				g_string_append_printf(string_to_write,
					"<rect id=\"%s-highlight\" class=\"highlight\" width=\"%.4fpx\" height=\"%.4fpx\" opacity=\"0.0\" x=\"%.4fpx\" y=\"%.4fpx\" stroke-width=\"%.4fpx\">\n",
					layer_data->name->str,
					x_scale * ((layer_highlight *) layer_data->object_data)->width,
					y_scale * ((layer_highlight *) layer_data->object_data)->height,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * 2);

				// Animate the highlight box SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the highlight box SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the highlight box SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_finish);
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_finish);

				// Remove the rectangle from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</rect>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				break;

			case TYPE_MOUSE_CURSOR:
				// We're processing a mouse pointer layer
				string_to_write = g_string_new(NULL);

				// Add the SVG tag, but ensure the mouse pointer starts out invisible
				g_string_append_printf(string_to_write,
					"<path d=\"M %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f z\""
					" fill=\"#ffffff\" opacity=\"0.0\" fill-rule=\"evenodd\" stroke=\"#000000\""
					" stroke-width=\"%.4f\" stroke-linecap=\"square\" stroke-miterlimit=\"4\""
					" stroke-dasharray=\"none\" stroke-dashoffset=\"0\">\n",
					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 20.875),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 19.1705),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 0.25),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 0.5623),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 0.25),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 26.2681),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 6.2604),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 20.2253),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 12.7899),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 32.8861),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 18.3869),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 32.8861),

					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_start + 11.2354),
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_start + 19.1705),

					y_scale * 1.1776);  // Stroke width

				// Animate the mouse pointer SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the mouse pointer SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the mouse pointer SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t<animateMotion attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"0px,0px\" to=\"%.4fpx,%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * (((layer_highlight *) layer_data->object_data)->x_offset_finish - ((layer_highlight *) layer_data->object_data)->x_offset_start),  // X distance to travel
					y_scale * (((layer_highlight *) layer_data->object_data)->y_offset_finish - ((layer_highlight *) layer_data->object_data)->y_offset_start));  // Y distance to travel

				// Remove the mouse pointer from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</path>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				break;

			case TYPE_TEXT:
				// We're processing a text layer
				string_to_write = g_string_new(NULL);

				// Start the group enclosing the text layer elements
				g_string_append_printf(string_to_write, "<g id=\"%s\">\n", layer_data->name->str);

				// If the layer has an external link, then create <a> tag around it
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "<a xlink:href=\"%s\" xlink:show=\"new\">\n", layer_data->external_link->str);
				}

				// Create the SVG tag for the background box the text goes onto
				g_string_append_printf(string_to_write,
					"\t<rect id=\"%s-bg\" class=\"text\" width=\"%.4fpx\" height=\"%.4fpx\" opacity=\"0.0\" x=\"%.4fpx\" y=\"%.4fpx\" rx=\"%.4fpx\" ry=\"%.4fpx\" stroke-width=\"%.4fpx\">\n",
					layer_data->name->str,
					(x_scale * ((layer_text *) layer_data->object_data)->rendered_width) + 8,
					y_scale * (((layer_text *) layer_data->object_data)->rendered_height + 10),
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,
					y_scale * ((layer_text *) layer_data->object_data)->y_offset_start,
					y_scale * 10,  // Relative X
					y_scale * 10,  // Relative Y
					y_scale * 2);  // Stroke width

				// Animate the background box SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />\n",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the background box SVG properties so it keeps visible after faded in
				tmp_gfloat = (((gfloat) layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2;
				if (0 > tmp_gfloat)
					tmp_gfloat = 0;
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the background box SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Animate the SVG properties to move it to it's destination location
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_start,
					x_scale * ((layer_highlight *) layer_data->object_data)->x_offset_finish);
				g_string_append_printf(string_to_write,
					"\t\t<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />\n",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_start,
					y_scale * ((layer_highlight *) layer_data->object_data)->y_offset_finish);

				// Remove the text background from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"\t<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n\t</rect>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// Create the text tag
				num_text_lines = gtk_text_buffer_get_line_count(((layer_text *) layer_data->object_data)->text_buffer);
				font_size = (y_scale * ((layer_text *) layer_data->object_data)->rendered_height - 2) / num_text_lines;
				g_string_append_printf(string_to_write, "\t<text id=\"%s-text\" font-family=\"Bitstream Vera Sans svg\" class=\"text\" x=\"%.4fpx\" y=\"%.4fpx\" opacity=\"0.0\" font-size=\"%.4fpx\" lengthAdjust=\"spacingAndGlyphs\" dx=\"%.4fpx\" dy=\"%.4fpx\" ",
					layer_data->name->str,
					x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,  // X offset
					y_scale * ((layer_text *) layer_data->object_data)->y_offset_start,  // Y offset
					font_size,  // Font size
					x_scale * 10,  // Horizontal space between text background border and text start
					y_scale * ((((layer_text *) layer_data->object_data)->rendered_height + 54) / 2));  // Vertical space between text background border and text start

				// Does the text we're displaying have multiple lines?
				if (1 < num_text_lines)
				{
					// * Yes, so calculate the true total length of the text, and wrap the lines of lines in tspan's *

					// Initialise some buffer strings we need for the calculation
					text_slice_buffer = g_string_new(NULL);
					text_output_buffer = g_string_new(NULL);
					pango_text_slice_buffer = gtk_text_buffer_new(NULL);

					// Initialise a bunch of pango objects we'll need
					pango_context = gdk_pango_context_get();
					pango_layout = pango_layout_new(pango_context);
					font_description = pango_context_get_font_description(pango_context);
					pango_font_description_set_size(font_description, PANGO_SCALE * ((layer_text *) layer_data->object_data)->font_size);
					pango_font_description_set_family(font_description, "Bitstream Vera Sans");  // "Sans", "Serif", "Monospace"
					pango_font_description_set_style(font_description, PANGO_STYLE_NORMAL);
					pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
					pango_font_description_set_weight(font_description, PANGO_WEIGHT_NORMAL);
					pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);

					// Add each line of text to the output, wrapped with a tspan
					for (text_lines_counter = 0; text_lines_counter < num_text_lines; text_lines_counter++)
					{
						gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(((layer_text *) layer_data->object_data)->text_buffer), &text_start, text_lines_counter);
						text_end = text_start;
						gtk_text_iter_forward_to_line_end(&text_end);
						g_string_append_printf(text_output_buffer, "<tspan x=\"%.4fpx\" dx=\"%.4fpx\" dy=\"1em\">%s\n",
							x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,  // X offset
							x_scale * 10,  // Horizontal space between text background border and text start
							gtk_text_iter_get_visible_text(&text_start, &text_end));  // Text to be rendered

						// Animate the SVG properties to move it to it's destination location
						g_string_append_printf(text_output_buffer,
							"<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
							1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
							time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
							x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,
							x_scale * ((layer_text *) layer_data->object_data)->x_offset_finish);
						g_string_append_printf(text_output_buffer,
							"<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
							1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
							time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
							(gfloat) (y_scale * ((layer_text *) layer_data->object_data)->y_offset_start) + (font_size * text_lines_counter),
							(gfloat) (y_scale * ((layer_text *) layer_data->object_data)->y_offset_finish) + (font_size * text_lines_counter));  // Y finish

						// Close the tspan
						g_string_append_printf(text_output_buffer, "</tspan>\n");
						
						// Add the text slice to the text slice buffer we'll use for calculating the total length
						g_string_append_printf(text_slice_buffer, "%s", gtk_text_iter_get_visible_text(&text_start, &text_end));
					}

					// Work out the width of the rendered text
					gtk_text_buffer_set_text(GTK_TEXT_BUFFER(pango_text_slice_buffer), text_slice_buffer->str, -1);
					gtk_text_buffer_get_bounds(pango_text_slice_buffer, &text_start, &text_end);
					pango_layout_set_markup(pango_layout, gtk_text_buffer_get_text(pango_text_slice_buffer, &text_start, &text_end, FALSE), -1);  // Auto calculate string length
					pango_layout_get_size(pango_layout, &pango_width, &pango_height);

					if (debug_level)
					{
//						printf("Complete text output: '%s'\n", text_output_buffer->str);
						printf("Complete width: %d\n", pango_width);
						printf("Tweaked PANGO_SCALEd width: %.4fpx\n", (gfloat) (pango_width / PANGO_SCALE) + 20);
						printf("Tweaked xscaled width: %.4fpx\n", (gfloat) ((pango_width / PANGO_SCALE) + 20) * x_scale);
					}

					// Add the text length and text string to the output
					g_string_append_printf(string_to_write, "textLength=\"%.4fpx\">%s", 
						(gfloat) ((pango_width / PANGO_SCALE) * x_scale),
						text_output_buffer->str);

					// Free the buffers created for this calculation
					g_string_free(text_slice_buffer, TRUE);
					g_string_free(text_output_buffer, TRUE);
					if (NULL != pango_text_slice_buffer) g_object_unref(pango_text_slice_buffer);

				} else
				{
					// Specify the total length of the text
					g_string_append_printf(string_to_write, "textLength=\"%.4fpx\">",
						x_scale * (((layer_text *) layer_data->object_data)->rendered_width - 40));  // How wide to make the entire string

					// Just one line of text, so no need to use tspan's
					gtk_text_buffer_get_bounds(((layer_text *) layer_data->object_data)->text_buffer, &text_start, &text_end);
					g_string_append_printf(string_to_write, "%s",
					gtk_text_buffer_get_text(((layer_text *) layer_data->object_data)->text_buffer, &text_start, &text_end, FALSE));  // Text to be rendered

					// Animate the SVG properties to move it to it's destination location
					g_string_append_printf(string_to_write,
						"<animate attributeName=\"x\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						x_scale * ((layer_text *) layer_data->object_data)->x_offset_start,
						x_scale * ((layer_text *) layer_data->object_data)->x_offset_finish);
					g_string_append_printf(string_to_write,
						"<animate attributeName=\"y\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"%0.4fs\" fill=\"freeze\" from=\"%.4fpx\" to=\"%.4fpx\" />",
						1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
						time_start + ((layer_data->finish_frame - layer_data->start_frame) / frames_per_second) - 2,
						(y_scale * (((layer_text *) layer_data->object_data)->y_offset_start)),  // Y start
						(y_scale * ((layer_text *) layer_data->object_data)->y_offset_finish));  // Y finish
				}

				// Animate the text SVG properties to fade it in over 1 second
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"0.0\" to=\"1.0\" />",
					1 + time_start + (layer_data->start_frame / frames_per_second));

				// Animate the text SVG properties so it keeps visible after faded in
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" values=\"1.0\" keyTimes=\"0\" begin=\"%.4fs\" dur=\"%.4fs\" />",
					1 + time_start + 1 + (layer_data->start_frame / frames_per_second),
					tmp_gfloat);

				// Animate the text SVG properties so they fade out over 1 second
				g_string_append_printf(string_to_write,
					"<animate attributeName=\"opacity\" attributeType=\"XML\" begin=\"%.4fs\" dur=\"1s\" fill=\"freeze\" from=\"1.0\" to=\"0.0\" />",
					1 + time_start + (layer_data->finish_frame / frames_per_second) - 1);

				// Remove the text itself from display after it's no longer needed
				g_string_append_printf(string_to_write,
					"<set attributeName=\"display\" attributeType=\"XML\" to=\"none\" begin=\"%.4fs\" />\n</text>\n",
					1 + time_start + (layer_data->finish_frame / frames_per_second));

				// If the layer has an external link, then close the <a> tag
				if (0 != layer_data->external_link->len)
				{
					g_string_append_printf(string_to_write, "</a>\n");
				}

				// Close the group
				g_string_append_printf(string_to_write, "</g>\n");

				break;

			default:
				g_string_printf(tmp_gstring, "ED29: Unknown object type found in layer '%s'\n", layer_data->name->str);
				display_warning(tmp_gstring->str);
		}

		// If there is a string to write to the output file, do so
		if (NULL != string_to_write)
		{
			return_value = g_io_channel_write_chars(output_file, string_to_write->str, string_to_write->len, &tmp_gsize, &error);
			if (G_IO_STATUS_ERROR == return_value)
			{
				// * An error occured when writing the SVG closing tags to the output file, so alert the user, and return to the calling routine indicating failure *

				// Display the warning message using our function
				g_string_printf(tmp_gstring, "Error ED12: An error '%s' occured when writing SVG data to the output file '%s'", error->message, file_name->str);
				display_warning(tmp_gstring->str);

				// Free the memory allocated in this function
				g_string_free(file_name, TRUE);
				g_string_free(tmp_gstring, TRUE);
				g_error_free(error);
				return;
			}

			// Reset the string to be written to the output file
			g_string_free(string_to_write, TRUE);
			string_to_write = NULL;
		}
	}

	// Update the time counter, so the next frame starts after the end of this one
	export_time_counter = time_end;
	return;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2007/09/29 04:22:11  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 * Revision 1.2  2007/09/28 12:05:05  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 * Revision 1.1  2007/09/27 10:39:56  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

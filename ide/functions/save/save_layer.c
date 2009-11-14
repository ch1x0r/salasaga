/*
 * $Id$
 *
 * Salasaga: Function that takes a layer structure pointer, and adds it's data to an in-memory XML document
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// XML includes
#include <libxml/parser.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../dialog/display_warning.h"


void save_layer(gpointer element, gpointer user_data)
{
	// Local variables

	gchar				*base64_string;			// Pointer to an Base64 string
	guint				character_counter;		// Counts how many characters have been written
	xmlNodePtr			character_node;			// Pointer to a character node
	gchar				*conversion_buffer;		// Used for converting from UTF-32 to UTF-8
	gint				end_offset;				// Used when calculating text buffer character offsets
	GError				*error = NULL;			// Pointer to error return structure
	GString				*layer_name;			// Name of the layer
	xmlNodePtr			layer_node;				// Pointer to the new layer node
	layer				*layer_pointer;			// Points to the presently processing layer
	guint				layer_type;				// The type of layer
	guint				loop_counter;
	GtkTextIter 		loop_iter;
	GString				*message;				// Used to construct message strings
	guint				num_tags;
	gchar				*pixbuf_buffer;			// Gets given a pointer to a compressed jpeg image
	gsize				pixbuf_size;			// Gets given the size of a compressed jpeg image
	xmlNodePtr			slide_node;				// Pointer to the slide node
	GtkTextBuffer		*source_buffer;			// Simplified pointer, pointing to text layer text buffer contents
	GtkTextIter			source_buffer_end;		// Used when calculating text buffer character offsets
	GtkTextIter			source_buffer_start;	// Used when calculating text buffer character offsets
	gint				start_offset;			// Used when calculating text buffer character offsets
	GSList				*tag_list;
	GtkTextTag			*tag_ptr;
	gunichar			temp_char;				// Used for converting from UTF-32 to UTF-8
	xmlNodePtr			text_buffer_node;		// Pointer to a text buffer node
	GtkTextIter			text_end;				// The end position of the text buffer
	GtkTextIter			text_start;				// The start position of the text buffer
	gboolean			tmp_bool;				// Temporary boolean value
	GString				*tmp_gstring;			// Temporary GString
	GString				*tmp_gstring2;			// Temporary GString
	GdkRectangle		tmp_rect = {0, 0, status_bar->allocation.width, status_bar->allocation.height};  // Temporary rectangle covering the area of the status bar


	// Initialise various things
	layer_pointer = element;
	slide_node = user_data;
	message = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);
	tmp_gstring2 = g_string_new(NULL);

	// Create some useful pointers
	layer_name	= layer_pointer->name;
	layer_type = layer_pointer->object_type;

	// Create the layer container
	layer_node = xmlNewChild(slide_node, NULL, (const xmlChar *) "layer", NULL);
	if (NULL == layer_node)
	{
		g_string_printf(message, "%s ED24: %s", _("Error"), _("Error creating the layer elements."));
		display_warning(message->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_string_free(tmp_gstring2, TRUE);
		g_string_free(message, TRUE);
		return;
	}

	// Add the layer data to the layer container
	xmlNewChild(layer_node, NULL, (const xmlChar *) "name", (const xmlChar *) layer_name->str);
	g_string_printf(tmp_gstring, "%0.4f", layer_pointer->start_time);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "start_time", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%0.4f", layer_pointer->duration);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "duration", (const xmlChar *) tmp_gstring->str);
	g_string_printf(tmp_gstring, "%d", layer_pointer->visible);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "visible", (const xmlChar *) tmp_gstring->str);
	if (0 != layer_pointer->external_link->len)
	{
		xmlNewChild(layer_node, NULL, (const xmlChar *) "external_link", (const xmlChar *) layer_pointer->external_link->str);
	}
	if (0 != layer_pointer->external_link_window->len)
	{
		xmlNewChild(layer_node, NULL, (const xmlChar *) "external_link_window", (const xmlChar *) layer_pointer->external_link_window->str);
	}
	switch (layer_pointer->transition_in_type)
	{
		case TRANS_LAYER_FADE:
			xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_in_type", (const xmlChar *) "fade");
			break;

		default:
			// Default to no transition in
			xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_in_type", (const xmlChar *) "none");
	}
	g_string_printf(tmp_gstring, "%0.4f", layer_pointer->transition_in_duration);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_in_duration", (const xmlChar *) tmp_gstring->str);
	switch (layer_pointer->transition_out_type)
	{
		case TRANS_LAYER_FADE:
			xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_out_type", (const xmlChar *) "fade");
			break;

		default:
			// Default to no transition in
			xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_out_type", (const xmlChar *) "none");
	}
	g_string_printf(tmp_gstring, "%0.4f", layer_pointer->transition_out_duration);
	xmlNewChild(layer_node, NULL, (const xmlChar *) "transition_out_duration", (const xmlChar *) tmp_gstring->str);
	switch (layer_type)
	{
		case TYPE_GDK_PIXBUF:

			// * We save the image data in the project file, encoded in base64 text *
			// * format.  This way a project file is self-contained and portable    *

			// Convert the compressed image into png data
			tmp_bool = gdk_pixbuf_save_to_buffer(GDK_PIXBUF(((layer_image *) layer_pointer->object_data)->image_data),
						&pixbuf_buffer,  // Will come back filled out with location of png data
						&pixbuf_size,  // Will come back filled out with size of png data
						"png",
						&error,
						NULL);
			if (FALSE == tmp_bool)
			{
				// Something went wrong when encoding the image to jpeg format
				g_string_printf(message, "%s ED62: %s", _("Error"), _("Something went wrong when encoding a slide to png format."));
				display_warning(message->str);
				g_string_free(message, TRUE);

				// Free the memory allocated in this function
				g_string_free(tmp_gstring, TRUE);
				g_string_free(tmp_gstring2, TRUE);
				g_error_free(error);
				return;
			}

			// Store the count of image data bytes in the file for read back verification
			g_string_printf(tmp_gstring2, "%lu", pixbuf_size);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data_length", (const xmlChar *) tmp_gstring2->str);

			// Base64 encode the image data
			base64_string = g_base64_encode((const guchar *) pixbuf_buffer, pixbuf_size);
			g_free(pixbuf_buffer);

			// Create a string to write to the output file
			g_string_printf(tmp_gstring, "%s", base64_string);

			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "image");
			xmlNewChild(layer_node, NULL, (const xmlChar *) "data", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->background);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "background", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_image *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_EMPTY:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "empty");
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_empty *) layer_pointer->object_data)->bg_color.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->background);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "background", (const xmlChar *) tmp_gstring->str);
			break;

		case TYPE_MOUSE_CURSOR:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "mouse");
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_mouse *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			switch (((layer_mouse *) layer_pointer->object_data)->click)
			{
				case MOUSE_LEFT_ONE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "left_one");
					break;

				case MOUSE_LEFT_DOUBLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "left_double");
					break;

				case MOUSE_LEFT_TRIPLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "left_triple");
					break;

				case MOUSE_RIGHT_ONE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "right_one");
					break;

				case MOUSE_RIGHT_DOUBLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "right_double");
					break;

				case MOUSE_RIGHT_TRIPLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "right_triple");
					break;

				case MOUSE_MIDDLE_ONE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "middle_one");
					break;

				case MOUSE_MIDDLE_DOUBLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "middle_double");
					break;

				case MOUSE_MIDDLE_TRIPLE:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "middle_triple");
					break;

				default:
					xmlNewChild(layer_node, NULL, (const xmlChar *) "click", (const xmlChar *) "none");
			}
			break;

		case TYPE_TEXT:

			// Simplify pointers
			source_buffer = ((layer_text *) layer_pointer->object_data)->text_buffer;
			gtk_text_buffer_get_bounds(source_buffer, &text_start, &text_end);

			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "text");
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			if (TRUE == ((layer_text *) layer_pointer->object_data)->show_bg)
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "show_bg", (const xmlChar *) "true");
			} else
			{
				xmlNewChild(layer_node, NULL, (const xmlChar *) "show_bg", (const xmlChar *) "false");
			}
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_border_colour.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_border_colour_red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_border_colour.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_border_colour_green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_border_colour.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_border_colour_blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_fill_colour.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_fill_colour_red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_fill_colour.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_fill_colour_green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_text *) layer_pointer->object_data)->bg_fill_colour.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_fill_colour_blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%0.4f", ((layer_text *) layer_pointer->object_data)->bg_border_width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "bg_border_width", (const xmlChar *) tmp_gstring->str);

			// * Each text layer buffer becomes a branch with nodes *

			// Create a new text buffer container node
			text_buffer_node = xmlNewChild(layer_node, NULL, (const xmlChar *) "text_buffer", NULL);
			if (NULL == text_buffer_node)
			{
				g_string_printf(tmp_gstring, "%s ED439: %s", _("Error"), _("Error saving a text data node in the project file."));
				display_warning(tmp_gstring->str);
				g_string_free(tmp_gstring, TRUE);
				return;
			}

			// Get the bounds of the source gtk text buffer
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(source_buffer), &source_buffer_start, &source_buffer_end);
			gtk_text_iter_order(&source_buffer_start, &source_buffer_end);

			// Write the number of characters as a property
			start_offset = gtk_text_iter_get_offset(&source_buffer_start);
			end_offset = gtk_text_iter_get_offset(&source_buffer_end);
			g_string_printf(tmp_gstring, "%u", end_offset);
			xmlNewProp(text_buffer_node, (const xmlChar *) "num_characters", (const xmlChar *) tmp_gstring->str);

			// Scan through the source text buffer one character at a time, getting the character and the tags that apply to it
			for (character_counter = 0; character_counter < end_offset; character_counter++)
			{
				// Create a new character container node
				g_string_printf(tmp_gstring, "character_%u", character_counter);
				character_node = xmlNewChild(text_buffer_node, NULL, (const xmlChar *) tmp_gstring->str, NULL);
				if (NULL == character_node)
				{
					g_string_printf(tmp_gstring, "%s ED440: %s", _("Error"), _("Error saving character data to the project file."));
					display_warning(tmp_gstring->str);
					g_string_free(tmp_gstring, TRUE);
					return;
				}

				// Save each character of the text buffer as a property
				gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(source_buffer), &loop_iter, character_counter);
				temp_char = gtk_text_iter_get_char(&loop_iter);
				conversion_buffer = g_ucs4_to_utf8(&temp_char, 1, NULL, NULL, NULL);
				if (NULL == conversion_buffer)
				{
					g_string_printf(tmp_gstring, "%s ED443: %s", _("Error"), _("Could not convert unicode character from ucs4 to utf8."));
					display_warning(tmp_gstring->str);
					continue;
				}

				// Validate the retrieved character
				if (TRUE != g_unichar_validate(temp_char))
				{
					// Something other than a unicode character was retrieved
					g_string_printf(tmp_gstring, "%s ED444: %s", _("Error"), _("Invalid unicode character found in text."));
					display_warning(tmp_gstring->str);
					continue;
				}
				xmlNewProp(character_node, (const xmlChar *) "character", (const xmlChar *) conversion_buffer);
				g_free(conversion_buffer);

				// Save the tags names to nodes
				tag_list = gtk_text_iter_get_tags(&loop_iter);
				num_tags = g_slist_length(tag_list);
				for (loop_counter = 0; loop_counter < num_tags; loop_counter++)
				{
					// Turn each tag into a named representation
					tag_ptr = g_slist_nth_data(tag_list, loop_counter);
					xmlNewChild(character_node, NULL, (const xmlChar *) "tag", (const xmlChar *) tag_ptr->name);
				}
				g_slist_free(tag_list);
			}

			break;

		case TYPE_HIGHLIGHT:
			// Add the layer data to the output project file
			xmlNewChild(layer_node, NULL, (const xmlChar *) "type", (const xmlChar *) "highlight");
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_start);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_start", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->x_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "x_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", layer_pointer->y_offset_finish);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "y_offset_finish", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->height);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "height", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->border_colour.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "border_colour_red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->border_colour.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "border_colour_green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->border_colour.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "border_colour_blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%0.4f", ((layer_highlight *) layer_pointer->object_data)->border_width);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "border_width", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->fill_colour.red);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "fill_colour_red", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->fill_colour.green);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "fill_colour_green", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%u", ((layer_highlight *) layer_pointer->object_data)->fill_colour.blue);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "fill_colour_blue", (const xmlChar *) tmp_gstring->str);
			g_string_printf(tmp_gstring, "%0.4f", ((layer_highlight *) layer_pointer->object_data)->opacity);
			xmlNewChild(layer_node, NULL, (const xmlChar *) "opacity", (const xmlChar *) tmp_gstring->str);
			break;
	}

	// Free the memory used in this function
	g_string_free(message, TRUE);
	g_string_free(tmp_gstring, TRUE);
	g_string_free(tmp_gstring2, TRUE);

	// Update the status bar to show progress
	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(status_bar));
	gtk_widget_draw(status_bar, &tmp_rect);
	gdk_flush();

	return;
}

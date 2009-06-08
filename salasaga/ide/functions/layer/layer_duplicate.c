/*
 * $Id$
 *
 * Salasaga: Creates a new layer, copied from the specified one
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../cairo/create_cairo_pixbuf_pattern.h"
#include "../dialog/display_warning.h"
#include "../gtk_text_buffer_duplicate.h"


layer *layer_duplicate(layer *source_layer)
{
	// Local variables
	layer_highlight		*dest_highlight_data;		// Pointer to the destination highlight specific data
	layer_text			*dest_text_data;			// Pointer to the destination text specific data
	GString				*message;					// Used to construct message strings
	layer				*new_layer;					// Pointer to the newly created layer
	layer_highlight		*source_highlight_data;		// Pointer to the source highlight specific data
	layer_text			*source_text_data;			// Pointer to the source text specific data


	// Initialisation
	message = g_string_new(NULL);

	// Allocate memory for a new layer
	new_layer = g_try_new0(layer, 1);
	if (NULL == new_layer)
	{
		// We couldn't allocate memory for a new layer
		g_string_printf(message, "%s ED287: %s", _("Error"), _("Unable to allocate memory for the copy buffer.  Copy failed."));
		display_warning(message->str);
		g_string_free(message, TRUE);
		return NULL;
	}

	// Copy the selected layer into the new layer
	new_layer->object_type = source_layer->object_type;
	new_layer->start_time = source_layer->start_time;
	new_layer->transition_in_type = source_layer->transition_in_type;
	new_layer->transition_in_duration = source_layer->transition_in_duration;
	new_layer->duration = source_layer->duration;
	new_layer->transition_out_type = source_layer->transition_out_type;
	new_layer->transition_out_duration = source_layer->transition_out_duration;
	new_layer->x_offset_start = source_layer->x_offset_start;
	new_layer->y_offset_start = source_layer->y_offset_start;
	new_layer->x_offset_finish = source_layer->x_offset_finish;
	new_layer->y_offset_finish = source_layer->y_offset_finish;
	new_layer->visible = source_layer->visible;
	new_layer->background = FALSE;  // Not a background layer when copied
	new_layer->external_link = g_string_new(source_layer->external_link->str);
	new_layer->external_link_window = g_string_new(source_layer->external_link_window->str);
	new_layer->dictionary_shape = NULL;
	new_layer->display_list_item = NULL;
	if (NULL == source_layer->name)
	{
		new_layer->name = NULL;
	} else
	{
		// If the source layer is a background layer, it gets special treatment
		if (0 == g_ascii_strncasecmp(source_layer->name->str, _("Background"), 10))
		{
			if (TYPE_GDK_PIXBUF == source_layer->object_type)
			{
				new_layer->name = g_string_new(_("Image"));
			} else
			{
				new_layer->name = g_string_new(_("Empty"));
			}
		} else
		{
			// It's not a background layer, so we just copy the name
			new_layer->name = g_string_new(source_layer->name->str);
		}
	}

	// Layer type specific data
	switch (source_layer->object_type)
	{
		case TYPE_EMPTY:
			new_layer->object_data = (GObject *) g_try_new0(layer_empty, 1);
			if (NULL == new_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				g_string_printf(message, "%s ED289: %s", _("Error"), _("Unable to allocate memory in the copy buffer for layer specific data.  Copy failed."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_free(new_layer);
				return NULL;
			}
			((layer_empty *) new_layer->object_data)->bg_color.pixel = ((layer_empty *) source_layer->object_data)->bg_color.pixel;
			((layer_empty *) new_layer->object_data)->bg_color.red = ((layer_empty *) source_layer->object_data)->bg_color.red;
			((layer_empty *) new_layer->object_data)->bg_color.green = ((layer_empty *) source_layer->object_data)->bg_color.green;
			((layer_empty *) new_layer->object_data)->bg_color.blue = ((layer_empty *) source_layer->object_data)->bg_color.blue;
			break;

		case TYPE_GDK_PIXBUF:
			new_layer->object_data = (GObject *) g_try_new0(layer_image, 1);
			if (NULL == new_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				g_string_printf(message, "%s ED290: %s", _("Error"), _("Unable to allocate memory in the copy buffer for layer specific data.  Copy failed."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_free(new_layer);
				return NULL;
			}
			((layer_image *) new_layer->object_data)->width = ((layer_image *) source_layer->object_data)->width;
			((layer_image *) new_layer->object_data)->height = ((layer_image *) source_layer->object_data)->height;
			((layer_image *) new_layer->object_data)->image_data = gdk_pixbuf_copy(((layer_image *) source_layer->object_data)->image_data);
			((layer_image *) new_layer->object_data)->modified = ((layer_image *) source_layer->object_data)->modified;
			((layer_image *) new_layer->object_data)->cairo_pattern = create_cairo_pixbuf_pattern(((layer_image *) new_layer->object_data)->image_data);
			if (NULL == ((layer_image *) new_layer->object_data)->cairo_pattern)
			{
				// Something went wrong when creating the image pattern
				g_string_printf(message, "%s ED375: %s", _("Error"), _("Couldn't create an image pattern."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return NULL;
			}
			break;

		case TYPE_HIGHLIGHT:
			new_layer->object_data = (GObject *) g_try_new0(layer_highlight, 1);
			if (NULL == new_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				g_string_printf(message, "%s ED291: %s", _("Error"), _("Unable to allocate memory in the copy buffer for layer specific data.  Copy failed."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_free(new_layer);
				return NULL;
			}
			dest_highlight_data = (layer_highlight *) new_layer->object_data;
			source_highlight_data = (layer_highlight *) source_layer->object_data;
			dest_highlight_data->width = ((layer_highlight *) source_layer->object_data)->width;
			dest_highlight_data->height = ((layer_highlight *) source_layer->object_data)->height;
			dest_highlight_data->border_colour.red = source_highlight_data->border_colour.red;
			dest_highlight_data->border_colour.green = source_highlight_data->border_colour.green;
			dest_highlight_data->border_colour.blue = source_highlight_data->border_colour.blue;
			dest_highlight_data->border_width = source_highlight_data->border_width;
			dest_highlight_data->fill_colour.red = source_highlight_data->fill_colour.red;
			dest_highlight_data->fill_colour.green = source_highlight_data->fill_colour.green;
			dest_highlight_data->fill_colour.blue = source_highlight_data->fill_colour.blue;
			dest_highlight_data->opacity = source_highlight_data->opacity;
			break;

		case TYPE_MOUSE_CURSOR:
			new_layer->object_data = (GObject *) g_try_new0(layer_mouse, 1);
			if (NULL == new_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				g_string_printf(message, "%s ED292: %s", _("Error"), _("Unable to allocate memory in the copy buffer for layer specific data.  Copy failed."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_free(new_layer);
				return NULL;
			}
			((layer_mouse *) new_layer->object_data)->width = ((layer_mouse *) source_layer->object_data)->width;
			((layer_mouse *) new_layer->object_data)->height = ((layer_mouse *) source_layer->object_data)->height;
			((layer_mouse *) new_layer->object_data)->click = ((layer_mouse *) source_layer->object_data)->click;
			break;

		case TYPE_TEXT:
			new_layer->object_data = (GObject *) g_try_new0(layer_text, 1);
			if (NULL == new_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				g_string_printf(message, "%s ED293: %s", _("Error"), _("Unable to allocate memory in the copy buffer for layer specific data.  Copy failed."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				g_free(new_layer);
				return NULL;
			}
			dest_text_data = (layer_text *) new_layer->object_data;
			source_text_data = (layer_text *) source_layer->object_data;
			dest_text_data->rendered_width = source_text_data->rendered_width;
			dest_text_data->rendered_height = source_text_data->rendered_height;
			dest_text_data->text_color.pixel = source_text_data->text_color.pixel;
			dest_text_data->text_color.red = source_text_data->text_color.red;
			dest_text_data->text_color.green = source_text_data->text_color.green;
			dest_text_data->text_color.blue = source_text_data->text_color.blue;
			dest_text_data->font_size = source_text_data->font_size;
			dest_text_data->show_bg = source_text_data->show_bg;
			dest_text_data->bg_border_width = source_text_data->bg_border_width;
			dest_text_data->bg_border_colour.red = source_text_data->bg_border_colour.red;
			dest_text_data->bg_border_colour.green = source_text_data->bg_border_colour.green;
			dest_text_data->bg_border_colour.blue = source_text_data->bg_border_colour.blue;
			dest_text_data->bg_fill_colour.red = source_text_data->bg_fill_colour.red;
			dest_text_data->bg_fill_colour.green = source_text_data->bg_fill_colour.green;
			dest_text_data->bg_fill_colour.blue = source_text_data->bg_fill_colour.blue;
			dest_text_data->font_face = source_text_data->font_face;

			// Copy the existing text buffer
			dest_text_data->text_buffer = gtk_text_buffer_duplicate(source_text_data->text_buffer);
			break;

		default:

			// No code written to handle this type of layer yet
			g_string_printf(message, "%s ED288: %s", _("Error"), _("Unknown layer type when duplicating layer."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			g_free(new_layer);
			return NULL;
	}

	// Duplication succeeded, so return the new layer
	return new_layer;
}

/*
 * $Id$
 *
 * Salasaga: Copies the presently selected layer into the copy buffer
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


// Standard includes
#include <stdlib.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_warning.h"
#include "layer_free.h"


void layer_copy(void)
{
	// Local variables
	slide				*current_slide_data;		// Alias to make things easier
	GtkWidget			*list_widget;				// Alias to the timeline widget to make things easier
	guint				selected_layer;				// Holds the number of the layer that is selected
	GtkTextIter			text_start;					// The start position of the text buffer
	GtkTextIter			text_end;					// The end position of the text buffer
	layer				*this_layer;				// Pointer to the presently selected layer
	GtkTreePath			*tmp_path;					// Temporary path


	// Initialise some things
	current_slide_data = current_slide->data;
	list_widget = current_slide_data->timeline_widget;

	// Determine which layer the user has selected in the timeline
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, NULL);
	selected_layer = atoi(gtk_tree_path_to_string(tmp_path));
	current_slide_data->layers = g_list_first(current_slide_data->layers);
	this_layer = g_list_nth_data(current_slide_data->layers, selected_layer);

	// If there's presently a layer in the copy buffer we destroy it
	if (NULL != copy_layer)
	{
		layer_free(this_layer);
		copy_layer = NULL;
	}

	// Create a new layer
	copy_layer = g_try_new0(layer, 1);
	if (NULL == copy_layer)
	{
		// We couldn't allocate memory for a new layer
		display_warning("Error ED287: Unable to allocate memory for the copy buffer.  Copy failed.");
		return;
	}

	// Copy the selected layer into the new layer
	copy_layer->object_type = this_layer->object_type;
	copy_layer->start_frame = this_layer->start_frame;
	copy_layer->finish_frame = this_layer->finish_frame;
	copy_layer->x_offset_start = this_layer->x_offset_start;
	copy_layer->y_offset_start = this_layer->y_offset_start;
	copy_layer->x_offset_finish = this_layer->x_offset_finish;
	copy_layer->visible = this_layer->visible;
	copy_layer->background = FALSE;  // Not a background layer when copied (could be changed in future though, unsure)
	copy_layer->row_iter = NULL;  // Should get set by the paste function
	copy_layer->external_link = g_string_new(this_layer->external_link->str);
	copy_layer->external_link_window = g_string_new(this_layer->external_link_window->str);
	copy_layer->dictionary_shape = NULL;
	copy_layer->display_list_item = NULL;
	if (NULL == this_layer->name)
	{
		copy_layer->name = NULL;
	} else
	{
		copy_layer->name = g_string_new(this_layer->name->str);
	}

	// Layer type specific data
	switch (this_layer->object_type)
	{
		case TYPE_EMPTY:
			copy_layer->object_data = (GObject *) g_try_new0(layer_empty, 1);
			if (NULL == copy_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				display_warning("Error ED289: Unable to allocate memory in the copy buffer for layer specific data.  Copy failed.");
				g_free(copy_layer);
				return;
			}
			((layer_empty *) copy_layer->object_data)->bg_color.pixel = ((layer_empty *) this_layer->object_data)->bg_color.pixel;
			((layer_empty *) copy_layer->object_data)->bg_color.red = ((layer_empty *) this_layer->object_data)->bg_color.red;
			((layer_empty *) copy_layer->object_data)->bg_color.green = ((layer_empty *) this_layer->object_data)->bg_color.green;
			((layer_empty *) copy_layer->object_data)->bg_color.blue = ((layer_empty *) this_layer->object_data)->bg_color.blue;
			break;

		case TYPE_GDK_PIXBUF:
			copy_layer->object_data = (GObject *) g_try_new0(layer_image, 1);
			if (NULL == copy_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				display_warning("Error ED290: Unable to allocate memory in the copy buffer for layer specific data.  Copy failed.");
				g_free(copy_layer);
				return;
			}
			((layer_image *) copy_layer->object_data)->width = ((layer_image *) this_layer->object_data)->width;
			((layer_image *) copy_layer->object_data)->height = ((layer_image *) this_layer->object_data)->height;
			((layer_image *) copy_layer->object_data)->image_path = g_string_new(((layer_image *) this_layer->object_data)->image_path->str);
			((layer_image *) copy_layer->object_data)->image_data = gdk_pixbuf_copy(((layer_image *) this_layer->object_data)->image_data);
			((layer_image *) copy_layer->object_data)->modified = ((layer_image *) this_layer->object_data)->modified;
			break;

		case TYPE_HIGHLIGHT:
			copy_layer->object_data = (GObject *) g_try_new0(layer_highlight, 1);
			if (NULL == copy_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				display_warning("Error ED291: Unable to allocate memory in the copy buffer for layer specific data.  Copy failed.");
				g_free(copy_layer);
				return;
			}
			((layer_highlight *) copy_layer->object_data)->width = ((layer_highlight *) this_layer->object_data)->width;
			((layer_highlight *) copy_layer->object_data)->height = ((layer_highlight *) this_layer->object_data)->height;
			break;

		case TYPE_MOUSE_CURSOR:
			copy_layer->object_data = (GObject *) g_try_new0(layer_mouse, 1);
			if (NULL == copy_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				display_warning("Error ED292: Unable to allocate memory in the copy buffer for layer specific data.  Copy failed.");
				g_free(copy_layer);
				return;
			}
			((layer_mouse *) copy_layer->object_data)->width = ((layer_mouse *) this_layer->object_data)->width;
			((layer_mouse *) copy_layer->object_data)->height = ((layer_mouse *) this_layer->object_data)->height;
			((layer_mouse *) copy_layer->object_data)->click = ((layer_mouse *) this_layer->object_data)->click;
			((layer_mouse *) copy_layer->object_data)->image_path = g_string_new(((layer_mouse *) this_layer->object_data)->image_path->str);
			break;

		case TYPE_TEXT:
			copy_layer->object_data = (GObject *) g_try_new0(layer_text, 1);
			if (NULL == copy_layer->object_data)
			{
				// We couldn't allocate memory for a new layer
				display_warning("Error ED293: Unable to allocate memory in the copy buffer for layer specific data.  Copy failed.");
				g_free(copy_layer);
				return;
			}
			((layer_text *) copy_layer->object_data)->rendered_width = ((layer_text *) this_layer->object_data)->rendered_width;
			((layer_text *) copy_layer->object_data)->rendered_height = ((layer_text *) this_layer->object_data)->rendered_height;
			((layer_text *) copy_layer->object_data)->text_color.pixel = ((layer_text *) this_layer->object_data)->text_color.pixel;
			((layer_text *) copy_layer->object_data)->text_color.red = ((layer_text *) this_layer->object_data)->text_color.red;
			((layer_text *) copy_layer->object_data)->text_color.green = ((layer_text *) this_layer->object_data)->text_color.green;
			((layer_text *) copy_layer->object_data)->text_color.blue = ((layer_text *) this_layer->object_data)->text_color.blue;
			((layer_text *) copy_layer->object_data)->font_size = ((layer_text *) this_layer->object_data)->font_size;

			// Copy the existing text buffer
			((layer_text *) copy_layer->object_data)->text_buffer = gtk_text_buffer_new(NULL);
			gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(((layer_text *) this_layer->object_data)->text_buffer), &text_start, &text_end);
			gtk_text_buffer_set_text(GTK_TEXT_BUFFER(((layer_text *) copy_layer->object_data)->text_buffer), gtk_text_buffer_get_slice(GTK_TEXT_BUFFER(((layer_text *) this_layer->object_data)->text_buffer), &text_start, &text_end, TRUE), -1);
			break;

		default:

			// No code written to handle this type of layer yet
			display_warning("Error ED288: Unknown layer type");
			break;
	}

	return;
}

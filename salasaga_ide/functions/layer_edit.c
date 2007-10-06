/*
 * $Id$
 *
 * Flame Project: Function called when the user clicks the Edit Layer toolbar button 
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
#include "create_timeline_slider.h"
#include "display_dialog_empty.h"
#include "display_dialog_highlight.h"
#include "display_dialog_image.h"
#include "display_dialog_mouse.h"
#include "display_dialog_text.h"
#include "display_warning.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "regenerate_timeline_duration_images.h"
#include "sound_beep.h"


void layer_edit(void)
{
	// Local variables
	GdkPixbuf			*layer_pixbuf;				// Pointer used when creating duration images for layers
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkListStore			*list_pointer;				//
	GtkWidget			*list_widget;				// Points to the timeline widget
	guint				num_layers;				// Number of layers
	guint				selected_row;				// Holds the row that is selected
	slide				*slide_data;				// Pointer to current slide data
	gboolean			return_code;				// Did the edit dialog return ok?

	guint				start_frame;				// Used when working out a layer's start frame
	guint				finish_frame;				// Used when working out a layer's finish frame
	gfloat				start_pixel;				// Starting slider pixel to fill in
	gfloat				finish_pixel;				// Ending slider pixel to fill in
	gfloat				pixel_width;				// Width of pixels to fill

	GtkTextIter			text_start;				// The start position of the text buffer
	GtkTextIter			text_end;				// The end position of the text buffer

	GtkTreeViewColumn		*tmp_column;				// Temporary column
	layer_empty			*tmp_empty_ob;				// Temporary empty layer object
	layer_highlight			*tmp_highlight_ob;			// Temporary highlight layer object
	layer_image			*tmp_image_ob;				// Temporary image layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	layer_text			*tmp_text_ob;				// Temporary text layer object
	GtkTreePath			*tmp_path;				// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise some variables
	slide_data = ((slide *) current_slide->data);
	layer_pointer = slide_data->layers;
	list_pointer = slide_data->layer_store;
	list_widget = slide_data->timeline_widget;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);

	// If the background layer is an image and it's selected, don't edit it
	if (1 == (num_layers - selected_row) && (TYPE_GDK_PIXBUF == tmp_layer->object_type))
	{
		// Warn the user then return
		sound_beep();
		display_warning("ED39: Background image layers can not be edited\n");
		return;
	}

	// * Open a dialog box showing the existing values, asking for the new ones *

	switch (tmp_layer->object_type)
	{
		case TYPE_EMPTY:
			// Open a dialog box for the user to edit the background layer values
			return_code = display_dialog_empty(tmp_layer, "Edit background color");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully, so update the slide list store with the new values *
				tmp_empty_ob = (layer_empty *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
			}
			break;

		case TYPE_GDK_PIXBUF:
			// Open a dialog box for the user to edit the image layer values
			return_code = display_dialog_image(tmp_layer, "Edit image layer", FALSE);
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer finish_frame is longer than the slide duration, then extend the slide duration
				if (tmp_layer->finish_frame > slide_data->duration)
				{
					slide_data->duration = tmp_layer->finish_frame;

					// Regenerate the duration images for the other layers as well!
					regenerate_timeline_duration_images(slide_data);
				}

				// Work out the start and ending frames for this layer
				start_frame = tmp_layer->start_frame;
				finish_frame = tmp_layer->finish_frame;

				// Calculate the duration of the layer for drawing inside the slider
				start_pixel = 180 * ((gfloat) start_frame / (gfloat) slide_data->duration);
				finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) slide_data->duration);
				pixel_width = finish_pixel - start_pixel;

				// Create duration image
				layer_pixbuf = NULL;
				layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);
				
				// Update the slide list store with the new values
				tmp_image_ob = (layer_image *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_DURATION, layer_pixbuf,
							TIMELINE_X_OFF_START, tmp_image_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_image_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_image_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_image_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_MOUSE_CURSOR:
			// Open a dialog box for the user to edit the mouse pointer values
			return_code = display_dialog_mouse(tmp_layer, "Edit mouse pointer", FALSE);
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer finish_frame is longer than the slide duration, then extend the slide duration
				if (tmp_layer->finish_frame > slide_data->duration)
				{
					slide_data->duration = tmp_layer->finish_frame;

					// Regenerate the duration images for the other layers as well!
					regenerate_timeline_duration_images(slide_data);
				}

				// Work out the start and ending frames for this layer
				start_frame = tmp_layer->start_frame;
				finish_frame = tmp_layer->finish_frame;

				// Calculate the duration of the layer for drawing inside the slider
				start_pixel = 180 * ((gfloat) start_frame / (gfloat) slide_data->duration);
				finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) slide_data->duration);
				pixel_width = finish_pixel - start_pixel;

				// Create duration image
				layer_pixbuf = NULL;
				layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);
				
				// Update the slide list store with the new values
				tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_DURATION, layer_pixbuf,
							TIMELINE_X_OFF_START, tmp_mouse_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_mouse_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_mouse_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_mouse_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_TEXT:
			// Open a dialog box for the user to edit the text layer values
			return_code = display_dialog_text(tmp_layer, "Edit text layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer finish_frame is longer than the slide duration, then extend the slide duration
				if (tmp_layer->finish_frame > slide_data->duration)
				{
					slide_data->duration = tmp_layer->finish_frame;

					// Regenerate the duration images for the other layers as well!
					regenerate_timeline_duration_images(slide_data);
				}

				// Work out the start and ending frames for this layer
				start_frame = tmp_layer->start_frame;
				finish_frame = tmp_layer->finish_frame;

				// Calculate the duration of the layer for drawing inside the slider
				start_pixel = 180 * ((gfloat) start_frame / (gfloat) slide_data->duration);
				finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) slide_data->duration);
				pixel_width = finish_pixel - start_pixel;

				// Create duration image
				layer_pixbuf = NULL;
				layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);
				
				// Update the slide list store with the new values
				tmp_text_ob = (layer_text *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_text_buffer_get_bounds(((layer_text *) tmp_layer->object_data)->text_buffer, &text_start, &text_end);
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_NAME, tmp_layer->name->str,
							TIMELINE_DURATION, layer_pixbuf,
							TIMELINE_X_OFF_START, tmp_text_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_text_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_text_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_text_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_HIGHLIGHT:
			// Open a dialog box for the user to edit the highlight layer values
			return_code = display_dialog_highlight(tmp_layer, "Edit highlight layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully *

				// If the new layer finish_frame is longer than the slide duration, then extend the slide duration
				if (tmp_layer->finish_frame > slide_data->duration)
				{
					slide_data->duration = tmp_layer->finish_frame;

					// Regenerate the duration images for the other layers as well!
					regenerate_timeline_duration_images(slide_data);
				}

				// Work out the start and ending frames for this layer
				start_frame = tmp_layer->start_frame;
				finish_frame = tmp_layer->finish_frame;

				// Calculate the duration of the layer for drawing inside the slider
				start_pixel = 180 * ((gfloat) start_frame / (gfloat) slide_data->duration);
				finish_pixel = 180 * ((gfloat) finish_frame / (gfloat) slide_data->duration);
				pixel_width = finish_pixel - start_pixel;

				// Create duration image
				layer_pixbuf = NULL;
				layer_pixbuf = create_timeline_slider(layer_pixbuf, 180, 20, start_pixel, pixel_width);
				
				// Update the slide list store with the new values
				tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_DURATION, layer_pixbuf,
							TIMELINE_X_OFF_START, tmp_highlight_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_highlight_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_highlight_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_highlight_ob->y_offset_finish,
							-1);
			}
			break;

		default:
			display_warning("ED34: Unknown layer type\n");
			return;
	}

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/06 11:39:27  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:14  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

/*
 * $Id$
 *
 * Salasaga: Function called when the user clicks the Edit Layer toolbar button 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
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


void layer_edit(void)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkListStore		*list_pointer;				//
	GtkWidget			*list_widget;				// Points to the timeline widget
	guint				num_layers;					// Number of layers
	guint				selected_row;				// Holds the row that is selected
	slide				*slide_data;				// Pointer to current slide data
	gboolean			return_code;				// Did the edit dialog return ok?

	GtkTextIter			text_start;					// The start position of the text buffer
	GtkTextIter			text_end;					// The end position of the text buffer

	layer_empty			*tmp_empty_ob;				// Temporary empty layer object
	layer_highlight		*tmp_highlight_ob;			// Temporary highlight layer object
	layer_image			*tmp_image_ob;				// Temporary image layer object
	GtkTreeIter			*tmp_iter;					// Temporary iter
	layer				*tmp_layer;					// Temporary layer
	layer_mouse			*tmp_mouse_ob;				// Temporary mouse layer object
	layer_text			*tmp_text_ob;				// Temporary text layer object
	GtkTreePath			*tmp_path;					// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Initialise some variables
	slide_data = (slide *) current_slide->data;
	layer_pointer = slide_data->layers;
	list_pointer = slide_data->layer_store;
	list_widget = slide_data->timeline_widget;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, NULL);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);

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
				}

				// Update the slide list store with the new values
				tmp_image_ob = (layer_image *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
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
				}

				// Update the slide list store with the new values
				tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
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
				}

				// Update the slide list store with the new values
				tmp_text_ob = (layer_text *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_text_buffer_get_bounds(((layer_text *) tmp_layer->object_data)->text_buffer, &text_start, &text_end);
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_NAME, tmp_layer->name->str,
							TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
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
				}

				// Update the slide list store with the new values
				tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(list_pointer, tmp_iter,
							TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
							-1);
			}
			break;

		default:
			display_warning("Error ED34: Unknown layer type\n");
			return;
	}

	// Regenerate the timeline duration images for all layers in this slide
	regenerate_timeline_duration_images(slide_data);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Layer edited");
	gdk_flush();

}

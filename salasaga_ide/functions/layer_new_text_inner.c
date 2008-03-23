/*
 * $Id$
 *
 * Salasaga: Displays a dialog box asking for the values required to make a new text layer
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


// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "display_dialog_text.h"
#include "draw_workspace.h"
#include "regenerate_film_strip_thumbnails.h"
#include "regenerate_timeline_duration_images.h"


void layer_new_text_inner(guint release_x, guint release_y)
{
	// Local variables
	guint				finish_frame;				// Used when working out a layer's finish frame
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkTreePath			*old_path = NULL;			// The old path, which we'll free
	gboolean			return_code;				// Catches a TRUE/FALSE return value
	slide				*slide_data;				// Pointer to the data for the current slide

	GtkTreeIter			*tmp_iter;					// Temporary iter
	layer				*tmp_layer;					// Temporary layer
	GtkTreePath			*tmp_path;					// Temporary GtkPath
	layer_text			*tmp_text_ob;				// Temporary text layer object


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		gdk_beep();
		return;
	}

	// Change the cursor back to normal
	gdk_window_set_cursor(main_drawing_area->window, NULL);

	// * Prepare a new text layer object in memory, with reasonable defaults *

	// Simplify pointing to the current slide structure in memory
	slide_data = current_slide->data;

	// Create the text layer data
	tmp_text_ob = g_new(layer_text, 1);
	tmp_text_ob->text_color.red = 0;
	tmp_text_ob->text_color.green = 0;
	tmp_text_ob->text_color.blue = 0;
	tmp_text_ob->font_size = 40;
	tmp_text_ob->text_buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), "New text...", -1);

	// Construct a new text layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_TEXT;
	tmp_layer->object_data = (GObject *) tmp_text_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_data->duration;
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new("Text layer");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new("_self");

	// Display a dialog box to edit these values, using our new text layer object
	return_code = display_dialog_text(tmp_layer, "Add new text layer");
	if (FALSE == return_code)
	{
		// The user cancelled out of the create text layer dialog box, so destroy our new text layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_string_free(tmp_layer->external_link, TRUE);
		g_string_free(tmp_layer->external_link_window, TRUE);
		g_free(tmp_layer);
		g_object_unref(tmp_text_ob->text_buffer);
		g_free(tmp_text_ob);

		// Update the status bar
		gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " New layer cancelled");
		gdk_flush();

		return;
	}

	// * To get here, the user must have clicked OK in the create text layer dialog box, so we process the results *

	// Add the new text layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Simplify the ending frame for this layer
	finish_frame = tmp_layer->finish_frame;

	// If the user gave a finish frame that's longer than the present slide duration, we need to increase the slide duration to match
	if (finish_frame > slide_data->duration)
	{
		// Update slide duration data
		slide_data->duration = finish_frame;
	}

	// Add the new text layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(slide_data->layer_store, tmp_iter);
	gtk_list_store_set(slide_data->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_DURATION, NULL,
						TIMELINE_X_OFF_START, tmp_layer->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_layer->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_layer->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_layer->y_offset_finish,
						-1);

	// Regenerate the timeline duration images
	regenerate_timeline_duration_images(slide_data);

	// Redraw the workspace
	draw_workspace();

	// Recreate the film strip thumbnails
	regenerate_film_strip_thumbnails();

	// Select the new layer in the timeline widget
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(film_strip_view), &tmp_path, NULL);
	if (NULL != tmp_path)
		old_path = tmp_path;  // Make a backup of the old path, so we can free it
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);
	if (NULL != old_path)
		gtk_tree_path_free(old_path);  // Free the old path

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Text layer added");
	gdk_flush();
}

/*
 * $Id$
 *
 * Salasaga: Function called when the user selects Slide -> Insert from the top menu 
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../preference/application_preferences.h"
#include "../preference/project_preferences.h"
#include "../undo_redo/undo_functions.h"


void slide_insert(void)
{
	// Local variables
	GtkTreeIter			film_strip_iter;
	guint				preview_height;				// The height we calculate a film strip thumbnail should be
	gfloat				project_ratio;				// Ratio of project height to width
	layer				*tmp_layer;					// Temporary layer
	slide				*tmp_slide;					// Temporary slide
	undo_history_data	*undo_item_data = NULL;		// Memory structure undo history items are created in


	// Create a new, empty slide
	tmp_slide = g_new(slide, 1);
	tmp_slide->layers = NULL;
	tmp_slide->name = NULL;
	tmp_slide->duration = get_default_slide_duration();
	tmp_slide->scaled_cached_pixmap = NULL;
	tmp_slide->cached_pixmap_valid = FALSE;
	tmp_slide->num_layers = 1;

	// Allocate a new layer structure for use in the slide
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_EMPTY;
	tmp_layer->start_time = 0;
	tmp_layer->duration = get_default_layer_duration();
	tmp_layer->visible = TRUE;
	tmp_layer->background = TRUE;
	tmp_layer->name = g_string_new(_("Empty"));
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new(_("_self"));
	tmp_layer->object_data = (GObject *) g_new(layer_empty, 1);
	((layer_empty *) tmp_layer->object_data)->bg_color.red = get_default_bg_colour_red();
	((layer_empty *) tmp_layer->object_data)->bg_color.green = get_default_bg_colour_green();
	((layer_empty *) tmp_layer->object_data)->bg_color.blue = get_default_bg_colour_blue();
	tmp_layer->x_offset_start = 0;
	tmp_layer->y_offset_start = 0;
	tmp_layer->x_offset_finish = 0;
	tmp_layer->y_offset_finish = 0;
	tmp_layer->transition_in_type = TRANS_LAYER_NONE;
	tmp_layer->transition_in_duration = 0.0;
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Determine the proper thumbnail height
	project_ratio = (gfloat) get_project_height() / (gfloat) get_project_width();
	preview_height = get_preview_width() * project_ratio;

	// Create a blank thumbnail using the default background colour, then add it to the new slide structure
	tmp_slide->thumbnail = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, get_preview_width(), preview_height);
	gdk_pixbuf_fill(tmp_slide->thumbnail, ((get_default_bg_colour_red() / 256) << 24)
		+ ((get_default_bg_colour_green() / 256) << 16)
		+ ((get_default_bg_colour_blue() / 256) << 8) + 0xff);

	// Add the empty layer to the new slide being created
	tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

	// Add the thumbnail to the GtkListView based film strip
	gtk_list_store_append(get_film_strip_store(), &film_strip_iter);  // Acquire an iterator
	gtk_list_store_set(get_film_strip_store(), &film_strip_iter, 0, tmp_slide->thumbnail, -1);

	// Set the timeline widget for the slide to NULL, so we know to create it later on
	tmp_slide->timeline_widget = NULL;

	// Create and store the undo history item for this layer
	undo_item_data = g_new0(undo_history_data, 1);
	undo_item_data->layer_data_new = NULL;  // NULL means not set
	undo_item_data->layer_data_old = NULL;  // NULL means not set
	undo_item_data->position_new = 0;
	undo_item_data->position_old = -1;  // -1 means not set
	undo_item_data->slide_data = tmp_slide;
	undo_history_add_item(UNDO_INSERT_SLIDE, undo_item_data, TRUE);

	// If the current slide hasn't been initialised (this is the first slide), then we initialise it
	if (NULL == get_current_slide())
	{
		slides = g_list_append(slides, tmp_slide);
		set_current_slide(slides);
	} else
	{
		// Add the newly created slide to the end of the slides
		slides = g_list_append(slides, tmp_slide);
	}

	// Set the changes made variable
	set_changes_made(TRUE);

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), _(" Slide added"));
	gdk_flush();
}

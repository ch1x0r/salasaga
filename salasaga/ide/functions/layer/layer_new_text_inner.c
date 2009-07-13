/*
 * $Id$
 *
 * Salasaga: Displays a dialog box asking for the values required to make a new text layer
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
#include "../draw_timeline.h"
#include "../callbacks/text_layer_create_colour_tag.h"
#include "../callbacks/text_layer_create_font_size_tag.h"
#include "../film_strip/film_strip_create_thumbnail.h"
#include "../widgets/time_line/time_line_set_selected_layer_num.h"
#include "../working_area/draw_workspace.h"


void layer_new_text_inner(guint release_x, guint release_y)
{
	// Local variables
	GList				*layer_pointer;				// Points to the layers in the selected slide
	GtkTextIter			selection_end;
	GtkTextIter			selection_start;
	slide				*slide_data;				// Pointer to the data for the current slide
	GdkColor			text_colour;				// Used to set the default colour of the text
	GtkTextTag			*text_tag;					// Used for the default text tags
	layer				*tmp_layer;					// Temporary layer
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
	tmp_text_ob->text_buffer = gtk_text_buffer_new(text_tags_table);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), _("New text..."), -1);

	// Apply default font of DejaVu Sans
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), &selection_start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), &selection_end);
	gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), salasaga_font_names[FONT_DEJAVU_SANS], &selection_start, &selection_end);

	// Apply default 40 point text size
	text_tag = text_layer_create_font_size_tag(40.0);
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), GTK_TEXT_TAG(text_tag), &selection_start, &selection_end);

	// Apply default black foreground colour
	text_colour.red = 0;
	text_colour.green = 0;
	text_colour.blue = 0;
	text_tag = text_layer_create_colour_tag(&text_colour);
	gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), GTK_TEXT_TAG(text_tag), &selection_start, &selection_end);

	tmp_text_ob->show_bg = TRUE;
	tmp_text_ob->bg_border_width = 1.0;
	tmp_text_ob->bg_border_colour.red = 0;
	tmp_text_ob->bg_border_colour.green = 0;
	tmp_text_ob->bg_border_colour.blue = 0;
	tmp_text_ob->bg_fill_colour.red = 65535;
	tmp_text_ob->bg_fill_colour.green = 65535;
	tmp_text_ob->bg_fill_colour.blue = 52428;  // Sensible defaults
	tmp_text_ob->rendered_width = 0;
	tmp_text_ob->rendered_height = 0;

	// Construct a new text layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_TEXT;
	tmp_layer->object_data = (GObject *) tmp_text_ob;
	tmp_layer->start_time = 0.0;
	tmp_layer->duration = default_layer_duration;
	tmp_layer->x_offset_start = release_x;
	tmp_layer->y_offset_start = release_y;
	tmp_layer->x_offset_finish = release_x;
	tmp_layer->y_offset_finish = release_y;
	tmp_layer->visible = TRUE;
	tmp_layer->background = FALSE;
	tmp_layer->name = g_string_new(_("Text layer"));
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->external_link_window = g_string_new(_("_self"));
	tmp_layer->transition_in_type = TRANS_LAYER_NONE;
	tmp_layer->transition_in_duration = 0.0;
	tmp_layer->transition_out_type = TRANS_LAYER_NONE;
	tmp_layer->transition_out_duration = 0.0;

	// Add the new text layer to the slide
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);
	slide_data->num_layers++;

	// If the new layer end time is longer than the slide duration, then extend the slide duration
	if (tmp_layer->duration > slide_data->duration)
	{
		// Change the slide duration
		slide_data->duration = tmp_layer->duration;

		// Change the background layer duration
		tmp_layer = g_list_nth_data(layer_pointer, slide_data->num_layers - 1);
		tmp_layer->duration = slide_data->duration;
	}

	// Regenerate the time line
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Recreate the slide thumbnail
	film_strip_create_thumbnail(slide_data);

	// Select the new layer in the time line widget
	time_line_set_selected_layer_num(slide_data->timeline_widget, 0);

	// Set the changes made variable
	changes_made = TRUE;

	// Update the status bar
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(status_bar), _(" Text layer added"));
	gdk_flush();
}

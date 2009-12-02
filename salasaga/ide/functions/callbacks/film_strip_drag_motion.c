/*
 * $Id$
 *
 * Salasaga: Callback function called when the user drags a slide around in the film strip.
 *           This function moves the currently selected slide to the new position the user dragged it to.
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Math include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../../externs.h"


gboolean film_strip_drag_motion(GtkWidget *widget, GdkDragContext *drag_context, gint x, gint y, guint t, gpointer user_data)
{
	// Local variables
	gint				actual_y_pos;				// The target Y pixel position in the whole film strip
	GtkTreeIter			current_slide_iter;			// Points to the currently selected slide
	gint				current_slide_position;		// Points to the currently selected slide
	slide				*current_slide_data;		// Pointer to the data for the currently selected slide
	gdouble				film_strip_adjustment_present_value;
	gint				num_slides;					// The number of slides in the project
	gint				slide_height;
	GList				*target_slide;				// Pointer to the target slide
	GtkTreeIter			target_slide_iter;
	gint				target_slide_position;
	slide				*this_slide;
	GString				*tmp_gstring;
	GtkAdjustment		*vert_adjustment;


	// Initialisation
	this_slide = current_slide->data;

	// Find out how many pixels high each slide is in the film strip
	gtk_tree_view_column_cell_get_size(GTK_TREE_VIEW_COLUMN(film_strip_column), NULL, NULL, NULL, NULL, &slide_height);

	// Work out the actual Y position the user has dragged to in the entire film strip
	vert_adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(film_strip_container));
	film_strip_adjustment_present_value = gtk_adjustment_get_value(GTK_ADJUSTMENT(vert_adjustment));
	actual_y_pos = y + film_strip_adjustment_present_value;

	// Work out which slide in the film strip the actual y position points to
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	current_slide_position = g_list_position(slides, current_slide);
	target_slide_position = roundf(actual_y_pos / slide_height);

	// Make sure the target slide position isn't pointing past the last slide
	if ((num_slides - 1) < target_slide_position)
	{
		target_slide_position = num_slides - 1;
	}

	// Do we need to move the selected slide to a new position?
	if (current_slide_position != target_slide_position)
	{
		// Create a temporary GString
		tmp_gstring = g_string_new(NULL);

		// Create iters pointing to the current and target slide positions
		g_string_printf(tmp_gstring, "%u", current_slide_position);
		gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &current_slide_iter, tmp_gstring->str);
		g_string_printf(tmp_gstring, "%u", target_slide_position);
		gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(film_strip_store), &target_slide_iter, tmp_gstring->str);

		// Swap the slides in the film strip area
		gtk_list_store_swap(GTK_LIST_STORE(film_strip_store), &current_slide_iter, &target_slide_iter);

		// Swap the slides in the slide list
		current_slide_data = current_slide->data;
		target_slide = g_list_nth(slides, target_slide_position);
		current_slide->data = target_slide->data;
		target_slide->data = current_slide_data;
		current_slide = target_slide;

		// Free the temporary GString
		g_string_free(tmp_gstring, TRUE);
	}

	return  FALSE;
}

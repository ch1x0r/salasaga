/*
 * $Id$
 *
 * Salasaga: Construct the widget used to display the slide in the timeline 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../salasaga_types.h"
#include "../externs.h"
#include "timeline_edited_name.h"
#include "timeline_edited_visibility.h"
#include "timeline_edited_x_offset_finish.h"
#include "timeline_edited_x_offset_start.h"
#include "timeline_edited_y_offset_finish.h"
#include "timeline_edited_y_offset_start.h"


GtkWidget *construct_timeline_widget(slide *slide_data)
{
	// Local variables
	gint					layer_counter;
	layer					*layer_data;
	GList					*layer_pointer;
	gint					num_layers;
	GtkCellRenderer			*timeline_renderer_pixbuf_duration;
	GtkCellRenderer			*timeline_renderer_text_name;
	GtkCellRenderer			*timeline_renderer_text_x_offset_finish;
	GtkCellRenderer			*timeline_renderer_text_x_offset_start;
	GtkCellRenderer			*timeline_renderer_text_y_offset_finish;
	GtkCellRenderer			*timeline_renderer_text_y_offset_start;
	GtkCellRenderer			*timeline_renderer_visible;

	GtkTreePath				*tmp_path;				// Temporary GtkPath


	// * This function assumes the existing timeline widget has not been created, or has been destroyed *

	// Create the timeline widget for this slide
	slide_data->timeline_widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(slide_data->layer_store));

	// Create cell renderers
	timeline_renderer_text_name = gtk_cell_renderer_text_new();
	timeline_renderer_pixbuf_duration = gtk_cell_renderer_pixbuf_new();
	timeline_renderer_text_x_offset_start = gtk_cell_renderer_text_new();
	timeline_renderer_text_y_offset_start = gtk_cell_renderer_text_new();
	timeline_renderer_text_x_offset_finish = gtk_cell_renderer_text_new();
	timeline_renderer_text_y_offset_finish = gtk_cell_renderer_text_new();
	timeline_renderer_visible = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(timeline_renderer_visible), FALSE);

	// Hook a signal handler to the name column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_name), "edited", G_CALLBACK(timeline_edited_name), NULL);

	// Hook a signal handler to the visibility column renderer
	g_signal_connect (G_OBJECT (timeline_renderer_visible), "toggled", G_CALLBACK (timeline_edited_visibility), GTK_TREE_MODEL(slide_data->layer_store));
	
	// Hook a signal handler to the x offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_start), "edited", G_CALLBACK(timeline_edited_x_offset_start), NULL);

	// Hook a signal handler to the y offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_start), "edited", G_CALLBACK(timeline_edited_y_offset_start), NULL);

	// Hook a signal handler to the x offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_finish), "edited", G_CALLBACK(timeline_edited_x_offset_finish), NULL);

	// Hook a signal handler to the y offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_finish), "edited", G_CALLBACK(timeline_edited_y_offset_finish), NULL);

	// Create name column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Name", timeline_renderer_text_name, "text", TIMELINE_NAME, "editable", TRUE, NULL);

	// Create visibility column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Visible", timeline_renderer_visible, "active", TIMELINE_VISIBILITY, NULL);

	// Create duration column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Duration", timeline_renderer_pixbuf_duration, "pixbuf", TIMELINE_DURATION, NULL);

	// Create X offset start column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Start X", timeline_renderer_text_x_offset_start, "text", TIMELINE_X_OFF_START, "editable", TRUE, NULL);

	// Create Y offset start column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Start Y", timeline_renderer_text_y_offset_start, "text", TIMELINE_Y_OFF_START, "editable", TRUE, NULL);

	// Create X offset finish column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Finish X", timeline_renderer_text_x_offset_finish, "text", TIMELINE_X_OFF_FINISH, "editable", TRUE, NULL);

	// Create Y offset finish column
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(slide_data->timeline_widget), -1, "Finish Y", timeline_renderer_text_y_offset_finish, "text", TIMELINE_Y_OFF_FINISH, "editable", TRUE, NULL);

	// Set up some pointers to make things easier
	layer_pointer = slide_data->layers;
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Loop through the layers, setting the visibility checkbox in the timeline widget correctly for each
	for (layer_counter = 0; layer_counter < num_layers; layer_counter++)
	{
		layer_pointer = g_list_first(layer_pointer);
		layer_data = g_list_nth_data(layer_pointer, layer_counter);

		if (TRUE == layer_data->visible)
		{
			// This layer IS visible
			gtk_list_store_set(GTK_LIST_STORE(slide_data->layer_store), layer_data->row_iter, TIMELINE_VISIBILITY, TRUE, -1);
		} else
		{
			// This layer IS NOT visible
			gtk_list_store_set(GTK_LIST_STORE(slide_data->layer_store), layer_data->row_iter, TIMELINE_VISIBILITY, FALSE, -1);
		}
	}

	// Select the top row in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);

	return slide_data->timeline_widget;
}

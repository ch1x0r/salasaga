/*
 * $Id$
 *
 * Flame Project: Construct the widget used to display the slide in the timeline 
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


GtkWidget *construct_timeline_widget(slide *slide_data)
{
	// Local variables
	GtkCellRenderer			*timeline_renderer_text_name;
	GtkCellRenderer			*timeline_renderer_toggle;
	GtkCellRenderer			*timeline_renderer_pixbuf_duration;
	GtkCellRenderer			*timeline_renderer_text_x_offset_start;
	GtkCellRenderer			*timeline_renderer_text_y_offset_start;
	GtkCellRenderer			*timeline_renderer_text_x_offset_finish;
	GtkCellRenderer			*timeline_renderer_text_y_offset_finish;
	GtkTreeViewColumn		*timeline_column_name;
	GtkTreeViewColumn		*timeline_column_visibility;
	GtkTreeViewColumn		*timeline_column_duration;
	GtkTreeViewColumn		*timeline_column_x_off_start;
	GtkTreeViewColumn		*timeline_column_y_off_start;
	GtkTreeViewColumn		*timeline_column_x_off_finish;
	GtkTreeViewColumn		*timeline_column_y_off_finish;

	GtkTreePath			*tmp_path;				// Temporary GtkPath


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
	timeline_renderer_toggle = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(timeline_renderer_toggle), FALSE);

	// Hook a signal handler to the name column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_name), "edited", G_CALLBACK(timeline_edited_name), NULL);

	// Hook a signal handler to the start column renderer
//	g_signal_connect(G_OBJECT(timeline_renderer_text_start), "edited", G_CALLBACK(timeline_edited_start), NULL);

	// Hook a signal handler to the finish column renderer
//	g_signal_connect(G_OBJECT(timeline_renderer_text_finish), "edited", G_CALLBACK(timeline_edited_finish), NULL);

	// Hook a signal handler to the x offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_start), "edited", G_CALLBACK(timeline_edited_x_offset_start), NULL);

	// Hook a signal handler to the y offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_start), "edited", G_CALLBACK(timeline_edited_y_offset_start), NULL);

	// Hook a signal handler to the x offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_finish), "edited", G_CALLBACK(timeline_edited_x_offset_finish), NULL);

	// Hook a signal handler to the y offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_finish), "edited", G_CALLBACK(timeline_edited_y_offset_finish), NULL);

	// Create name column
	timeline_column_name = gtk_tree_view_column_new_with_attributes("Name", timeline_renderer_text_name, "text", TIMELINE_NAME, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_name);

	// Create visibility column
	timeline_column_visibility = gtk_tree_view_column_new_with_attributes("Visible", timeline_renderer_toggle, "active", TIMELINE_VISIBILITY, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_visibility);

	// Create duration column
	timeline_column_duration = gtk_tree_view_column_new_with_attributes("Duration", timeline_renderer_pixbuf_duration, "pixbuf", TIMELINE_DURATION, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_duration);

	// Create X offset start column
	timeline_column_x_off_start = gtk_tree_view_column_new_with_attributes("Start X", timeline_renderer_text_x_offset_start, "text", TIMELINE_X_OFF_START, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_x_off_start);

	// Create Y offset start column
	timeline_column_y_off_start = gtk_tree_view_column_new_with_attributes("Start Y", timeline_renderer_text_y_offset_start, "text", TIMELINE_Y_OFF_START, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_y_off_start);

	// Create X offset finish column
	timeline_column_x_off_finish = gtk_tree_view_column_new_with_attributes("Finish X", timeline_renderer_text_x_offset_finish, "text", TIMELINE_X_OFF_FINISH, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_x_off_finish);

	// Create Y offset finish column
	timeline_column_y_off_finish = gtk_tree_view_column_new_with_attributes("Finish Y", timeline_renderer_text_y_offset_finish, "text", TIMELINE_Y_OFF_FINISH, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_y_off_finish);

	// Select the top row in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);

	return slide_data->timeline_widget;
}


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/29 04:22:15  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

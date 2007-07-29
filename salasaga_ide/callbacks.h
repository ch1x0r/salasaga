/*
 * $Id$
 *
 * Flame Project: Header file for all call back functions
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Callback functions defined in the corresponding source file
gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data);
void film_strip_handle_changed(GObject *paned, GParamSpec *pspec, gpointer data);
gint film_strip_handle_released(GObject *paned, GParamSpec *pspec, gpointer data);
void film_strip_slide_clicked(GtkTreeSelection *selection, gpointer data);
gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data);
//void timeline_edited_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_name(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
//void timeline_edited_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_x_offset_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_x_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_y_offset_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_y_offset_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gint zoom_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CALLBACKS_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.9  2007/07/29 05:22:20  vapour
 * Began modifying timeline widget to use a pixbuf for display of time duration.
 *
 * Revision 1.8  2007/07/09 12:23:35  vapour
 * Added the declaration for the new film_strip_handle_released function.
 *
 * Revision 1.7  2007/07/08 14:05:46  vapour
 * Added the declaration for the new film_strip_handle_changed function.
 *
 * Revision 1.6  2007/06/30 06:04:19  vapour
 * The timeline and workspace area are now updated when a slide is selected in the film strip.  All done with the GtkTreeView approach now.
 *
 * Revision 1.5  2006/07/09 11:44:02  vapour
 * Reordered the functions alphabetically.
 *
 * Revision 1.4  2006/04/22 08:36:54  vapour
 * + Replaced the text string display in the timeline (layer) widget area, with the x and y finish positions.
 * + Updated the entire project to use the word "finish" consistently, instead of "final".
 *
 * Revision 1.3  2006/04/21 17:46:40  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.2  2006/04/16 06:04:45  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

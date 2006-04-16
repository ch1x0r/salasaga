/*
 * $Id$
 * 
 * Flame Project: Header file for all call back functions
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2006/04/16 06:04:45  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

#ifndef __CALLBACKS_H__
#define __CALLBACKS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Callback functions defined in the corresponding source file
gint event_size_allocate_received(GtkWidget *widget, GdkEvent *event, gpointer data);
gint film_strip_slide_clicked(GtkWidget *widget, GdkEvent *event, slide *clicked_slide);
gint resolution_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data);
void timeline_edited_name(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_start(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_final(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_x_offset(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_y_offset(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
void timeline_edited_text(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);
gboolean working_area_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean working_area_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gboolean working_area_expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data);
gboolean working_area_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
gint zoom_selector_changed(GtkWidget *widget, GdkEvent *event, gpointer data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CALLBACKS_H__

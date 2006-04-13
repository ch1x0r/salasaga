/*
 * $Id$
 * 
 * Flame Project: Header file for all call back functions
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.9  2006/04/09 12:13:26  jc
 * Moved the film_strip_slide_clicked function into the callbacks file.
 *
 * Revision 1.8  2006/04/06 03:32:25  jc
 * Moved slide_insert(), slide_delete(), slide_move_up(), and slide_move_down() into gui-functions.h.
 *
 * Revision 1.7  2006/04/04 13:31:48  jc
 * Added stub functions for working with slides.
 *
 * Revision 1.6  2006/03/31 05:08:51  jc
 * Added working code for handling mouse drags by detecting mouse button release.
 *
 * Revision 1.5  2006/03/22 09:24:36  jc
 * Added an initial (stub) handler for capturing mouse motion events on the drawing area.
 *
 * Revision 1.4  2006/03/22 07:33:32  jc
 * Added an event handler and initial working code to the drawing area, for catching mouse clicks.
 *
 * Revision 1.3  2006/03/22 07:09:07  jc
 * Moved the zoom and resolution selector change callbacks in here.
 *
 * Revision 1.2  2006/03/22 07:04:48  jc
 * Moved the working_area_expose_event callback in here.
 *
 * Revision 1.1  2006/03/13 04:02:24  jc
 * Added a new header and source file to contain callback functions.  Populated it with working callbacks triggered when the user edits fields in the timeline widget.
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

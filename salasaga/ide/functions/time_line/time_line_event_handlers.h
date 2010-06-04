/*
 * $Id: time_line_event_handlers.c 2659 2010-05-27 15:01:48Z allupaku $
 *
 * Salasaga: Event Handling functions for the time line widget
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
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

#ifndef TIME_LINE_EVENT_HANDLERS_H_
#define TIME_LINE_EVENT_HANDLERS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Event Handler for bottom right
// ----- selection , dragging , resizing , and switching layer positions
void bot_right_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

void bot_right_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

void bot_right_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

// Event handlers for top right
// -----  Cursor dragging
void top_right_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

void top_right_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

void top_right_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

// Event handlers for top left = + / - symbols
// -----  Expansion / Reduction
void top_left_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TIME_LINE_EVENT_HANDLERS_H_ */

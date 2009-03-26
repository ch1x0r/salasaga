/*
 * $Id$
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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

#ifndef TIME_LINE_H_
#define TIME_LINE_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Include needed classes
#include <glib.h>
#include <glib-object.h>

// Macros	
#define TIME_LINE_TYPE (time_line_get_type())
#define TIME_LINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TIME_LINE_TYPE, TimeLine))
#define TIME_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TIME_LINE_TYPE, TimeLineClass))
#define IS_TIME_LINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TIME_LINE_TYPE))
#define IS_TIME_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TIME_LINE_TYPE))

// Required structures
typedef struct _TimeLine TimeLine;
typedef struct _TimeLineClass TimeLineClass;
struct _TimeLine
{
	GtkDrawingArea drawing_area;
};

struct _TimeLineClass
{
	GtkDrawingAreaClass parent_class;
};

// Public function definitions
GType time_line_get_type(void) G_GNUC_CONST;
GtkWidget* time_line_new(void);
gboolean time_line_regenerate_widget(GtkWidget *widget);
gfloat time_line_get_cursor_position(GtkWidget *widget);
gint time_line_get_selected_layer_num(GtkWidget *widget);
gboolean time_line_set_selected_layer_num(GtkWidget *widget, gint selected_row);
gboolean time_line_set_stored_slide_duration(GtkWidget *widget, gfloat new_duration);
void timeline_widget_motion_notify_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
void timeline_widget_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
void timeline_widget_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*TIME_LINE_H_*/
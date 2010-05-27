/*
 * $Id$
 *
 * Salasaga: Instance initialiser for the time line widget
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


// Turn on C99 compatibility - needed for roundf() to work
#define _ISOC99_SOURCE

// Standard include
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_widget_motion_notify_handler.h"
#include "time_line_internal_make_widgets.h"
#include "../working_area/draw_workspace.h"


gboolean expose_event_top_left(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;
	GString				*message;
	TimeLinePrivate		*priv;
	priv = (TimeLinePrivate *)user;
	if(priv->top_left_evb->window == NULL || priv->display_buffer_top_left == NULL){
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED358: %s", _("Error"), _("Null data in window"));
				display_warning(message->str);
				g_string_free(message, TRUE);
	}
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->top_left_evb->window));
	}

	gdk_draw_drawable(GDK_DRAWABLE(priv->top_left_evb->window), GDK_GC(this_gc),
	GDK_PIXMAP(priv->display_buffer_top_left),0, 0,	0, 0,priv->left_border_width, priv->top_border_height);
	//gtk_widget_modify_bg(GTK_WIDGET(priv->top_left_evb),GTK_STATE_NORMAL,&entireRowColor);
	return TRUE;
}


gboolean expose_event_top_right(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;
	GString				*message;
	TimeLinePrivate		*priv;
	priv = (TimeLinePrivate *)user;

	if(priv->top_right_evb->window == NULL || priv->display_buffer_top_right == NULL){
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED358: %s", _("Error"), _("Null data in window"));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return TRUE;
	}
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->top_right_evb->window));
	}
	gdk_draw_drawable(GDK_DRAWABLE(priv->top_right_evb->window), GDK_GC(this_gc),
	GDK_PIXMAP(priv->display_buffer_top_right),0,0,0,0,1000, priv->top_border_height);
	return TRUE;
}

gboolean expose_event_bot_right(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;
	GString				*message;
	TimeLinePrivate		*priv;
	priv = (TimeLinePrivate *)user;

	if(priv->bot_right_evb->window == NULL || priv->display_buffer_bot_right == NULL){
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED358: %s", _("Error"), _("Null data in window"));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return TRUE;
	}
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->bot_right_evb->window));
	}
	gdk_draw_drawable(GDK_DRAWABLE(priv->bot_right_evb->window), GDK_GC(this_gc),
	GDK_PIXMAP(priv->display_buffer_bot_right),0,0,0,0,1000, 1000);
	return TRUE;
}


gboolean expose_event_bot_left(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;
	GString				*message;
	TimeLinePrivate		*priv;
	priv = (TimeLinePrivate *)user;

	if(priv->bot_left_evb->window == NULL || priv->display_buffer_bot_left == NULL){
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED358: %s", _("Error"), _("Null data in window"));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return TRUE;
	}
	if (NULL == this_gc)
	{
		this_gc = gdk_gc_new(GDK_DRAWABLE(priv->bot_left_evb->window));
	}
	gdk_draw_drawable(GDK_DRAWABLE(priv->bot_left_evb->window), GDK_GC(this_gc),
	GDK_PIXMAP(priv->display_buffer_bot_left),0,0,0,0,priv->left_border_width,1000);
	return TRUE;
}




gboolean realize_allocate(GtkWidget *widget,gpointer user_data)
{

	TimeLinePrivate		*priv;
//	GString *message;
	priv = (TimeLinePrivate *)user_data;
//	message = g_string_new(NULL);
	if(widget->allocation.width > priv->left_border_width && (widget->allocation.width <1200) ){
	priv->main_width = widget->allocation.width - priv->left_border_width;
	priv->main_height = widget->allocation.height - priv->top_border_height;
//	g_string_printf(message, "%d : %d",priv->main_width,widget->allocation.height);
//	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(get_status_bar()), message->str);
//	g_string_free(message, TRUE);
	gtk_widget_set_size_request(priv->top_left_vp,priv->left_border_width,priv->top_border_height);
	gtk_widget_set_size_request(priv->bot_left_vp,priv->left_border_width,priv->main_height);
	gtk_widget_set_size_request(priv->top_right_vp,priv->main_width,priv->top_border_height);
	gtk_widget_set_size_request(priv->bot_right_vp,priv->main_width,priv->main_height);

	}
	time_line_internal_draw_selection_highlight(priv, WIDGET_MINIMUM_WIDTH);
	return TRUE;
}

void time_line_init(TimeLine *time_line)
{
	// Local variables
	TimeLinePrivate		*priv;

//	GString *message;
//	message = g_string_new(NULL);
//				g_string_printf(message, "%s ED358: %s", _("Error"), _("In initialize"));
//				display_warning(message->str);
//				g_string_free(message, TRUE);
	// Initialise variable defaults
	time_line_set_pixels_per_second(60);
	priv = TIME_LINE_GET_PRIVATE(time_line);
	priv->cached_bg_valid = FALSE;
	priv->cursor_drag_active = FALSE;
	priv->display_buffer_bot_right = NULL;
	priv->drag_active = FALSE;
	priv->resize_type = RESIZE_NONE;
	priv->selected_layer_num = 0;
	priv->stored_x = 0;
	priv->stored_y = 0;
	priv->guide_line_start = 0;
	priv->guide_line_end = 0;

	// Store the slide duration
	priv->stored_slide_duration = get_current_slide_duration();

	// fixme3: These may be better as widget properties
	priv->cursor_position = 0.0;
	priv->left_border_width = 120;
	priv->row_height = 20;
	priv->top_border_height = 15;

	//time_line_internal_make_widgets(priv);
	time_line_internal_make_widgets(priv,WIDGET_MINIMUM_WIDTH,WIDGET_MINIMUM_HEIGHT);
	// Call our internal time line function to create the cached background image
	time_line_internal_initialise_bg_image(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);
	// Call our internal function to create the display buffer
	time_line_internal_initialise_display_buffer(priv, WIDGET_MINIMUM_WIDTH, WIDGET_MINIMUM_HEIGHT);
	realize_allocate(get_time_line_container(),priv);
	time_line_internal_draw_layer_info(priv);

	gtk_box_pack_start(GTK_BOX(time_line), GTK_WIDGET(priv->main_table), TRUE, TRUE, 0);

	//realize_allocate(get_time_line_container(),priv);
	g_signal_connect_after(priv->main_table, "realize", G_CALLBACK(realize_allocate),priv);

	g_signal_connect(priv->top_left_evb, "expose-event", G_CALLBACK(expose_event_top_left),priv);

	g_signal_connect(priv->top_right_evb, "expose-event", G_CALLBACK(expose_event_top_right),priv);

	g_signal_connect(priv->bot_right_evb, "expose-event", G_CALLBACK(expose_event_bot_right),priv);

	g_signal_connect(priv->bot_left_evb, "expose-event", G_CALLBACK(expose_event_bot_left),priv);

	// Draw the layer information


	// Select the highlighted layer
	time_line_internal_draw_selection_highlight(priv, WIDGET_MINIMUM_WIDTH);

	// Set a periodic time out, so we rate limit the calls to the motion notify (mouse drag) handler
	priv->mouse_x = -1;
	priv->mouse_y = -1;
	priv->timeout_id = g_timeout_add(330,  // Timeout interval (1/1000ths of a second)
							(GSourceFunc) draw_workspace,  // Function to call
							 NULL); //time_line);  // Pass the time line widget to the function
}

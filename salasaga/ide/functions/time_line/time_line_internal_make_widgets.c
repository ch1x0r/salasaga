/*
 * $Id: time_line_internal_make_widgets.c  by Althaf $
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

#include <gtk/gtk.h>

#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "../dialog/display_warning.h"

#include "top_left_button_release_event.h"
#include "top_left_button_press_event.h"
#include "top_left_motion_notify_event.h"
#include "top_right_button_press_event.h"
#include "top_right_button_release_event.h"
#include "top_right_motion_notify_event.h"
#include "bot_left_button_press_event.h"
#include "bot_left_button_release_event.h"
#include "bot_left_motion_notify_event.h"
#include "bot_right_button_press_event.h"
#include "bot_right_button_release_event.h"
#include "bot_right_motion_notify_event.h"

#include "realize_allocate_table.h"
#include "expose_table.h"
#include "size_allocate_table.h"

#include "expose_event_top_left.h"
#include "expose_event_top_right.h"
#include "expose_event_bot_right.h"
#include "expose_event_bot_left.h"



gboolean time_line_internal_make_widgets(TimeLinePrivate *priv, guint width, guint height){
	GtkWidget * align;

	if(NULL == priv->main_table){
		//0. Creating a new table
		priv->main_table = gtk_table_new(2,2,FALSE);
		// Setting the row and col spacing ofthe table
		gtk_table_set_row_spacings(GTK_TABLE(priv->main_table),0);
		gtk_table_set_col_spacings(GTK_TABLE(priv->main_table),0);

		//1.Creating a new scrolled window - top left part of the time line
		priv->top_left_swin = gtk_scrolled_window_new(NULL,NULL);
		// Setting the suitable properties
		gtk_container_set_border_width(GTK_CONTAINER(priv->top_left_swin),0);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->top_left_swin),GTK_SHADOW_NONE);
		//Creating a view port for the top left
		priv->top_left_vp = gtk_viewport_new(NULL,NULL);
		gtk_container_add(GTK_CONTAINER(priv->top_left_swin),GTK_WIDGET(priv->top_left_vp));
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(priv->top_left_vp),GTK_SHADOW_NONE);
		// Creating the event box
		priv->top_left_evb = gtk_event_box_new();
		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_evb),120,20);
		gtk_container_add(GTK_CONTAINER(priv->top_left_vp),GTK_WIDGET(priv->top_left_evb));

		// Setting the scrolled window policy so that the scroll bar is never shown.
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->top_left_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
		// adding the scrolled window to the table
		align = gtk_alignment_new(0,0,1,1);
		gtk_container_add(GTK_CONTAINER(align),priv->top_left_swin);
		gtk_table_attach_defaults(GTK_TABLE(priv->main_table),GTK_WIDGET(align),0,1,0,1);


		//2. Creating the right scrolled window for the seconds / time bar - top right
		priv->top_right_swin = gtk_scrolled_window_new(NULL,NULL);
		gtk_container_set_border_width(GTK_CONTAINER(priv->top_right_swin),0);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->top_right_swin),GTK_SHADOW_NONE);
		// Creating the view port and setting the properties for acvoiding all kinds of borders
		priv->top_right_vp = gtk_viewport_new(NULL,NULL);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(priv->top_right_vp),GTK_SHADOW_NONE);
		gtk_container_add(GTK_CONTAINER(priv->top_right_swin),GTK_WIDGET(priv->top_right_vp));
		//creating the event box
		priv->top_right_evb = gtk_event_box_new();
		// Label need to be removed
		//trial_label = gtk_label_new("Hello This is a trial to know the scroll");
		//gtk_container_add(GTK_CONTAINER(priv->top_right_evb),GTK_WIDGET(trial_label));
		gtk_container_add(GTK_CONTAINER(priv->top_right_vp),GTK_WIDGET(priv->top_right_evb));
		// setting the width as 500 random.
//		gtk_widget_set_size_request(GTK_WIDGET(priv->top_right_evb),1000,20);
//		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_vp),400,20);
		//never show the scroll
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->top_right_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);

		align = gtk_alignment_new(0,0,1,1);
		gtk_container_add(GTK_CONTAINER(align),priv->top_right_swin);
		gtk_table_attach(GTK_TABLE(priv->main_table),GTK_WIDGET(align),1,2,0,1,GTK_FILL,GTK_FILL,0,0);



		// 3. creating scrolled window for left - bottom left
		priv->bot_left_swin = gtk_scrolled_window_new(NULL,NULL);
		gtk_container_set_border_width(GTK_CONTAINER(priv->bot_left_swin),0);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->bot_left_swin),GTK_SHADOW_NONE);
		// creating a view port and seeeing the border properties
		priv->bot_left_vp = gtk_viewport_new(NULL,NULL);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(priv->bot_left_vp),GTK_SHADOW_NONE);
		gtk_container_add(GTK_CONTAINER(priv->bot_left_swin),GTK_WIDGET(priv->bot_left_vp));
		// creating the event box.
		priv->bot_left_evb = gtk_event_box_new();
		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_evb),priv->left_border_width,height);
		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_vp),priv->left_border_width,height-priv->top_border_height);

		gtk_container_add(GTK_CONTAINER(priv->bot_left_vp),GTK_WIDGET(priv->bot_left_evb));

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->bot_left_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
		align = gtk_alignment_new(0,0,1,1);
		gtk_container_add(GTK_CONTAINER(align),priv->bot_left_swin);
		gtk_table_attach(GTK_TABLE(priv->main_table),GTK_WIDGET(align),0,1,1,2,GTK_FILL,GTK_FILL,0,0);


		//4. Scrolled window for the layer part
		priv->bot_right_swin = gtk_scrolled_window_new(NULL,NULL);
		gtk_container_set_border_width(GTK_CONTAINER(priv->bot_right_swin),0);
		gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(priv->bot_right_swin),GTK_SHADOW_NONE);
		// view port for the important part
		priv->bot_right_vp = gtk_viewport_new(NULL,NULL);
		gtk_viewport_set_shadow_type(GTK_VIEWPORT(priv->bot_right_vp),GTK_SHADOW_NONE);
		gtk_container_add(GTK_CONTAINER(priv->bot_right_swin),GTK_WIDGET(priv->bot_right_vp));
		// creating the event box for the layers
		priv->bot_right_evb = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(priv->bot_right_vp),GTK_WIDGET(priv->bot_right_evb));

		// connectin the scrolled window adjustment - horizontal of the 4 the scrolled window - the main one - to the
		// hadjustment of the second one, which is above it. So that when it is scrolled horizontally, both will be scrolled together
		gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(priv->top_right_swin),gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(priv->bot_right_swin)));
		// similar connection for vertical scroll / adjustment for the left box, so that the layer info will scroll together with the name
		gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(priv->bot_left_swin),gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(priv->bot_right_swin)));

		// arbitrarily wide values.. for the scroll bar to come up automatically - need to be corrected
		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_evb),width,height-15);
		// need to be varied accoring to the allocation width
		gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_vp),width,height);
		// autmatic scroll bars
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->bot_right_swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		align = gtk_alignment_new(0,0,1,1);
		gtk_container_add(GTK_CONTAINER(align),priv->bot_right_swin);
		gtk_table_attach(GTK_TABLE(priv->main_table),GTK_WIDGET(align),1,2,1,2,GTK_FILL,GTK_FILL,0,0);
	}


		g_signal_connect(priv->bot_right_evb, "button_release_event", G_CALLBACK(bot_right_button_release_event), priv);
		g_signal_connect(priv->bot_right_evb, "button_press_event", G_CALLBACK(bot_right_button_press_event), priv);
		g_signal_connect(priv->bot_right_evb, "motion_notify_event", G_CALLBACK(bot_right_motion_notify_event), priv);

		// Ensure we get the signals we want
		gtk_widget_set_events(priv->bot_right_evb, gtk_widget_get_events(priv->bot_right_evb)
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_BUTTON1_MOTION_MASK
			| GDK_KEY_RELEASE_MASK);

		g_signal_connect(priv->top_right_evb, "button_release_event", G_CALLBACK(top_right_button_release_event), priv);
		g_signal_connect(priv->top_right_evb, "button_press_event", G_CALLBACK(top_right_button_press_event), priv);
		g_signal_connect(priv->top_right_evb, "motion_notify_event", G_CALLBACK(top_right_motion_notify_event), priv);

		// Ensure we get the signals we want
		gtk_widget_set_events(priv->top_right_evb, gtk_widget_get_events(priv->top_right_evb)
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_BUTTON1_MOTION_MASK
			| GDK_KEY_RELEASE_MASK);


		g_signal_connect(priv->top_left_evb, "button_release_event", G_CALLBACK(top_left_button_release_event), priv);
		g_signal_connect(priv->top_left_evb, "button_press_event", G_CALLBACK(top_left_button_press_event), priv);
		g_signal_connect(priv->top_left_evb, "motion_notify_event", G_CALLBACK(top_left_motion_notify_event), priv);

		// Ensure we get the signals we want
		gtk_widget_set_events(priv->top_left_evb, gtk_widget_get_events(priv->top_left_evb)
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_BUTTON1_MOTION_MASK
			| GDK_KEY_RELEASE_MASK);

		g_signal_connect(priv->bot_left_evb, "button_release_event", G_CALLBACK(bot_left_button_release_event), priv);
		g_signal_connect(priv->bot_left_evb, "button_press_event", G_CALLBACK(bot_left_button_press_event), priv);
		g_signal_connect(priv->bot_left_evb, "motion_notify_event", G_CALLBACK(bot_left_motion_notify_event), priv);

		// Ensure we get the signals we want
		gtk_widget_set_events(priv->bot_left_evb, gtk_widget_get_events(priv->bot_left_evb)
			| GDK_LEAVE_NOTIFY_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK
			| GDK_BUTTON1_MOTION_MASK
			| GDK_KEY_RELEASE_MASK);


		g_signal_connect_after(GTK_WIDGET(priv->main_table), "realize", G_CALLBACK(realize_allocate_table),priv);
		g_signal_connect_after(GTK_WIDGET(priv->main_table), "expose-event", G_CALLBACK(expose_table),priv);
		g_signal_connect(priv->main_table, "size-allocate", G_CALLBACK(size_allocate_table), priv);
		// connecting the inner expose signals
		g_signal_connect(GTK_WIDGET(priv->top_left_evb), "expose-event", G_CALLBACK(expose_event_top_left),priv);
		g_signal_connect(GTK_WIDGET(priv->top_right_evb), "expose-event", G_CALLBACK(expose_event_top_right),priv);
		g_signal_connect(GTK_WIDGET(priv->bot_right_evb), "expose-event", G_CALLBACK(expose_event_bot_right),priv);
		g_signal_connect(GTK_WIDGET(priv->bot_left_evb), "expose-event", G_CALLBACK(expose_event_bot_left),priv);



return TRUE;
}




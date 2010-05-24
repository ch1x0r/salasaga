/*
 * $Id: time_line_internal_make_widgets.c  by allupaku $
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


gboolean time_line_internal_make_widgets(TimeLinePrivate *priv){

	GdkColor firstColColor;
	GdkColor entireRowColor;
	GtkWidget * trial_label;
		gdk_color_parse ("red", &firstColColor);
		gdk_color_parse ("white", &entireRowColor);

	if(NULL == priv->main_table){
		priv->main_table = gtk_table_new(2,2,FALSE);
		gtk_table_set_row_spacings(GTK_TABLE(priv->main_table),0);
		gtk_table_set_col_spacings(GTK_TABLE(priv->main_table),0);
		if(NULL == priv->top_left_swin)
		{
			priv->top_left_swin = gtk_scrolled_window_new(NULL,NULL);
			gtk_container_set_border_width(GTK_CONTAINER(priv->top_left_swin),0);
			gtk_scrolled_window_set_shadow_type(priv->top_left_swin,GTK_SHADOW_NONE);
			if(NULL == priv->top_left_vp){
				priv->top_left_vp = gtk_viewport_new(NULL,NULL);
				gtk_container_add(GTK_CONTAINER(priv->top_left_swin),GTK_WIDGET(priv->top_left_vp));
				gtk_viewport_set_shadow_type(priv->top_left_vp,GTK_SHADOW_NONE);
			}
			if(NULL == priv->top_left_evb)
			{
				priv->top_left_evb = gtk_event_box_new();
				gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_evb),120,20);
				gtk_container_add(GTK_CONTAINER(priv->top_left_vp),GTK_WIDGET(priv->top_left_evb));
			}

			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->top_left_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			gtk_table_attach_defaults(GTK_TABLE(priv->main_table),GTK_WIDGET(priv->top_left_swin),0,1,0,1);
		}
		if(NULL == priv->top_right_swin)
		{
			priv->top_right_swin = gtk_scrolled_window_new(NULL,NULL);
			gtk_container_set_border_width(GTK_CONTAINER(priv->top_right_swin),0);
			gtk_scrolled_window_set_shadow_type(priv->top_right_swin,GTK_SHADOW_NONE);
			if(NULL == priv->top_right_vp){
				priv->top_right_vp = gtk_viewport_new(NULL,NULL);
				gtk_viewport_set_shadow_type(priv->top_right_vp,GTK_SHADOW_NONE);
				gtk_container_add(GTK_CONTAINER(priv->top_right_swin),GTK_WIDGET(priv->top_right_vp));
			}
			if(NULL == priv->top_right_evb)
			{
				priv->top_right_evb = gtk_event_box_new();
				trial_label = gtk_label_new("Hello This is a trial to know the scroll");
				gtk_container_add(GTK_CONTAINER(priv->top_right_evb),GTK_WIDGET(trial_label));
				gtk_container_add(GTK_CONTAINER(priv->top_right_vp),GTK_WIDGET(priv->top_right_evb));

			}
			gtk_widget_set_size_request(GTK_WIDGET(priv->top_right_evb),500,20);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->top_right_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			gtk_table_attach_defaults(GTK_TABLE(priv->main_table),GTK_WIDGET(priv->top_right_swin),1,2,0,1);
		}

		if(NULL == priv->bot_left_swin)
		{
			priv->bot_left_swin = gtk_scrolled_window_new(NULL,NULL);
			gtk_container_set_border_width(GTK_CONTAINER(priv->bot_left_swin),0);
			gtk_scrolled_window_set_shadow_type(priv->bot_left_swin,GTK_SHADOW_NONE);
			if(NULL == priv->bot_left_vp){
				priv->bot_left_vp = gtk_viewport_new(NULL,NULL);
				gtk_viewport_set_shadow_type(priv->bot_left_vp,GTK_SHADOW_NONE);
				gtk_container_add(GTK_CONTAINER(priv->bot_left_swin),GTK_WIDGET(priv->bot_left_vp));
			}
			if(NULL == priv->bot_left_evb)
			{
				priv->bot_left_evb = gtk_event_box_new();
				gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_evb),120,20);
				gtk_container_add(GTK_CONTAINER(priv->bot_left_vp),GTK_WIDGET(priv->bot_left_evb));
			}
			trial_label = gtk_label_new("Hello This is a trial to know the Vertical Scroll");
			gtk_container_add(GTK_CONTAINER(priv->bot_left_evb),GTK_WIDGET(trial_label));
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->bot_left_swin), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
			gtk_table_attach_defaults(GTK_TABLE(priv->main_table),GTK_WIDGET(priv->bot_left_swin),0,1,1,2);
		}
		if(NULL == priv->bot_right_swin)
		{
			priv->bot_right_swin = gtk_scrolled_window_new(NULL,NULL);
			gtk_container_set_border_width(GTK_CONTAINER(priv->bot_right_swin),0);
			gtk_scrolled_window_set_shadow_type(priv->bot_right_swin,GTK_SHADOW_NONE);
			if(NULL == priv->bot_right_vp){
				priv->bot_right_vp = gtk_viewport_new(NULL,NULL);
				gtk_viewport_set_shadow_type(priv->bot_right_vp,GTK_SHADOW_NONE);
				gtk_container_add(GTK_CONTAINER(priv->bot_right_swin),GTK_WIDGET(priv->bot_right_vp));

			}
			if(NULL == priv->bot_right_evb)
			{
				priv->bot_right_evb = gtk_event_box_new();

				gtk_container_add(GTK_CONTAINER(priv->bot_right_vp),GTK_WIDGET(priv->bot_right_evb));

			}

			gtk_scrolled_window_set_hadjustment(priv->top_right_swin,gtk_scrolled_window_get_hadjustment(priv->bot_right_swin));
			gtk_scrolled_window_set_vadjustment(priv->bot_left_swin,gtk_scrolled_window_get_vadjustment(priv->bot_right_swin));

			gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_evb),1000,400);
			gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_vp),700,150);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(priv->bot_right_swin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
			gtk_table_attach_defaults(GTK_TABLE(priv->main_table),GTK_WIDGET(priv->bot_right_swin),1,2,1,2);
		}

		gtk_widget_modify_bg(GTK_WIDGET(priv->bot_right_evb),GTK_STATE_NORMAL,&entireRowColor);
		gtk_widget_modify_bg(GTK_WIDGET(priv->top_left_evb),GTK_STATE_NORMAL,&entireRowColor);
		gtk_widget_modify_bg(GTK_WIDGET(priv->top_right_evb),GTK_STATE_NORMAL,&firstColColor);
		gtk_widget_modify_bg(GTK_WIDGET(priv->bot_left_evb),GTK_STATE_NORMAL,&firstColColor);


	}
	else
		return TRUE;
return TRUE;
}




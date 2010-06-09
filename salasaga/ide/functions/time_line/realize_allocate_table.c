/*
 * realize_allocate_table.c
 *
 *  Created on: Jun 9, 2010
 *      Author: althaf
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
#include "../dialog/display_warning.h"
#include "time_line.h"
#include "time_line_get_type.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_draw_selection_highlight.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"
#include "time_line_internal_make_widgets.h"
#include "time_line_get_left_border_width.h"
#include "time_line_get_cursor_position.h"
#include "time_line_internal_draw_cursor.h"
#include "../working_area/draw_workspace.h"
#include "draw_timeline.h"

gboolean realize_allocate_table(GtkWidget *widget,gpointer user_data)
{

	TimeLinePrivate		*priv;
	gint width;
	priv = (TimeLinePrivate *)user_data;
	if(widget->allocation.width > priv->left_border_width ){
	priv->main_width = widget->allocation.width - priv->left_border_width;
	priv->main_height = widget->allocation.height - priv->top_border_height;

	width = ((get_current_slide_duration()+1) * time_line_get_pixels_per_second());
	//height = (get_current_slide_num_layers()*priv->row_height)+10;
	if(width<priv->main_width)
		width = priv->main_width;

	gtk_widget_set_size_request(GTK_WIDGET(priv->top_left_vp),priv->left_border_width,priv->top_border_height);
	gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_vp),priv->left_border_width,priv->main_height);
	gtk_widget_set_size_request(GTK_WIDGET(priv->top_right_vp),priv->main_width,priv->top_border_height);
	gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_vp),priv->main_width,priv->main_height);

	gtk_widget_set_size_request(GTK_WIDGET(priv->top_right_evb),((get_current_slide_duration()+1) * time_line_get_pixels_per_second()),priv->top_border_height);
	gtk_widget_set_size_request(GTK_WIDGET(priv->bot_right_evb),((get_current_slide_duration()+1) * time_line_get_pixels_per_second()),(get_current_slide_num_layers()*priv->row_height)+22);
	gtk_widget_set_size_request(GTK_WIDGET(priv->bot_left_evb),priv->left_border_width,(get_current_slide_num_layers()*priv->row_height)+22);

	time_line_internal_initialise_bg_image(priv,((get_current_slide_duration()+1) * time_line_get_pixels_per_second()),(get_current_slide_num_layers()*priv->row_height)+22);
	time_line_internal_initialise_display_buffer(priv, ((get_current_slide_duration()+1) * time_line_get_pixels_per_second()),(get_current_slide_num_layers()*priv->row_height)+22);
	time_line_internal_draw_layer_info(priv);
	time_line_internal_draw_selection_highlight(priv, width);
	}


	return TRUE;
}

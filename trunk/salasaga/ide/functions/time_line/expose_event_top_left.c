/*
 * expose_event_top_left.c
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
#include "time_line.h"


gboolean expose_event_top_left(GtkWidget *widget,GdkEventExpose *event, gpointer user){

	static GdkGC		*this_gc = NULL;

	TimeLinePrivate		*priv;
	priv = (TimeLinePrivate *)user;
	if(priv->top_left_evb->window == NULL || priv->display_buffer_top_left == NULL){

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



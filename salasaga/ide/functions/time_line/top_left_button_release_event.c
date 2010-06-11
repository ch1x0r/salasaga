/*
 * top_left_button_release_event.c
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

#include <math.h>
#include <gtk/gtk.h>
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "time_line.h"
#include "draw_timeline.h"
#include "../dialog/display_warning.h"
#include "time_line_internal_draw_layer_info.h"
#include "time_line_internal_initialise_bg_image.h"
#include "time_line_internal_initialise_display_buffer.h"
void top_left_button_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

	// Local variables
		GdkModifierType		button_state;				// Mouse button states
		GtkAllocation		area;						// Area covered by an individual guide line
		GString				*message;					// Used to construct message strings
		gint				mouse_x;					// Mouse x position
		gint				mouse_y;					// Mouse x position
		TimeLinePrivate		*priv;
		gboolean			return_code_gbool;			// Receives boolean return codes

		priv = data;

		if (NULL == widget)
		{
			return;
		}

		// Check for primary mouse button
		if (1 != event->button)
		{
			// Not a primary mouse, so we return
			return;
		}
		if (TRUE == priv->cursor_drag_active)
		{
			// Note that the cursor drag has finished
			priv->cursor_drag_active = FALSE;
			return;
		}
		if(TRUE == priv->left_resize_active){
			priv->left_resize_active = FALSE;
			return;
		}
		message = g_string_new(NULL);
		gdk_window_get_pointer(event->window, &mouse_x, &mouse_y, &button_state);

		if ((ADJUSTMENTS_Y <= mouse_y) && (ADJUSTMENTS_Y + ADJUSTMENTS_SIZE) >= mouse_y)
			{
				// * It's in the correct range *

				// Check if this button release is for the minus button
				if (((ADJUSTMENTS_X <= mouse_x) && ((ADJUSTMENTS_X + ADJUSTMENTS_SIZE) >= mouse_x)) || ((ADJUSTMENTS_X + 15 <= mouse_x) && ((ADJUSTMENTS_X + 15 + ADJUSTMENTS_SIZE) >= mouse_x)))
				{
					// Sanity check
					if((ADJUSTMENTS_X <= mouse_x) && ((ADJUSTMENTS_X + ADJUSTMENTS_SIZE) >= mouse_x)){
					if (time_line_get_pixels_per_second() >= 96)
					{
						// We're already at the acceptable scaling limit, so beep then return
						gdk_beep();
						return;
					}

					// Adjust the number of pixels per second
					time_line_set_pixels_per_second(time_line_get_pixels_per_second() * 2);
					}
					else{
						if (time_line_get_pixels_per_second() <= 24)
						{
							// We're already at the acceptable scaling limit, so beep then return
							gdk_beep();

							return;
						}

					// Adjust the number of pixels per second
					time_line_set_pixels_per_second(time_line_get_pixels_per_second() / 2);

					}
					g_object_unref(GDK_PIXMAP(priv->cached_bg_image_top_left));
					priv->cached_bg_image_top_left = NULL;
					g_object_unref(GDK_PIXMAP(priv->cached_bg_image_top_right));
					priv->cached_bg_image_top_right = NULL;
					g_object_unref(GDK_PIXMAP(priv->cached_bg_image_bot_left));
					priv->cached_bg_image_bot_left = NULL;
					g_object_unref(GDK_PIXMAP(priv->cached_bg_image_bot_right));
					priv->cached_bg_image_bot_right = NULL;

					// Regenerate the timeline images with the new pixel scale
					return_code_gbool = time_line_internal_initialise_bg_image(priv, priv->main_table->allocation.width, priv->main_table->allocation.height);

					if (FALSE == return_code_gbool)
					{
						g_string_printf(message, "%s ED361: %s", _("Error"), _("Couldn't recreate time line background image."));
						display_warning(message->str);
						g_string_free(message, TRUE);
						return;
					}
					return_code_gbool = time_line_internal_initialise_display_buffer(priv, priv->main_table->allocation.width, priv->main_table->allocation.height);
					if (FALSE == return_code_gbool)
					{
						g_string_printf(message, "%s ED362: %s", _("Error"), _("Couldn't recreate time line display buffer."));
						display_warning(message->str);
						g_string_free(message, TRUE);

						return;
					}
					return_code_gbool = time_line_internal_draw_layer_info(priv);
					if (FALSE == return_code_gbool)
					{
						g_string_printf(message, "%s ED363: %s", _("Error"), _("Couldn't redraw the time line layer information."));
						display_warning(message->str);
						g_string_free(message, TRUE);

						return;
					}
					draw_timeline();
					area.x = 0;
					area.y = 0;
					area.width = priv->top_right_evb->allocation.width;
					area.height = priv->top_right_evb->allocation.height;
					gdk_window_invalidate_rect(GTK_WIDGET(priv->top_right_evb)->window, &area, TRUE);
					area.width = priv->bot_right_evb->allocation.width;
					area.height = priv->bot_right_evb->allocation.height;
					gdk_window_invalidate_rect(GTK_WIDGET(priv->bot_right_evb)->window, &area, TRUE);


				}
			}

			// Remove guide lines from the widget
			area.x = priv->guide_line_start;
			area.y = 0;
			area.height = GTK_WIDGET(priv->bot_right_evb)->allocation.height;
			area.width = 1;
			gdk_window_invalidate_rect(GTK_WIDGET(priv->bot_right_evb)->window, &area, TRUE);
			area.x = priv->guide_line_end;
			gdk_window_invalidate_rect(GTK_WIDGET(priv->bot_right_evb)->window, &area, TRUE);
}


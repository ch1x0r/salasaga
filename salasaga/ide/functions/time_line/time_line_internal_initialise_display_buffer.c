/*
 * $Id$
 *
 * Salasaga: Create the display buffer
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
#include "../dialog/display_warning.h"
#include "time_line.h"
#include "time_line_internal_redraw_bg_area.h"


gboolean time_line_internal_initialise_display_buffer(TimeLinePrivate *priv, gint new_width, gint new_height)
{
	// Local variables
	static GdkColormap	*colourmap = NULL;			// Colormap used for drawing
	gint				existing_bg_height;			// Height in pixels of an existing pixmap
	gint				existing_bg_width;			// Width in pixels of an existing pixmap
	gint				height;
	GString				*message;					// Used to construct message strings
	gint				width;

	gint 				main_part_width;
	gint 				main_part_height;



	// Initialisation
	if (NULL == colourmap)
	{
		colourmap = gdk_colormap_get_system();
	}

	// Ensure we have at least the minimum required widget size
	if (WIDGET_MINIMUM_HEIGHT > new_height)
	{
		height = WIDGET_MINIMUM_HEIGHT;
	} else
	{
		height = new_height;
	}
	if (WIDGET_MINIMUM_WIDTH > new_width)
	{
		width = WIDGET_MINIMUM_WIDTH;
	} else
	{
		width = new_width;
	}
	main_part_width = width;// - priv->left_border_width;
	main_part_height = height - priv->top_border_height;

	// If we already have a display buffer, we check if we can reuse it
	if (NULL != priv->display_buffer_top_left)
	{
		// Retrieve the size of the existing cached display buffer
		gdk_drawable_get_size(GDK_PIXMAP(priv->display_buffer_top_left), &existing_bg_width, &existing_bg_height);

		// If the existing display buffer is not of the same height and width, we discard it
		if ((existing_bg_width != priv->left_border_width) || (existing_bg_height != priv->top_border_height))
		{
			// The existing display buffer is not of the same height and width
			g_object_unref(GDK_PIXMAP(priv->display_buffer_top_left));
			priv->display_buffer_top_left = gdk_pixmap_new(NULL, priv->left_border_width, priv->top_border_height, colourmap->visual->depth);
			if (NULL == priv->cached_bg_image_top_left)
			{
				// Couldn't allocate memory for a new display buffer
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED360: %s", _("Error"), _("Couldn't create the time line display buffer image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}
		}
	} else
	{
		// Create the display buffer
		priv->display_buffer_top_left = gdk_pixmap_new(NULL, priv->left_border_width, priv->top_border_height, colourmap->visual->depth);
		if (NULL == priv->cached_bg_image_top_left)
		{
			// Couldn't allocate memory for a new display buffer
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED357: %s", _("Error"), _("Couldn't create the time line display buffer image."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer_top_left), GDK_COLORMAP(colourmap));


	// If we already have a display buffer, we check if we can reuse it
	if (NULL != priv->display_buffer_top_right)
	{
		// Retrieve the size of the existing cached display buffer
		gdk_drawable_get_size(GDK_PIXMAP(priv->display_buffer_top_right), &existing_bg_width, &existing_bg_height);

		// If the existing display buffer is not of the same height and width, we discard it
		if ((existing_bg_width != main_part_width) || (existing_bg_height != priv->top_border_height))
		{
			// The existing display buffer is not of the same height and width
			g_object_unref(GDK_PIXMAP(priv->display_buffer_top_right));
			priv->display_buffer_top_right = gdk_pixmap_new(NULL, main_part_width, priv->top_border_height, colourmap->visual->depth);
			if (NULL == priv->display_buffer_top_right)
			{
				// Couldn't allocate memory for a new display buffer
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED360: %s", _("Error"), _("Couldn't create the time line display buffer image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}
		}
	} else
	{
		// Create the display buffer
		priv->display_buffer_top_right = gdk_pixmap_new(NULL, main_part_width, priv->top_border_height, colourmap->visual->depth);
		if (NULL == priv->display_buffer_top_right)
		{
			// Couldn't allocate memory for a new display buffer
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED357: %s", _("Error"), _("Couldn't create the time line display buffer image."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer_top_right), GDK_COLORMAP(colourmap));



	// If we already have a display buffer, we check if we can reuse it
	if (NULL != priv->display_buffer_bot_right)
	{
		// Retrieve the size of the existing cached display buffer
		gdk_drawable_get_size(GDK_PIXMAP(priv->display_buffer_bot_right), &existing_bg_width, &existing_bg_height);

		// If the existing display buffer is not of the same height and width, we discard it
		if ((existing_bg_width != main_part_width) || (existing_bg_height != main_part_height))
		{
			// The existing display buffer is not of the same height and width
			g_object_unref(GDK_PIXMAP(priv->display_buffer_bot_right));
			priv->display_buffer_bot_right = gdk_pixmap_new(NULL, main_part_width,main_part_height, colourmap->visual->depth);
			if (NULL == priv->display_buffer_bot_right)
			{
				// Couldn't allocate memory for a new display buffer
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED360: %s", _("Error"), _("Couldn't create the time line display buffer image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}
		}
	} else
	{
		// Create the display buffer
		priv->display_buffer_bot_right = gdk_pixmap_new(NULL, main_part_width, main_part_height, colourmap->visual->depth);
		if (NULL == priv->display_buffer_bot_right)
		{
			// Couldn't allocate memory for a new display buffer
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED357: %s", _("Error"), _("Couldn't create the time line display buffer image."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer_bot_right), GDK_COLORMAP(colourmap));


// If we already have a display buffer, we check if we can reuse it
	if (NULL != priv->display_buffer_bot_left)
	{
		// Retrieve the size of the existing cached display buffer
		gdk_drawable_get_size(GDK_PIXMAP(priv->display_buffer_bot_left), &existing_bg_width, &existing_bg_height);

		// If the existing display buffer is not of the same height and width, we discard it
		if ((existing_bg_width != priv->left_border_width) || (existing_bg_height != main_part_height))
		{
			// The existing display buffer is not of the same height and width
			g_object_unref(GDK_PIXMAP(priv->display_buffer_bot_left));
			priv->display_buffer_bot_left = gdk_pixmap_new(NULL, priv->left_border_width,main_part_height, colourmap->visual->depth);
			if (NULL == priv->display_buffer_bot_left)
			{
				// Couldn't allocate memory for a new display buffer
				message = g_string_new(NULL);
				g_string_printf(message, "%s ED360: %s", _("Error"), _("Couldn't create the time line display buffer image."));
				display_warning(message->str);
				g_string_free(message, TRUE);
				return FALSE;
			}
		}
	} else
	{
		// Create the display buffer
		priv->display_buffer_bot_left = gdk_pixmap_new(NULL, priv->left_border_width, main_part_height, colourmap->visual->depth);
		if (NULL == priv->display_buffer_bot_left)
		{
			// Couldn't allocate memory for a new display buffer
			message = g_string_new(NULL);
			g_string_printf(message, "%s ED357: %s", _("Error"), _("Couldn't create the time line display buffer image."));
			display_warning(message->str);
			g_string_free(message, TRUE);
			return FALSE;
		}
	}
	gdk_drawable_set_colormap(GDK_DRAWABLE(priv->display_buffer_bot_left), GDK_COLORMAP(colourmap));





	// Copy the timeline background image to the display buffer
	time_line_internal_redraw_bg_area(priv, 0, 0,width, height,0);


	return TRUE;
}

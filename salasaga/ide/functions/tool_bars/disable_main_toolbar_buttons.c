/*
 * $Id$
 *
 * Salasaga: Disables the main toolbar buttons that can only be used when a project is loaded
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


// GTK includes
#include <gtk/gtk.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
#endif

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"
#include "shared_toolbar_functions.h"


void disable_main_toolbar_buttons(void)
{
	// Disable the Adjust Dimensions icon
	if (NULL != get_main_toolbar_icon(DIMENSIONS))
	{
		g_object_ref(get_main_toolbar_icon(DIMENSIONS));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_main_toolbar_item(DIMENSIONS)), get_main_toolbar_icon_gray(DIMENSIONS));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(DIMENSIONS)), get_main_toolbar_tooltips(), _("Dimension adjustment disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_main_toolbar_item(DIMENSIONS)));
	}

	// Disable the Export Flash icon
	if (NULL != get_main_toolbar_icon(EXPORT_FLASH))
	{
		g_object_ref(get_main_toolbar_icon(EXPORT_FLASH));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_main_toolbar_item(EXPORT_FLASH)), get_main_toolbar_icon_gray(EXPORT_FLASH));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_main_toolbar_item(EXPORT_FLASH)), get_main_toolbar_tooltips(), _("Export to Flash disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_main_toolbar_item(EXPORT_FLASH)));
	}

	// Disconnect the main toolbar signal handlers
	if (0 != get_main_toolbar_signal(DIMENSIONS))
	{
		g_signal_handler_disconnect(G_OBJECT(get_main_toolbar_item(DIMENSIONS)), get_main_toolbar_signal(DIMENSIONS));
		g_signal_handler_disconnect(G_OBJECT(get_main_toolbar_item(EXPORT_FLASH)), get_main_toolbar_signal(EXPORT_FLASH));

		// Flag the signal handlers as unused (used by enable_layer_toolbar_buttons())
		set_main_toolbar_signal(DIMENSIONS, 0);
	}
}

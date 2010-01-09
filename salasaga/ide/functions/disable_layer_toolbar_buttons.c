/*
 * $Id$
 *
 * Salasaga: Disables the layer toolbar buttons 
 * 
 * Copyright (C) 2005-2010 Justin Clift <justin@salasaga.org>
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
#include "../salasaga_types.h"
#include "../externs.h"


void disable_layer_toolbar_buttons(void)
{
	// Disable the Edit Layer icon
	if (NULL != layer_toolbar_icons[LAYER_EDIT])
	{
		g_object_ref(layer_toolbar_icons[LAYER_EDIT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_icons_gray[LAYER_EDIT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_tooltips, _("Edit layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_EDIT]));
	}

	// Disable the Crop Layer icon
	if (NULL != layer_toolbar_icons[LAYER_CROP])
	{
		g_object_ref(layer_toolbar_icons[LAYER_CROP]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_CROP]), layer_toolbar_icons_gray[LAYER_CROP]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_CROP]), layer_toolbar_tooltips, _("Crop layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_CROP]));
	}

	// Disable the Delete Layer icon
	if (NULL != layer_toolbar_icons[LAYER_DELETE])
	{
		g_object_ref(layer_toolbar_icons[LAYER_DELETE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_icons_gray[LAYER_DELETE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_tooltips, _("Delete layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DELETE]));
	}

	// Disable the Move Layer Down icon
	if (NULL != layer_toolbar_icons[LAYER_DOWN])
	{
		g_object_ref(layer_toolbar_icons[LAYER_DOWN]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_icons_gray[LAYER_DOWN]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_tooltips, _("Move layer down disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DOWN]));
	}

	// Disable the Move Layer Up icon
	if (NULL != layer_toolbar_icons[LAYER_UP])
	{
		g_object_ref(layer_toolbar_icons[LAYER_UP]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_UP]), layer_toolbar_icons_gray[LAYER_UP]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_UP]), layer_toolbar_tooltips, _("Move layer up disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_UP]));
	}

	// Disable the Add Mouse Pointer icon
	if (NULL != layer_toolbar_icons[LAYER_MOUSE])
	{
		g_object_ref(layer_toolbar_icons[LAYER_MOUSE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_icons_gray[LAYER_MOUSE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_tooltips, _("Add mouse pointer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_MOUSE]));
	}

	// Disable the Add Text Layer icon
	if (NULL != layer_toolbar_icons[LAYER_TEXT])
	{
		g_object_ref(layer_toolbar_icons[LAYER_TEXT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_icons_gray[LAYER_TEXT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_tooltips, _("Add text layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_TEXT]));
	}

	// Disable the Add Highlight Layer icon
	if (NULL != layer_toolbar_icons[LAYER_HIGHLIGHT])
	{
		g_object_ref(layer_toolbar_icons[LAYER_HIGHLIGHT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_icons_gray[LAYER_HIGHLIGHT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_tooltips, _("Add highlight layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_HIGHLIGHT]));
	}

	// Disable the Add Image Layer icon
	if (NULL != layer_toolbar_icons[LAYER_IMAGE])
	{
		g_object_ref(layer_toolbar_icons[LAYER_IMAGE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_icons_gray[LAYER_IMAGE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_tooltips, _("Add image layer disabled: No project loaded"), "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_IMAGE]));
	}

	// Disconnect the layer toolbar signal handlers
	if (0 != layer_toolbar_signals[LAYER_EDIT])
	{
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_signals[LAYER_EDIT]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_CROP]), layer_toolbar_signals[LAYER_CROP]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_signals[LAYER_DELETE]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_signals[LAYER_DOWN]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_UP]), layer_toolbar_signals[LAYER_UP]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_signals[LAYER_MOUSE]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_signals[LAYER_TEXT]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_signals[LAYER_HIGHLIGHT]);
		g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_signals[LAYER_IMAGE]);

		// Flag the signal handlers as unused (used by enable_layer_toolbar_buttons())
		layer_toolbar_signals[LAYER_EDIT] = 0;
	}
}

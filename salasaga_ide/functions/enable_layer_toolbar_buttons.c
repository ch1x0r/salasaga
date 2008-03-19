/*
 * $Id$
 *
 * Salasaga: Enables the layer toolbar buttons 
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include "image_crop.h"
#include "layer_delete.h"
#include "layer_edit.h"
#include "layer_move_down.h"
#include "layer_move_up.h"
#include "layer_new_highlight.h"
#include "layer_new_image.h"
#include "layer_new_mouse.h"
#include "layer_new_text.h"


void enable_layer_toolbar_buttons(void)
{
	// Enable the Edit Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_EDIT])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_EDIT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_icons[LAYER_EDIT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_tooltips, "Edit layer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_EDIT]));
	}

	// Enable the Crop Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_CROP])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_CROP]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_CROP]), layer_toolbar_icons[LAYER_CROP]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_CROP]), layer_toolbar_tooltips, "Crop image", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_CROP]));
	}

	// Enable the Delete Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_DELETE])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_DELETE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_icons[LAYER_DELETE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_tooltips, "Delete selected layer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DELETE]));
	}

	// Enable the Move Layer Down icon
	if (NULL != layer_toolbar_icons_gray[LAYER_DOWN])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_DOWN]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_icons[LAYER_DOWN]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_tooltips, "Move layer down", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DOWN]));
	}

	// Enable the Move Layer Up icon
	if (NULL != layer_toolbar_icons_gray[LAYER_UP])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_UP]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_UP]), layer_toolbar_icons[LAYER_UP]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_UP]), layer_toolbar_tooltips, "Move layer up", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_UP]));
	}

	// Enable the Add Mouse Pointer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_MOUSE])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_MOUSE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_icons[LAYER_MOUSE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_tooltips, "Add a mouse pointer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_MOUSE]));
	}

	// Enable the Add Text Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_TEXT])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_TEXT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_icons[LAYER_TEXT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_tooltips, "Add a text layer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_TEXT]));
	}

	// Enable the Add Highlight Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_HIGHLIGHT])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_HIGHLIGHT]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_icons[LAYER_HIGHLIGHT]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_tooltips, "Add a highlight layer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_HIGHLIGHT]));
	}

	// Enable the Add Image Layer icon
	if (NULL != layer_toolbar_icons_gray[LAYER_IMAGE])
	{
		g_object_ref(layer_toolbar_icons_gray[LAYER_IMAGE]);
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_icons[LAYER_IMAGE]);
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_tooltips, "Add an image layer", "Private");
		gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_IMAGE]));
	}

	// Set the event handlers for the layer toolbar buttons
	if (0 == layer_toolbar_signals[LAYER_EDIT])
	{
		layer_toolbar_signals[LAYER_EDIT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_EDIT]), "clicked", G_CALLBACK(layer_edit), (gpointer) NULL);
		layer_toolbar_signals[LAYER_CROP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_CROP]), "clicked", G_CALLBACK(image_crop), (gpointer) NULL);
		layer_toolbar_signals[LAYER_DELETE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DELETE]), "clicked", G_CALLBACK(layer_delete), (gpointer) NULL);
		layer_toolbar_signals[LAYER_DOWN] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DOWN]), "clicked", G_CALLBACK(layer_move_down), (gpointer) NULL);
		layer_toolbar_signals[LAYER_UP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_UP]), "clicked", G_CALLBACK(layer_move_up), (gpointer) NULL);
		layer_toolbar_signals[LAYER_MOUSE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_MOUSE]), "clicked", G_CALLBACK(layer_new_mouse), (gpointer) NULL);
		layer_toolbar_signals[LAYER_TEXT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_TEXT]), "clicked", G_CALLBACK(layer_new_text), (gpointer) NULL);
		layer_toolbar_signals[LAYER_HIGHLIGHT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_HIGHLIGHT]), "clicked", G_CALLBACK(layer_new_highlight), (gpointer) NULL);
		layer_toolbar_signals[LAYER_IMAGE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_IMAGE]), "clicked", G_CALLBACK(layer_new_image), (gpointer) NULL);	
	}
}

/*
 * $Id$
 *
 * Salasaga: Enables the layer toolbar buttons 
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
#include "../../salasaga_types.h"
#include "../../externs.h"
#include "../layer/image_crop.h"
#include "../layer/layer_delete.h"
#include "../layer/layer_edit.h"
#include "../layer/layer_move_down.h"
#include "../layer/layer_move_up.h"
#include "../layer/layer_new_highlight.h"
#include "../layer/layer_new_image.h"
#include "../layer/layer_new_mouse.h"
#include "../layer/layer_new_text.h"
#include "shared_toolbar_functions.h"


void enable_layer_toolbar_buttons(void)
{
	// Enable the Edit Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_EDIT))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_EDIT));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_EDIT)), get_layer_toolbar_icon(LAYER_EDIT));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_EDIT)), get_layer_toolbar_tooltips(), _("Edit layer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_EDIT)));
	}

	// Enable the Crop Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_CROP))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_CROP));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_CROP)), get_layer_toolbar_icon(LAYER_CROP));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_CROP)), get_layer_toolbar_tooltips(), _("Crop image"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_CROP)));
	}

	// Enable the Delete Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_DELETE))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_DELETE));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_DELETE)), get_layer_toolbar_icon(LAYER_DELETE));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_DELETE)), get_layer_toolbar_tooltips(), _("Delete selected layer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_DELETE)));
	}

	// Enable the Move Layer Down icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_DOWN))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_DOWN));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_DOWN)), get_layer_toolbar_icon(LAYER_DOWN));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_DOWN)), get_layer_toolbar_tooltips(), _("Move layer down"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_DOWN)));
	}

	// Enable the Move Layer Up icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_UP))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_UP));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_UP)), get_layer_toolbar_icon(LAYER_UP));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_UP)), get_layer_toolbar_tooltips(), _("Move layer up"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_UP)));
	}

	// Enable the Add Mouse Pointer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_MOUSE))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_MOUSE));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_MOUSE)), get_layer_toolbar_icon(LAYER_MOUSE));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_MOUSE)), get_layer_toolbar_tooltips(), _("Add a mouse pointer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_MOUSE)));
	}

	// Enable the Add Text Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_TEXT))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_TEXT));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_TEXT)), get_layer_toolbar_icon(LAYER_TEXT));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_TEXT)), get_layer_toolbar_tooltips(), _("Add a text layer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_TEXT)));
	}

	// Enable the Add Highlight Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_HIGHLIGHT))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_HIGHLIGHT));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_HIGHLIGHT)), get_layer_toolbar_icon(LAYER_HIGHLIGHT));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_HIGHLIGHT)), get_layer_toolbar_tooltips(), _("Add a highlight layer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_HIGHLIGHT)));
	}

	// Enable the Add Image Layer icon
	if (NULL != get_layer_toolbar_icon_gray(LAYER_IMAGE))
	{
		g_object_ref(get_layer_toolbar_icon_gray(LAYER_IMAGE));
		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(get_layer_toolbar_item(LAYER_IMAGE)), get_layer_toolbar_icon(LAYER_IMAGE));
		gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_IMAGE)), get_layer_toolbar_tooltips(), _("Add an image layer"), "Private");
		gtk_widget_show_all(GTK_WIDGET(get_layer_toolbar_item(LAYER_IMAGE)));
	}

	// Set the event handlers for the layer toolbar buttons
	if (0 == get_layer_toolbar_signal(LAYER_EDIT))
	{
		set_layer_toolbar_signal(LAYER_EDIT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_EDIT)), "clicked", G_CALLBACK(layer_edit), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_CROP, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_CROP)), "clicked", G_CALLBACK(image_crop), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_DELETE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_DELETE)), "clicked", G_CALLBACK(layer_delete), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_DOWN, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_DOWN)), "clicked", G_CALLBACK(layer_move_down), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_UP, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_UP)), "clicked", G_CALLBACK(layer_move_up), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_MOUSE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_MOUSE)), "clicked", G_CALLBACK(layer_new_mouse), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_TEXT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_TEXT)), "clicked", G_CALLBACK(layer_new_text), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_HIGHLIGHT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_HIGHLIGHT)), "clicked", G_CALLBACK(layer_new_highlight), (gpointer) NULL));
		set_layer_toolbar_signal(LAYER_IMAGE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_IMAGE)), "clicked", G_CALLBACK(layer_new_image), (gpointer) NULL));
	}
}

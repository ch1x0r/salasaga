/*
 * $Id$
 *
 * Salasaga: Function to create the time line area
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
#include "../call_back/delete_key_release_event.h"
#include "../layer/image_crop.h"
#include "../layer/layer_delete.h"
#include "../layer/layer_edit.h"
#include "../layer/layer_move_down.h"
#include "../layer/layer_move_up.h"
#include "../layer/layer_new_highlight.h"
#include "../layer/layer_new_image.h"
#include "../layer/layer_new_mouse.h"
#include "../layer/layer_new_text.h"
#include "../preference/application_preferences.h"
#include "../time_line/time_line_button_press_event.h"
#include "../time_line/time_line_button_release_event.h"
#include "../time_line/time_line_motion_notify_event.h"
#include "../tool_bars/shared_toolbar_functions.h"


GtkWidget *create_time_line(void)
{
	// Purpose: To create the time line area
	//
	// Inputs: none
	//
	// Returns: A pointer to the time line GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		create_time_line();
	//

	// Local variables
	GtkWidget			*time_line_toolbar;			// Widget for holding the time line toolbar
	GtkScrolledWindow	*time_line_scrolled_window;	// Widget for holding the scrolled window
	GdkPixbuf			*tmp_gdk_pixbuf;			// Temporary GDK Pixbuf
	GString				*tmp_gstring;				// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// Create the VBox the time line elements are packed into
	set_time_line_vbox(gtk_vbox_new(FALSE, 0));

	// Create the scrolled window that the time line widgets are swapped into and out of
	time_line_scrolled_window = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(time_line_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(get_time_line_vbox()), GTK_WIDGET(time_line_scrolled_window), TRUE, TRUE, 0);

	// Create the time line container contents
	set_time_line_container(gtk_viewport_new(NULL, NULL));
	gtk_widget_set_can_focus(GTK_WIDGET(get_time_line_container()), TRUE);
	gtk_container_add(GTK_CONTAINER(time_line_scrolled_window), GTK_WIDGET(get_time_line_container()));

	// Add signal handlers to the time line area for receiving events (i.e. mouse clicks)
	g_signal_connect(get_time_line_container(), "button_release_event", G_CALLBACK(time_line_button_release_event), NULL);
	g_signal_connect(get_time_line_container(), "button_press_event", G_CALLBACK(time_line_button_press_event), NULL);
	g_signal_connect(get_time_line_container(), "motion_notify_event", G_CALLBACK(time_line_motion_notify_event), NULL);

	// Add a signal handler to the time line, to be called whenever a key is pressed while it is in focus
	g_signal_connect(get_time_line_container(), "key-release-event", G_CALLBACK(delete_key_release_event), NULL);

	// Ensure we get the signals we want
	gtk_widget_set_events(get_time_line_container(), gtk_widget_get_events(get_time_line_container())
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_BUTTON1_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK
		| GDK_KEY_RELEASE_MASK);

	// Create the time line toolbar
	time_line_toolbar = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(get_time_line_vbox()), GTK_WIDGET(time_line_toolbar), FALSE, FALSE, 0);

	// Create the tooltips structure
	set_layer_toolbar_tooltips(gtk_tooltips_new());
	gtk_tooltips_enable(GTK_TOOLTIPS(get_layer_toolbar_tooltips()));

	// * Create the layer toolbar icons *

	// Create the Edit Layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "edit", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_EDIT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_EDIT, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_EDIT)), _("Edit")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_EDIT)), get_layer_toolbar_tooltips(), _("Edit layer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_EDIT), LAYER_EDIT);
	set_layer_toolbar_signal(LAYER_EDIT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_EDIT)), "clicked", G_CALLBACK(layer_edit), (gpointer) NULL));

	// Create the Crop button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_CROP, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_CROP, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_CROP)), _("Crop")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_CROP)), get_layer_toolbar_tooltips(), _("Crop image"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_CROP), LAYER_CROP);
	set_layer_toolbar_signal(LAYER_CROP, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_CROP)), "clicked", G_CALLBACK(image_crop), (gpointer) NULL));

	// Create the Delete layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "delete", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_DELETE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_DELETE, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_DELETE)), _("Delete")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_DELETE)), get_layer_toolbar_tooltips(), _("Delete selected layer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_DELETE), LAYER_DELETE);
	set_layer_toolbar_signal(LAYER_DELETE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_DELETE)), "clicked", G_CALLBACK(layer_delete), (gpointer) NULL));

	// Create the Move Layer Down button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "down_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_DOWN, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_DOWN, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_DOWN)), _("Down")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_DOWN)), get_layer_toolbar_tooltips(), _("Move layer down"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_DOWN), LAYER_DOWN);
	set_layer_toolbar_signal(LAYER_DOWN, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_DOWN)), "clicked", G_CALLBACK(layer_move_down), (gpointer) NULL));

	// Create the Move Layer Up button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "up_arrow", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_UP, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_UP, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_UP)), _("Up")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_UP)), get_layer_toolbar_tooltips(), _("Move layer up"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_UP), LAYER_UP);
	set_layer_toolbar_signal(LAYER_UP, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_UP)), "clicked", G_CALLBACK(layer_move_up), (gpointer) NULL));

	// Add a spacer to the toolbar
	set_layer_toolbar_item(LAYER_SEPARATOR_1, gtk_separator_tool_item_new());
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_SEPARATOR_1), LAYER_SEPARATOR_1);

	// Create the add mouse pointer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_mouse", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_MOUSE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_MOUSE, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_MOUSE)), _("Mouse")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_MOUSE)), get_layer_toolbar_tooltips(), _("Add a mouse pointer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_MOUSE), LAYER_MOUSE);
	set_layer_toolbar_signal(LAYER_MOUSE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_MOUSE)), "clicked", G_CALLBACK(layer_new_mouse), (gpointer) NULL));

	// Create the add text layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_text", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_TEXT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_TEXT, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_TEXT)), _("Text")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_TEXT)), get_layer_toolbar_tooltips(), _("Add a text layer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_TEXT), LAYER_TEXT);
	set_layer_toolbar_signal(LAYER_TEXT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_TEXT)), "clicked", G_CALLBACK(layer_new_text), (gpointer) NULL));

	// Create the add highlight layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_highlight", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_HIGHLIGHT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_HIGHLIGHT, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_HIGHLIGHT)), _("Highlight")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_HIGHLIGHT)), get_layer_toolbar_tooltips(), _("Add a highlight layer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_HIGHLIGHT), LAYER_HIGHLIGHT);
	set_layer_toolbar_signal(LAYER_HIGHLIGHT, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_HIGHLIGHT)), "clicked", G_CALLBACK(layer_new_highlight), (gpointer) NULL));

	// Create the add image layer button
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_image", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon(LAYER_IMAGE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}
	set_layer_toolbar_item(LAYER_IMAGE, gtk_tool_button_new(GTK_WIDGET(get_layer_toolbar_icon(LAYER_IMAGE)), _("Image")));
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(get_layer_toolbar_item(LAYER_IMAGE)), get_layer_toolbar_tooltips(), _("Add an image layer"), "Private");
	gtk_toolbar_insert(GTK_TOOLBAR(time_line_toolbar), get_layer_toolbar_item(LAYER_IMAGE), LAYER_IMAGE);
	set_layer_toolbar_signal(LAYER_IMAGE, g_signal_connect(G_OBJECT(get_layer_toolbar_item(LAYER_IMAGE)), "clicked", G_CALLBACK(layer_new_image), (gpointer) NULL));

	// * Create the "grayed out" icons for buttons *

	// Create the grayed out Edit Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "edit_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_EDIT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Crop Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "crop_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_CROP, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Delete Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "delete_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_DELETE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Move Layer Down icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "down_arrow_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_DOWN, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Move Layer Up icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "up_arrow_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_UP, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Add Mouse Pointer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_mouse_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_MOUSE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Add Text Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_text_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_TEXT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Add Highlight Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_highlight_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_HIGHLIGHT, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Create the grayed out Add Image Layer icon
	g_string_printf(tmp_gstring, "%s%c%s.%s", icon_path->str, G_DIR_SEPARATOR, "add_image_grayed", icon_extension->str);
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_gstring->str, -1, get_icon_height(), NULL);
	if (NULL != tmp_gdk_pixbuf)
	{
		set_layer_toolbar_icon_gray(LAYER_IMAGE, gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));
		g_object_unref(GDK_PIXBUF(tmp_gdk_pixbuf));
	}

	// Free the memory allocated during this function
	g_string_free(tmp_gstring, TRUE);

	// Return the handle of the time line container
	return get_time_line_vbox();
}

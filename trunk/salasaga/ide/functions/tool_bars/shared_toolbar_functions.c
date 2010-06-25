/*
 * $Id$
 *
 * Salasaga: Function to set and retrieve shared tool bar related variables
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

// Salasaga includes
#include "../../salasaga_types.h"
#include "../global_functions.h"


// Main tool bar items
static GtkTooltips			*main_toolbar_tooltips;			// Tooltips structure
static GtkWidget			*main_toolbar_icons[MAIN_TB_COUNT];  // Array of toolbar icons
static GtkWidget			*main_toolbar_icons_gray[MAIN_TB_COUNT];  // Array of toolbar icons (the grayed out ones)
static GtkToolItem			*main_toolbar_items[MAIN_TB_COUNT];  // Array of toolbar items
static gulong				main_toolbar_signals[MAIN_TB_COUNT]; // Array of toolbar signals

// Layer tool bar items
static GtkTooltips			*layer_toolbar_tooltips;		// Tooltips structure
static GtkWidget			*layer_toolbar_icons[MAIN_TB_COUNT];  // Array of toolbar icons
static GtkWidget			*layer_toolbar_icons_gray[MAIN_TB_COUNT];  // Array of toolbar icons (the grayed out ones)
static GtkToolItem			*layer_toolbar_items[MAIN_TB_COUNT];  // Array of toolbar items
static gulong				layer_toolbar_signals[MAIN_TB_COUNT];  // Array of toolbar signals


// Functions to get and set main tool bar variables
GtkTooltips *get_main_toolbar_tooltips()
{
	return main_toolbar_tooltips;
}

GtkWidget *get_main_toolbar_icon(guint index)
{
	return main_toolbar_icons[index];
}

GtkWidget *get_main_toolbar_icon_gray(guint index)
{
	return main_toolbar_icons_gray[index];
}

GtkToolItem *get_main_toolbar_item(guint index)
{
	return main_toolbar_items[index];
}

gulong get_main_toolbar_signal(guint index)
{
	return main_toolbar_signals[index];
}

void set_main_toolbar_tooltips(GtkTooltips *new_main_toolbar_tooltips)
{
	main_toolbar_tooltips = new_main_toolbar_tooltips;
}

void set_main_toolbar_icon(guint index, GtkWidget *new_icon)
{
	main_toolbar_icons[index] = new_icon;
}

void set_main_toolbar_icon_gray(guint index, GtkWidget *new_icon)
{
	main_toolbar_icons_gray[index] = new_icon;
}

void set_main_toolbar_item(guint index, GtkToolItem *new_item)
{
	main_toolbar_items[index] = new_item;
}

void set_main_toolbar_signal(guint index, gulong new_signal)
{
	main_toolbar_signals[index] = new_signal;
}

// Functions to get and set layer tool bar variables
GtkTooltips *get_layer_toolbar_tooltips()
{
	return layer_toolbar_tooltips;
}

GtkWidget *get_layer_toolbar_icon(guint index)
{
	return layer_toolbar_icons[index];
}

GtkWidget *get_layer_toolbar_icon_gray(guint index)
{
	return layer_toolbar_icons_gray[index];
}

GtkToolItem *get_layer_toolbar_item(guint index)
{
	return layer_toolbar_items[index];
}

gulong get_layer_toolbar_signal(guint index)
{
	return layer_toolbar_signals[index];
}

void set_layer_toolbar_tooltips(GtkTooltips *new_layer_toolbar_tooltips)
{
	layer_toolbar_tooltips = new_layer_toolbar_tooltips;
}

void set_layer_toolbar_icon(guint index, GtkWidget *new_icon)
{
	layer_toolbar_icons[index] = new_icon;
}

void set_layer_toolbar_icon_gray(guint index, GtkWidget *new_icon)
{
	layer_toolbar_icons_gray[index] = new_icon;
}

void set_layer_toolbar_item(guint index, GtkToolItem *new_item)
{
	layer_toolbar_items[index] = new_item;
}

void set_layer_toolbar_signal(guint index, gulong new_signal)
{
	layer_toolbar_signals[index] = new_signal;
}

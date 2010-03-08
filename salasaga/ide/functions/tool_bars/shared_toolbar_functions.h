/*
 * $Id$
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

#ifndef SHARED_TOOLBAR_FUNCTIONS_
#define SHARED_TOOLBAR_FUNCTIONS_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


GtkTooltips *get_main_toolbar_tooltips();
GtkWidget *get_main_toolbar_icon(guint index);
GtkWidget *get_main_toolbar_icon_gray(guint index);
GtkToolItem *get_main_toolbar_item(guint index);
gulong get_main_toolbar_signal(guint index);
void set_main_toolbar_tooltips(GtkTooltips *new_main_toolbar_tooltips);
void set_main_toolbar_icon(guint index, GtkWidget *new_icon);
void set_main_toolbar_icon_gray(guint index, GtkWidget *new_icon);
void set_main_toolbar_item(guint index, GtkToolItem *new_item);
void set_main_toolbar_signal(guint index, gulong new_signal);
GtkTooltips *get_layer_toolbar_tooltips();
GtkWidget *get_layer_toolbar_icon(guint index);
GtkWidget *get_layer_toolbar_icon_gray(guint index);
GtkToolItem *get_layer_toolbar_item(guint index);
gulong get_layer_toolbar_signal(guint index);
void set_layer_toolbar_tooltips(GtkTooltips *new_layer_toolbar_tooltips);
void set_layer_toolbar_icon(guint index, GtkWidget *new_icon);
void set_layer_toolbar_icon_gray(guint index, GtkWidget *new_icon);
void set_layer_toolbar_item(guint index, GtkToolItem *new_item);
void set_layer_toolbar_signal(guint index, gulong new_signal);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* SHARED_TOOLBAR_FUNCTIONS_ */

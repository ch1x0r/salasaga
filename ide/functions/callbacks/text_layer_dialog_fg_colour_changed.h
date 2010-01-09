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

#ifndef TEXT_LAYER_DIALOG_FG_COLOUR_CHANGED_H_
#define TEXT_LAYER_DIALOG_FG_COLOUR_CHANGED_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gboolean text_layer_dialog_fg_colour_changed(GtkWidget *calling_widget, text_dialog_widgets *text_widgets);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TEXT_LAYER_DIALOG_FG_COLOUR_CHANGED_H_ */

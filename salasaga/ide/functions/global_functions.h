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

#ifndef GLOBAL_FUNCTIONS_H_
#define GLOBAL_FUNCTIONS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GList *get_boundary_list();
cairo_font_face_t *get_cairo_font_face(guint index);
guint get_capture_height();
guint get_capture_width();
guint get_capture_x();
guint get_capture_y();
gboolean get_changes_made();
layer *get_copy_layer();
void set_boundary_list(GList *new_boundary_list);
void set_cairo_font_face(guint index, cairo_font_face_t *new_cairo_font_face);
void set_capture_height(guint new_capture_height);
void set_capture_width(guint new_capture_width);
void set_capture_x(guint new_capture_x);
void set_capture_y(guint new_capture_y);
void set_changes_made(gboolean new_changes_made);
void set_copy_layer(layer *new_copy_layer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* GLOBAL_FUNCTIONS_H_ */

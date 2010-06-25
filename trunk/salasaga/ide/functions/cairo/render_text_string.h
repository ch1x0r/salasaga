/*
 * $Id$
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

#ifndef RENDER_TEXT_STRING_H_
#define RENDER_TEXT_STRING_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int render_text_string(cairo_t *existing_cairo_context, layer_text *text_object, gfloat scaled_width_ratio, gfloat scaled_height_ratio, gdouble incoming_cairo_pos_x, gdouble incoming_cairo_pos_y, gfloat time_alpha, gboolean display_onscreen);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* RENDER_TEXT_STRING_H_ */

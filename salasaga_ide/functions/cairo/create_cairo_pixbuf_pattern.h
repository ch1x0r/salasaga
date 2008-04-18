/*
 * $Id$
 *
 * Copyright (C) 2005-2008 Justin Clift <justin@salasaga.org>
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

#ifndef CREATE_CAIRO_PIXBUF_PATTERN_H_
#define CREATE_CAIRO_PIXBUF_PATTERN_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

cairo_pattern_t *create_cairo_pixbuf_pattern(GdkPixbuf *source_pixbuf);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*CREATE_CAIRO_PIXBUF_PATTERN_H_*/

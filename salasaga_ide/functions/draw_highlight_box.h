/*
 * $Id$
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

#ifndef DRAW_HIGHLIGHT_BOX_H_
#define DRAW_HIGHLIGHT_BOX_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*DRAW_HIGHLIGHT_BOX_H_*/

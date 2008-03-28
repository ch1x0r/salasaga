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

#ifndef CREATE_TIMELINE_SLIDER_H_
#define CREATE_TIMELINE_SLIDER_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GdkPixbuf *create_timeline_slider(GdkPixbuf *output_pixbuf, gint total_width, gint total_height, gint start_pixel, gint trans_in_pixel, gint trans_out_pixel, gint finish_pixel);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*CREATE_TIMELINE_SLIDER_H_*/

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

#ifndef TIME_LINE_INTERNAL_REDRAW_BG_AREA_H_
#define TIME_LINE_INTERNAL_REDRAW_BG_AREA_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gboolean time_line_internal_redraw_bg_area(TimeLinePrivate *priv, gint x1, gint y1, gint width, gint height,guint portion);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TIME_LINE_INTERNAL_REDRAW_BG_AREA_H_ */

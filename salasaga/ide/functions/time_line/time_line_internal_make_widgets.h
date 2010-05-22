/*
 * $Id: time_line_internal_make_widgets.h 2610 2010-03-23 05:48:58Z allupaku $
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

#ifndef TIME_LINE_INTERNAL_MAKE_WIDGETS_
#define TIME_LINE_INTERNAL_MAKE_WIDGETS_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
gboolean scroll_win(GtkAdjustment *adjustment,gpointer user_data);
gboolean time_line_internal_make_widgets(TimeLinePrivate *priv);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TIME_LINE_INTERNAL_REDRAW_BG_AREA_H_ */

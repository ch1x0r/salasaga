/*
 * expose_event_top_right.h
 *
 *  Created on: Jun 9, 2010
 *      Author: althaf
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

#ifndef EXPOSE_EVENT_TOP_RIGHT_H_
#define EXPOSE_EVENT_TOP_RIGHT_H_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gboolean expose_event_top_right(GtkWidget *widget,GdkEventExpose *event, gpointer user);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif /* EXPOSE_EVENT_TOP_RIGHT_H_ */

/*
 * bot_left_button_press_event.h
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

#ifndef BOT_LEFT_BUTTON_PRESS_EVENT_H_
#define BOT_LEFT_BUTTON_PRESS_EVENT_H_
// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


void bot_left_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif /* BOT_LEFT_BUTTON_PRESS_EVENT_H_ */

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

#ifndef UNDO_FUNCTIONS_H_
#define UNDO_FUNCTIONS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


gint undo_history_add_item(gint undo_type, gpointer undo_data, gboolean remove_new);
gint undo_history_clear(void);
gint undo_history_redo_item(void);
gint undo_history_undo_item(void);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* UNDO_FUNCTIONS_H_ */

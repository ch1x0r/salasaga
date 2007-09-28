/*
 * $Id$
 *
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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

#ifndef TIMELINE_EDITED_Y_OFFSET_FINISH_H_
#define TIMELINE_EDITED_Y_OFFSET_FINISH_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void timeline_edited_y_offset_finish(GtkCellRendererText *selection, gchar *row, gchar *new_value, gpointer data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*TIMELINE_EDITED_Y_OFFSET_FINISH_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/09/28 12:05:07  vapour
 * Broke callbacks.c and callbacks.h into its component functions.
 *
 */

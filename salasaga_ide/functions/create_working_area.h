/*
 * $Id$
 *
 * Copyright (C) 2007-2008 Justin Clift <justin@postgresql.org>
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

#ifndef CREATE_WORKING_AREA_H_
#define CREATE_WORKING_AREA_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GtkWidget *create_working_area(GtkWidget *working_frame);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*CREATE_WORKING_AREA_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/01/15 16:19:07  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/29 02:42:26  vapour
 * Broke flame-edit.c into its component functions.
 *
 */

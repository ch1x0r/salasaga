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

#ifndef MENU_EXPORT_FLASH_INNER_H_
#define MENU_EXPORT_FLASH_INNER_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

gint menu_export_flash_inner(gchar *output_filename);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*MENU_EXPORT_FLASH_INNER_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.5  2008/01/19 06:49:38  vapour
 * Updated function definition with new input parameter and return type.
 *
 * Revision 1.4  2008/01/15 16:18:59  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.3  2008/01/13 10:26:58  vapour
 * Updated to use Ming (0.4.0 beta 5).
 *
 * Revision 1.2  2007/10/07 14:15:23  vapour
 * Moved initial allocation of swf buffer into the inner function.
 *
 * Revision 1.1  2007/09/27 10:40:43  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

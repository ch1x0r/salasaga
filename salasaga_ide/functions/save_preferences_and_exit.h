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

#ifndef SAVE_PREFERENCES_AND_EXIT_H_
#define SAVE_PREFERENCES_AND_EXIT_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void save_preferences_and_exit(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*SAVE_PREFERENCES_AND_EXIT_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/01/15 16:19:00  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.1  2007/09/27 10:40:52  vapour
 * Broke backend.c and backend.h into its component functions.
 *
 */

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

#ifndef MENU_EXPORT_FLASH_CONTROL_BAR_
#define MENU_EXPORT_FLASH_CONTROL_BAR_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int menu_export_flash_control_bar(SWFMovie main_movie, gfloat height_scale_factor, gfloat width_scale_factor);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*MENU_EXPORT_FLASH_CONTROL_BAR_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/02/03 00:51:04  vapour
 * Fixed a small bug. Updated the symbol defined by this header to match the function name, rather than still use the name from the template.
 *
 * Revision 1.1  2008/01/23 02:09:09  vapour
 * Added new function with initial working code to output a swf control bar.  Only has Stop and Play buttons at the moment (and they work), but it's a start.
 *
 */

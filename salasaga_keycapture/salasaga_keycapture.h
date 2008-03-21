/*
 * $Id$
 *
 * Salasaga: Header file for the Control-Printscreen key capturing dll
 * 
 * Copyright (C) 2007 Justin Clift <justin@salasaga.org>
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

#ifdef _WIN32

#ifdef BUILD_DLL
// the dll exports
#define EXPORT __declspec(dllexport)
#else
// the exe imports
#define EXPORT __declspec(dllimport)
#endif


// Windows-only function to be imported/exported
EXPORT LRESULT win32_keyboard_press_hook(int hook_code, WPARAM message_id, LPARAM message_data_ptr);
#endif

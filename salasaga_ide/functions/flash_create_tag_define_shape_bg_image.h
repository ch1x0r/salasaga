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

#ifndef FLASH_CREATE_TAG_DEFINE_SHAPE_H_
#define FLASH_CREATE_TAG_DEFINE_SHAPE_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GByteArray *flash_create_tag_define_shape_bg_image(guint16 bitmap_id, guint16 character_id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*FLASH_CREATE_TAG_DEFINE_SHAPE_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/18 11:28:52  vapour
 * Updated variable definition and declaration for the flash_create_tag_define_shape_bg_image function.
 * Updated code calling it to also pass the bitmap id.
 * Added initial untested code to include fill style information.
 * Still needs work and testing.
 *
 * Revision 1.1  2007/10/17 11:04:07  vapour
 * Adding initial code to define shapes to place the background images on.  Still needs work.
 *
 */

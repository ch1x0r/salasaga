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

#ifndef FLASH_CREATE_TAG_HIGHLIGHT_H_
#define FLASH_CREATE_TAG_HIGHLIGHT_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

GByteArray *flash_create_tag_highlight(layer_image *layer_data);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*FLASH_CREATE_TAG_HIGHLIGHT_H_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2007/10/04 11:50:27  vapour
 * Added initial (non working) stub functions for the creation of swf byte code for hightlight, mouse cursor, and text layers.
 *
 */

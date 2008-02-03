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

#ifndef SWF_SHAPE_FROM_IMAGE_FILE_
#define SWF_SHAPE_FROM_IMAGE_FILE_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

SWFShape swf_shape_from_image_file(gchar *filename, gint width, gint height);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /*SWF_SHAPE_FROM_IMAGE_FILE_*/


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2008/02/03 00:51:58  vapour
 * Fixed a small bug. Updated the symbol defined by this header to match the function name, rather than still use the name from the template.
 *
 * Revision 1.1  2008/01/23 17:20:34  vapour
 * Added a new function, for loading a SWF Shape bitmap directly from a given filename.
 *
 */

/*
 * $Id$
 *
 * Salasaga: Include file for structures and similar used throughout the code
 * 
 * Copyright (C) 2008 Justin Clift <justin@salasaga.org>
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

#ifndef __SALASAGA_TYPES_H__
#define __SALASAGA_TYPES_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Define a run time check that hasn't been added to GTK
#define GTK_TYPE_TREE_ITER		(gtk_tree_iter_get_type ())
#define GTK_TREE_ITER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TREE_ITER, GtkTreeIter))

// * Define values used in the application *

// Base types for the validate_value function
#define V_CHAR				1
#define V_FLOAT_UNSIGNED	2
#define V_INT_UNSIGNED		4
#define V_RESOLUTION		8
#define V_ZOOM				16

// Capabilities for the validate_value function
#define V_NONE				0
#define V_EQUALS			1
#define V_FORWARD_SLASHES	2
#define V_FULL_STOP			4
#define V_HYPENS			8
#define V_NEW_LINES			16
#define V_PATH_SEP			32
#define V_PERCENT			64
#define V_PLUSES			128
#define V_SPACES			256
#define V_UNDERSCORES		512

// Field IDs for the validate_value function
enum
{
	FOLDER_PATH,
	PROJECT_NAME,
	SCREENSHOT_HEIGHT,
	SCREENSHOT_WIDTH,
	SCREENSHOT_X_OFFSET,
	SCREENSHOT_Y_OFFSET,
};

// * Define the structures used in the application *

// Defines the structure of a validation field entry
typedef struct
{
	guint				value_id;
	gchar				*name_string;
	guint				base_type;
	guint				capabilities;
	guint				min_value;
	gint				max_value;
} validatation_entry;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SALASAGA_TYPES_H__

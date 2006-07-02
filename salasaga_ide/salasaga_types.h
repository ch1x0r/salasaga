/*
 * $Id$
 *
 * Flame Project: Include file for structures and similar used throughout the code
 * 
 * Copyright (C) 2006 Justin Clift <justin@postgresql.org>
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

#ifndef __FLAME_TYPES_H__
#define __FLAME_TYPES_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Set some application constants
#define APP_NAME "Flame Project Editing GUI"
#define	APP_VERSION "0.7.5"

// Define a run time check that hasn't been added to GTK
#define GTK_TYPE_TREE_ITER				(gtk_tree_iter_get_type ())
#define GTK_TREE_ITER(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TREE_ITER, GtkTreeIter))


// * Define the structures used in the application *
// fixme4: These structures need better descriptions

// The known object types
enum {
	TYPE_GDK_PIXBUF,
	TYPE_MOUSE_CURSOR,
	TYPE_EMPTY,
	TYPE_TEXT,
	TYPE_HIGHLIGHT
};

// The order of the main toolbar buttons
enum
{
	NEW,
	OPEN,
	SAVE,
	QUIT,
	SEPARATOR_1,
	CAPTURE,
	IMPORT,
	CROP_ALL,
	SEPARATOR_2,
	EXPORT_FLASH,
	EXPORT_SVG,
	TB_COUNT
};

// The order of the layer toolbar buttons
enum
{
	EDIT,
	CROP,
	DELETE,
	DOWN,
	UP,
	LAYER_SEPARATOR_1,
	MOUSE,
	TEXT,
	HIGHLIGHT,
	IMAGE
};

// Fields of the timeline widget
enum
{
	TIMELINE_NAME,
	TIMELINE_VISIBILITY,
	TIMELINE_START,
	TIMELINE_FINISH,
	TIMELINE_X_OFF_START,
	TIMELINE_Y_OFF_START,
	TIMELINE_X_OFF_FINISH,
	TIMELINE_Y_OFF_FINISH,
	TIMELINE_N_COLUMNS
};

// Defines the structure of a boundary box
typedef struct
{
	GdkRegion			*region_ptr;
	GList				*layer_ptr;
} boundary_box;

// Defines the object and properties making up a layer
typedef struct
{
	guint				object_type;
	GObject				*object_data;
	guint				start_frame;
	guint				finish_frame;
	GString				*name;
	GtkTreeIter			*row_iter;
} layer;

// Defines the properties making up an empty layer
typedef struct
{
	GdkColor			bg_color;
} layer_empty;

// Defines the properties making up a highlight layer
typedef struct
{
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	gint				width;
	gint				height;
} layer_highlight;

// Defines the properties making up an image layer
typedef struct
{
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	gint				width;
	gint				height;
	GString				*image_path;
	GdkPixbuf			*image_data;
	gboolean			modified;			// FALSE if an image hasn't been modified, TRUE if it has (i.e. cropped)
} layer_image;

// Defines the properties making up a text layer
typedef struct
{
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	guint				rendered_width;
	guint				rendered_height;
	GdkColor			text_color;
	gfloat				font_size;
	GtkTextBuffer		*text_buffer;
} layer_text;

// Resolution storing structure
typedef struct {
	guint				width;
	guint				height;
} ResolutionStructure;

// Defines the collection of objects and properties making up a slide
typedef struct
{
	GtkWidget			*event_box;
	GtkImage			*thumbnail;
	gulong				click_handler;
	GList				*layers;
	GtkTooltips			*tooltip;
	GString				*name;
	GtkWidget			*timeline_widget;
	GtkListStore		*layer_store;
} slide;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FLAME_TYPES_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.16  2006/07/02 08:50:16  vapour
 * + Simplified the enum definitions.
 * + Added enum for the order of the layer toolbar icons.
 *
 * Revision 1.15  2006/06/27 13:38:12  vapour
 * Added a new enum for the rewritten main toolbar functions.
 *
 * Revision 1.14  2006/06/08 12:17:10  vapour
 * Replaced slide number with slide name.
 *
 * Revision 1.13  2006/06/08 11:35:59  vapour
 * Visually lined up variables names for my Linux system.
 *
 * Revision 1.12  2006/05/28 09:36:44  vapour
 * Re-tab aligned some variables for my Linux Eclipse.
 *
 * Revision 1.11  2006/05/23 13:08:32  vapour
 * Added a new ResolutionStructure data type.
 *
 * Revision 1.10  2006/05/17 11:24:11  vapour
 * Incremented the app version number to 0.7.4.
 *
 * Revision 1.9  2006/04/26 18:38:44  vapour
 * Incremented the version number to 0.7.3 to mark (celebrate!) getting the file loading code working. :)
 *
 * Revision 1.8  2006/04/26 18:31:30  vapour
 * Removed the layer type from the layer treeview list store enum.
 *
 * Revision 1.7  2006/04/25 10:54:22  vapour
 * Definition for image layer now includes a flag to track if it has been modified.
 *
 * Revision 1.6  2006/04/22 08:36:54  vapour
 * + Replaced the text string display in the timeline (layer) widget area, with the x and y finish positions.
 * + Updated the entire project to use the word "finish" consistently, instead of "final".
 *
 * Revision 1.5  2006/04/21 17:48:23  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.4  2006/04/20 12:03:50  vapour
 * Change to 3 part version numbers, as thats what people are used to.
 *
 * Revision 1.3  2006/04/18 18:02:00  vapour
 * Bumped the version number to 0.72, to mark the successful compilation on both Windows and Solaris as well. :)
 *
 * Revision 1.2  2006/04/16 06:01:42  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

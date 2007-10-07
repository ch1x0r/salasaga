/*
 * $Id$
 *
 * Flame Project: Include file for structures and similar used throughout the code
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

#ifndef __FLAME_TYPES_H__
#define __FLAME_TYPES_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Set some application constants
#define APP_NAME "Flame Project Editing GUI"
#define	APP_VERSION "0.7.8-pre"

// Define a run time check that hasn't been added to GTK
#define GTK_TYPE_TREE_ITER		(gtk_tree_iter_get_type ())
#define GTK_TREE_ITER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TREE_ITER, GtkTreeIter))


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
	MAIN_TB_COUNT
};

// The order of the layer toolbar buttons
enum
{
	LAYER_EDIT,
	LAYER_CROP,
	LAYER_DELETE,
	LAYER_DOWN,
	LAYER_UP,
	LAYER_SEPARATOR_1,
	LAYER_MOUSE,
	LAYER_TEXT,
	LAYER_HIGHLIGHT,
	LAYER_IMAGE,
	LAYER_TB_COUNT
};

// Types of mouse click
enum
{
	MOUSE_NONE,
	MOUSE_LEFT_ONE,
	MOUSE_LEFT_DOUBLE,
	MOUSE_LEFT_TRIPLE,
	MOUSE_RIGHT_ONE,
	MOUSE_RIGHT_DOUBLE,
	MOUSE_RIGHT_TRIPLE,
	MOUSE_MIDDLE_ONE,
	MOUSE_MIDDLE_DOUBLE,
	MOUSE_MIDDLE_TRIPLE
};

// Fields of the timeline widget
enum
{
	TIMELINE_NAME,
	TIMELINE_VISIBILITY,
	TIMELINE_DURATION,
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
	GString				*external_link;
	// fixme5: Noticing that we don't seem to have a visibility on/off toggle.  Will likely need to add that back in
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
	gboolean			modified;				// FALSE if an image hasn't been modified, TRUE if it has (i.e. cropped)
} layer_image;

// Defines the properties making up a mouse pointer layer
typedef struct
{
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	gint				width;
	gint				height;
	gint				click;
	GString				*image_path;
} layer_mouse;

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
	GtkImage			*thumbnail;
	GList				*layers;
	GtkTooltips			*tooltip;
	GString				*name;
	GtkWidget			*timeline_widget;
	GtkListStore		*layer_store;
	guint				duration;
} slide;

// Defines the information needed for each element of the swf timing array
typedef struct
{
	gboolean			action_this;			// Process this element?
	guint				layer_type;				// The type of element being displayed
	GObject				*layer_data;			// Pointer to the layer data
	guint				char_id;				// Unique id of the character
	gboolean			add;					// Add to the display list in this frame?
	gboolean			remove;					// Remove from the display list in this frame?
	gint				opacity;				// Opacity level (0-65535)
	gint				x_position;				// In twips
	gint				y_position;				// In twips
	gint				depth;					// Depth in the display list (lowest is 1)
} swf_frame_element;

// Tag types present in swf files
#define SWF_TAG_DEFINE_SHAPE_3				32

#define SWF_TAG_PLACE_OBJECT_2				26
#define SWF_TAG_REMOVE_OBJECT_2				28
#define SWF_TAG_SHOW_FRAME					1

#define SWF_TAG_SET_BACKGROUND_COLOUR		9
#define SWF_TAG_FRAME_LABEL					43
#define SWF_TAG_PROTECT						24
#define SWF_TAG_MOVIE_END					0
#define SWF_TAG_EXPORT_ASSETS				56
#define SWF_TAG_IMPORT_ASSETS				57
#define SWF_TAG_ENABLE_DEBUGGER_2			64
#define SWF_TAG_SCRIPT_LIMITS				65
#define SWF_TAG_SET_TAB_INDEX				66
#define SWF_TAG_DO_ACTION					12

// Action types present in swf files
#define SWF_ACTION_GOTO_FRAME				0x81
#define SWF_ACTION_NEXT_FRAME				0x04
#define SWF_ACTION_PREVIOUS_FRAME			0x05
#define SWF_ACTION_PLAY						0x06
#define SWF_ACTION_STOP						0x07
#define SWF_ACTION_TOGGLE_QUALITY			0x08
#define SWF_ACTION_STOP_SOUNDS				0x09
#define SWF_ACTION_WAIT_FOR_FRAME			0x8A
#define SWF_ACTION_SET_TARGET				0x8B
#define SWF_ACTION_GO_TO_LABEL				0x8C

// Swf bitmap tags
#define SWF_TAG_DEFINE_BITS_JPEG2			21
#define SWF_TAG_DEFINE_BITS_JPEG3			35
#define SWF_TAG_DEFINE_BITS_LOSSLESS		20
#define SWF_TAG_DEFINE_BITS_LOSSLESS2		36

// Swf PLACE OBJECT 2 bit values
#define SWF_BIT_VALUE_PLACE_HAS_CLP_ACT		128
#define SWF_BIT_VALUE_PLACE_HAS_CLP_DEP		64
#define SWF_BIT_VALUE_PLACE_HAS_NAME		32
#define SWF_BIT_VALUE_PLACE_HAS_RATIO		16
#define SWF_BIT_VALUE_PLACE_HAS_COLOR		8
#define SWF_BIT_VALUE_PLACE_HAS_MATRIX		4
#define SWF_BIT_VALUE_PLACE_HAS_CHAR		2
#define SWF_BIT_VALUE_PLACE_MOVE			1


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FLAME_TYPES_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.37  2007/10/07 06:43:21  vapour
 * Added a depth field to the swf element array, and improved one of the swf tag name defines.
 *
 * Revision 1.36  2007/10/01 13:16:35  vapour
 * Added some initial swf tags useful for defining bitmaps.
 *
 * Revision 1.35  2007/10/01 12:17:52  vapour
 * Added comment about not having visibiliy toggle any more and perhaps needing it.
 *
 * Revision 1.34  2007/10/01 11:06:50  vapour
 * Updated version number to 0.7.8-pre.
 *
 * Revision 1.33  2007/09/24 12:28:40  vapour
 * Updated version number to 0.7.7.12, to match the windows release.
 *
 * Revision 1.32  2007/09/19 13:27:31  vapour
 * Added a build date stamp to the Help -> About box.
 *
 * Revision 1.31  2007/09/18 02:53:42  vapour
 * Updated copyright year to 2007.
 *
 * Revision 1.30  2007/07/29 06:11:02  vapour
 * Began modifying timeline widget to use a pixbuf for display of time duration.
 *
 * Revision 1.29  2007/07/29 05:22:20  vapour
 * Began modifying timeline widget to use a pixbuf for display of time duration.
 *
 * Revision 1.28  2007/06/30 03:18:18  vapour
 * Began re-writing the film strip area to use a GtkListView widget instead of the hodge podge of event boxes, signal handlers, and other bits.
 *
 * Revision 1.27  2007/06/24 13:21:47  vapour
 * Neatened up, prior to work on it.
 *
 * Revision 1.26  2006/12/31 00:19:47  vapour
 * Updated internal version number to 0.7.7-dev.
 *
 * Revision 1.25  2006/09/21 13:21:30  vapour
 * Added the DEFINE_SHAPE_3 swf tag and further expanded the swf_frame_element structure.
 *
 * Revision 1.24  2006/09/20 12:35:03  vapour
 * Expanding the swf_frame_element definition, so the swf output function can access the raw data.
 *
 * Revision 1.23  2006/09/13 11:45:41  vapour
 * Added an initial stub type to assist with coordinating the info needed in a frame.
 *
 * Revision 1.22  2006/09/10 09:24:27  vapour
 * Started adding defines for the tag and action types in swf.
 *
 * Revision 1.21  2006/08/09 06:08:35  vapour
 * Added an external link attribute to layers.
 *
 * Revision 1.20  2006/07/30 05:23:11  vapour
 * Bumped version number up to 0.7.6, to mark that svg exporting is now Ok. :)
 *
 * Revision 1.19  2006/07/05 12:17:42  vapour
 * + Added mouse click types enum.
 * + Added mouse click field to mouse layer.
 *
 * Revision 1.18  2006/07/04 12:30:49  vapour
 * + Started adding types for the mouse pointer layer.
 * + Re-enabled the mouse pointer button on the toolbar.
 *
 * Revision 1.17  2006/07/02 09:26:37  vapour
 * Commented out the add mouse pointer icon definition.
 *
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

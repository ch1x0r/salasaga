/*
 * $Id$
 *
 * Flame Project: Include file for structures and similar used throughout the code
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

#ifndef __FLAME_TYPES_H__
#define __FLAME_TYPES_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Define a run time check that hasn't been added to GTK
#define GTK_TYPE_TREE_ITER		(gtk_tree_iter_get_type ())
#define GTK_TREE_ITER(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TREE_ITER, GtkTreeIter))

// * Define values used in the application *

// Basic application constants
#define APP_NAME "Flame Project Editing GUI"
#define	APP_VERSION "0.7.8-pre"
	
// Base types for the validate_value function
#define V_CHAR			1
#define V_INT_UNSIGNED	2
#define V_RESOLUTION	4
#define V_ZOOM			8

// Capabilities for the validate_value function
#define V_NONE			0
#define V_FULL_STOP		1
#define V_HYPENS		2
#define V_UNDERSCORES	4
#define V_PATH_SEP		8
#define V_SPACES		16
#define V_PERCENT		32

// The known layer types
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

// Field IDs for the validate_value function
enum
{
	COLOUR_COMP8,
	COLOUR_COMP16,
	EXTERNAL_LINK,
	EXTERNAL_LINK_WINDOW,
	FILE_PATH,
	FOLDER_PATH,
	FONT_SIZE,
	FRAME_NUMBER,
	ICON_HEIGHT,
	LAYER_HEIGHT,
	LAYER_NAME,
	LAYER_WIDTH,
	PREVIEW_WIDTH,
	PROJECT_FPS,
	PROJECT_HEIGHT,
	PROJECT_NAME,
	PROJECT_WIDTH,
	RESOLUTION,
	SCREENSHOT_HEIGHT,
	SCREENSHOT_WIDTH,
	SCREENSHOT_X_OFFSET,
	SCREENSHOT_Y_OFFSET,
	SLIDE_LENGTH,
	SLIDE_NAME,
	X_OFFSET,
	Y_OFFSET,
	ZOOM_LEVEL
};

// * Define the structures used in the application *

// Defines the structure of a boundary box
typedef struct
{
	GdkRegion			*region_ptr;
	GList				*layer_ptr;
} boundary_box;

// Control bar element structure
typedef struct
{
	guint				swf_width;
	guint				swf_height;
	gfloat				button_height;
	gfloat				button_spacing;
	gfloat				button_start_x;
	gfloat				button_start_y;
	gfloat				button_width;
	gfloat				cb_start_x;
	gfloat				cb_start_y;
	gfloat				cb_height;
	gfloat				cb_width;
} control_bar_elements;

// Defines the object and properties making up a layer
typedef struct
{
	guint				object_type;
	GObject				*object_data;
	guint				start_frame;
	guint				finish_frame;
	GString				*name;
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	gboolean			visible;					// Is this layer visible or not?
	GtkTreeIter			*row_iter;
	GString				*external_link;
	GString				*external_link_window;		// Name of the target window to load the external link in. Defaults to _self for swf.
	void				*dictionary_shape;			// SWF dictionary shape
	void				*display_list_item;			// SWF display list item
	gboolean			background;					// Is this the background layer?
} layer;

// Defines the properties making up an empty layer
typedef struct
{
	GdkColor			bg_color;
} layer_empty;

// Defines the properties making up a highlight layer
typedef struct
{
	gint				width;
	gint				height;
} layer_highlight;

// Defines the properties making up an image layer
typedef struct
{
	gint				width;
	gint				height;
	GString				*image_path;
	GdkPixbuf			*image_data;
	gboolean			modified;					// FALSE if an image hasn't been modified, TRUE if it has (i.e. cropped)
} layer_image;

// Defines the properties making up a mouse pointer layer
typedef struct
{
	gint				width;
	gint				height;
	gint				click;
	GString				*image_path;
} layer_mouse;

// Defines the properties making up a text layer
typedef struct
{
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
} resolution_structure;

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
	gboolean			action_this;				// Process this element?
	guint				layer_type;					// The type of element being displayed
	layer				*layer_info;				// Pointer to the layer info
	gboolean			add;						// Add to the display list in this frame?
	gboolean			remove;						// Remove from the display list in this frame?
	gint				opacity;					// Opacity level (0-65535)
	gboolean			is_moving;					// Should the position of this element be adjusted in this frame?
	gfloat				x_position;					// In twips
	gfloat				y_position;					// In twips
	gint				depth;						// The layer depth of this item when displayed
} swf_frame_element;

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

#endif // __FLAME_TYPES_H__


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.61  2008/03/03 08:21:06  vapour
 * Removed the swf text element position structure, as nothing uses it any more.
 *
 * Revision 1.60  2008/03/01 01:45:01  vapour
 * Expanded the amount of information available for positioning text elements.
 *
 * Revision 1.59  2008/02/28 16:54:02  vapour
 * Added an initial structure to assist with text element positioning for swf output.
 *
 * Revision 1.58  2008/02/27 02:33:50  vapour
 * Updated swf control bar elements to use floating point numbers for finer grained positioning.
 *
 * Revision 1.57  2008/02/27 01:45:52  vapour
 * Added a structure type for control bar elements.
 *
 * Revision 1.56  2008/02/20 23:09:04  vapour
 * Removed unused validation base types.
 *
 * Revision 1.55  2008/02/20 21:58:36  vapour
 * Added font size and layer name validation fields.
 *
 * Revision 1.54  2008/02/20 18:38:51  vapour
 * Added validation field values for the external link, external link target window, and x and y offsets.
 *
 * Revision 1.53  2008/02/20 09:08:38  vapour
 * Added the slide name validation field type.
 *
 * Revision 1.52  2008/02/19 12:34:00  vapour
 * Added validation field entries for icon height, film strip width, default slide length, and default zoom level.
 *
 * Revision 1.51  2008/02/19 06:37:35  vapour
 * Added a new resolution base type for validation, renamed the screenshot folder field to a more generic folder type of field.
 *
 * Revision 1.50  2008/02/18 07:05:21  vapour
 * Added project path validation entry, and changed the max_length type to gint so that it can take -1.
 *
 * Revision 1.49  2008/02/14 13:36:07  vapour
 * Added values and structure needed to support the new validate value function.
 *
 * Revision 1.48  2008/02/12 13:49:36  vapour
 * Expanded the layer structure to have fields for visibility and whether or not a layer is a background.
 *
 * Revision 1.47  2008/02/12 05:14:38  vapour
 * Slightly simplified the layer structures, by moving the x and y offset fields from the sub structures into the main layer structure.
 *
 * Revision 1.46  2008/02/03 05:46:46  vapour
 * Removing svg export functionality.
 *
 * Revision 1.45  2008/02/03 05:09:53  vapour
 * Adjusted swf output frame elements to use floating point references instead of integer references, in order to address a stuttering-playback-of-swf-elements bug reported by Bobby Powers.
 *
 * Revision 1.44  2008/02/01 10:54:34  vapour
 * Added fields to the empty layer structure for storing x and y values for it.  As all layer types now have these same values, this should probably be moved into the parent layer structure.
 *
 * Revision 1.43  2008/02/01 05:00:17  vapour
 * Added a variable to the layer structure, to store the target window for loading the external link.
 *
 * Revision 1.42  2008/01/21 19:17:03  vapour
 * Added a depth field to the swf element array.
 *
 * Revision 1.41  2008/01/15 16:20:32  vapour
 * Updated copyright notice to include 2008.
 *
 * Revision 1.40  2008/01/13 10:52:59  vapour
 * Added swf output supporting fields to the layer structure.
 * Removed some fields not needed now that ming is being used.
 * Removed swf values as they're no longer needed.
 *
 * Revision 1.39  2007/10/21 12:18:42  vapour
 * Added some defines to support the creation of swf shapes.
 *
 * Revision 1.38  2007/10/15 10:01:44  vapour
 * Added the value for DEFINE SHAPE to the swf tag types.
 *
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

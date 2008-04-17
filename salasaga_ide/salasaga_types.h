/*
 * $Id$
 *
 * Salasaga: Include file for structures and similar used throughout the code
 * 
 * Copyright (C) 2007-2008 Justin Clift <justin@salasaga.org>
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


// * Define values used in the application *

// Basic application constants
#define APP_NAME "Salasaga"
#define	APP_VERSION "0.8.0-pre"

// Statuses for the start and end points
#define END_POINTS_INACTIVE			0
#define END_POINTS_START_ACTIVE		1
#define END_POINTS_END_ACTIVE		2

// Statuses for the bounding box resize handles
#define	RESIZE_HANDLES_INACTIVE		0
#define	RESIZE_HANDLES_WAITING		1
#define	RESIZE_HANDLES_RESIZING		2
#define	RESIZE_HANDLES_RESIZING_TL	4
#define	RESIZE_HANDLES_RESIZING_TM	8
#define	RESIZE_HANDLES_RESIZING_TR	16
#define	RESIZE_HANDLES_RESIZING_RM	32
#define	RESIZE_HANDLES_RESIZING_BR	64
#define	RESIZE_HANDLES_RESIZING_BM	128
#define	RESIZE_HANDLES_RESIZING_BL	256
#define	RESIZE_HANDLES_RESIZING_LM	512
#define RESIZE_HANDLES_RESIZING_ALL	1020

// Text padding defines (in pixels) 
#define TEXT_BORDER_PADDING_WIDTH 10
#define TEXT_BORDER_PADDING_HEIGHT 4

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

// The order of the main toolbar buttons
enum
{
	NEW,
	OPEN,
	SAVE,
	SAVE_AS,
	QUIT,
	SEPARATOR_1,
	CAPTURE,
	IMPORT,
	CROP_ALL,
	SEPARATOR_2,
	EXPORT_FLASH,
	MAIN_TB_COUNT
};

// Field IDs for the validate_value function
enum
{
	COLOUR_COMP8,
	COLOUR_COMP16,
	END_BEHAVIOUR,
	EXTERNAL_LINK,
	EXTERNAL_LINK_WINDOW,
	FILE_PATH,
	FOLDER_PATH,
	FONT_SIZE,
	FRAME_NUMBER,
	HIGHLIGHT_HEIGHT,
	HIGHLIGHT_WIDTH,
	ICON_HEIGHT,
	IMAGE_DATA,
	IMAGE_DATA_LENGTH,
	LAYER_BACKGROUND,
	LAYER_DURATION,
	LAYER_HEIGHT,
	LAYER_NAME,
	LAYER_VISIBLE,
	LAYER_WIDTH,
	MOUSE_CLICK,
	PREVIEW_WIDTH,
	PROJECT_FPS,
	PROJECT_HEIGHT,
	PROJECT_NAME,
	PROJECT_VERSION,
	PROJECT_WIDTH,
	RESOLUTION,
	SCREENSHOT_HEIGHT,
	SCREENSHOT_WIDTH,
	SHOW_CONTROL_BAR,
	SLIDE_LENGTH,
	SLIDE_DURATION,
	SLIDE_NAME,
	START_BEHAVIOUR,
	TRANSITION_DURATION,
	TRANSITION_TYPE,
	X_OFFSET,
	Y_OFFSET,
	ZOOM_LEVEL
};

// The end behaviour types
enum {
	END_BEHAVIOUR_STOP,
	END_BEHAVIOUR_LOOP_PLAY,
	END_BEHAVIOUR_LOOP_STOP
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

// The start behaviour types
enum {
	START_BEHAVIOUR_PAUSED,
	START_BEHAVIOUR_PLAY
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

// The layer transition types
enum {
	TRANS_LAYER_NONE,
	TRANS_LAYER_FADE
};

// The known layer types
enum {
	TYPE_NONE,
	TYPE_EMPTY,
	TYPE_GDK_PIXBUF,
	TYPE_HIGHLIGHT,
	TYPE_MOUSE_CURSOR,
	TYPE_TEXT
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
	GString				*name;
	gboolean			background;					// Is this the background layer?
	gfloat				start_time;					// Number of seconds into the slide when the layer should start to appear
	gint				transition_in_type;			// The type of transition this layer uses to appear (transition in)
	gfloat				transition_in_duration;		// Number of seconds it takes for the transition in to complete
	gfloat				duration;					// Number of seconds the layer is visible for after the appearance transition
	gint				transition_out_type;		// The type of transition this layer uses to disappear (transition out)
	gfloat				transition_out_duration;	// Number of seconds it takes for the transition out to complete
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	gboolean			visible;					// Is this layer visible or not?
	GString				*external_link;
	GString				*external_link_window;		// Name of the target window to load the external link in. Defaults to _self for swf.
	void				*dictionary_shape;			// SWF dictionary shape
	void				*display_list_item;			// SWF display list item
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
	GdkPixbuf			*image_data;
	cairo_pattern_t		*cairo_pattern;				// Cairo pattern representation of the image_data pixbuf
	gboolean			modified;					// FALSE if an image hasn't been modified, TRUE if it has (i.e. cropped)
} layer_image;

// Defines the properties making up a mouse pointer layer
typedef struct
{
	gint				width;
	gint				height;
	gint				click;
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
	GdkPixbuf			*thumbnail;
	GList				*layers;
	GString				*name;
	GtkWidget			*timeline_widget;
	gfloat				duration;
	GdkPixmap			*scaled_cached_pixmap;
	gboolean			cached_pixmap_valid;
	gint				num_layers;
} slide;

// Defines the information needed for each element of the swf timing array
typedef struct
{
	gboolean			action_this;				// Process this element?
	guint				layer_type;					// The type of element being displayed
	layer				*layer_info;				// Pointer to the layer info
	GString				*object_name;				// The name of the object on the display list
	gboolean			add;						// Add to the display list in this frame?
	gboolean			remove;						// Remove from the display list in this frame?
	gboolean			opacity_change;				// Should there be a change in opacity?
	gboolean			is_moving;					// Should the position of this element be adjusted in this frame?
	gfloat				x_position;					// In twips
	gfloat				y_position;					// In twips
	gint				opacity;					// Opacity level (0-65535)
	gint				depth;						// The layer depth of this item when displayed
} swf_frame_element;

// Defines the structure of a validation field entry
typedef struct
{
	guint				value_id;
	gchar				*name_string;
	guint				base_type;
	guint				capabilities;
	gfloat				min_value;
	gfloat				max_value;
} validatation_entry;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SALASAGA_TYPES_H__

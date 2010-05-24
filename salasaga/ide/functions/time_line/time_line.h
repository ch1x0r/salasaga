/*
 * $Id$
 *
 * Copyright (C) 2005-2010 Digital Distribution Global Training Solutions Pty. Ltd.
 * <justin@salasaga.org>
 *
 * This file is part of Salasaga.
 *
 * Salasaga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TIME_LINE_H_
#define TIME_LINE_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Include needed classes
#include <glib.h>
#include <glib-object.h>

// Public macros
#define TIME_LINE_TYPE (time_line_get_type())
#define TIME_LINE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TIME_LINE_TYPE, TimeLine))
#define TIME_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), TIME_LINE_TYPE, TimeLineClass))
#define IS_TIME_LINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), TIME_LINE_TYPE))
#define IS_TIME_LINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TIME_LINE_TYPE))

// Public structures
typedef struct _TimeLine TimeLine;
typedef struct _TimeLineClass TimeLineClass;
struct _TimeLine
{
	GtkDrawingArea drawing_area;
};

struct _TimeLineClass
{
	GtkDrawingAreaClass parent_class;
};

// fixme2: Pulled these initial sizes out of the air, they may need to be revisited
#define WIDGET_MINIMUM_HEIGHT	150
#define WIDGET_MINIMUM_WIDTH	500

// Definitions for the scaling "adjustment" symbols (the plus and minus symbols)
#define ADJUSTMENTS_X	85
#define ADJUSTMENTS_Y	2
#define ADJUSTMENTS_SIZE	10

// Definitions for the cursor head - the triangular part at the top of the time line cursor
#define CURSOR_HEAD_TOP		1
#define CURSOR_HEAD_WIDTH	10

// * Private enums *
enum
{
	RESIZE_NONE,
	RESIZE_TRANS_IN_START,
	RESIZE_LAYER_START,
	RESIZE_LAYER_DURATION,
	RESIZE_TRANS_OUT_DURATION
};

// Private macros
#define TIME_LINE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TIME_LINE_TYPE, TimeLinePrivate))

// Private structures
typedef struct _TimeLinePrivate TimeLinePrivate;
struct _TimeLinePrivate
{
	GdkPixmap			*cached_bg_image;			// Cache of the timeline background image, for drawing upon

	GdkPixmap			*cached_bg_image_top_left;
	GdkPixmap			*cached_bg_image_top_right;
	GdkPixmap			*cached_bg_image_bot_left;

	// Tables used for the widget
	GtkWidget			*main_table; // consist of 4 cells
	GtkWidget			*top_left_table;
	GtkWidget			*top_right_table;
	GtkWidget			*bot_left_table;
	GtkWidget			*bot_right_table;

	GtkWidget			*top_left_vp;
	GtkWidget			*top_right_vp;
	GtkWidget			*bot_left_vp;
	GtkWidget			*bot_right_vp;

	GtkWidget			*top_left_swin;
	GtkWidget			*top_right_swin;
	GtkWidget			*bot_left_swin;
	GtkWidget			*bot_right_swin;

	GtkWidget			*top_left_evb;
	GtkWidget			*top_right_evb;
	GtkWidget			*bot_left_evb;
	GtkWidget			*bot_right_evb;

	gboolean			cached_bg_valid;			// Flag for whether the timeline background cache image is valid
	gboolean			cursor_drag_active;			// Tracks whether we're dragging the time line cursor or not
	gfloat				cursor_position;			// Where in the slide the cursor is positioned (in seconds or part thereof)

	GdkPixmap			*display_buffer;			// The rendered version of the timeline
	GdkPixmap			*display_buffer_top_left;
	GdkPixmap			*display_buffer_top_right;
	GdkPixmap			*display_buffer_bot_left;

	gboolean			drag_active;				// Tracks whether we have an active mouse drag or not
	gint				guide_line_end;				// The pixel number of the ending guide line
	gint				guide_line_start;			// The pixel number of the starting guide line
	gint				guide_line_resize;			// The pixel number of the resizing guide line
	gint				left_border_width;			// Number of pixels in the left border (layer name) area
	gint				mouse_x;					// Mouse pointer position
	gint				mouse_y;					// Mouse pointer position
	guint				resize_type;				// Tracks whether we have an active layer resize or not
	gint				row_height;					// Number of pixels in each layer row
	gint				selected_layer_num;			// The number of the selected layer
	gfloat				stored_slide_duration;		// The original duration of the slide in seconds
	gint				stored_x;
	gint				stored_y;
	guint				timeout_id;					// ID for the timeout source
	gint				top_border_height;			// Number of pixels in the top border (cursor) area
};

// Functions to access time line variables
gint time_line_get_pixels_per_second();
undo_history_data *time_line_get_undo_item();
void time_line_set_pixels_per_second(gint pps);
void time_line_set_undo_item(undo_history_data *item_data);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* TIME_LINE_H_ */

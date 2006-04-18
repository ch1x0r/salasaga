/*
 * $Id$
 * 
 * Flame Project: Include file for structures and similar used throughout the code
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.3  2006/04/18 18:02:00  vapour
 * Bumped the version number to 0.72, to mark the successful compilation on both Windows and Solaris as well. :)
 *
 * Revision 1.2  2006/04/16 06:01:42  vapour
 * Removed header info copied from my local repository.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

#ifndef __FLAME_TYPES_H__
#define __FLAME_TYPES_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Set some application constants
#define APP_NAME "Flame Project Editing GUI"
#define	APP_VERSION "0.72"

// Define a run time check that hasn't been added to GTK
#define GTK_TYPE_TREE_ITER				(gtk_tree_iter_get_type ())
#define GTK_TREE_ITER(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TREE_ITER, GtkTreeIter))


// * Define the structures used in the application *
// fixme4: These structures need better descriptions

// The known object types
typedef enum {
	TYPE_GDK_PIXBUF,
	TYPE_MOUSE_CURSOR,
	TYPE_EMPTY,
	TYPE_TEXT,
	TYPE_HIGHLIGHT
} ObjectType;

// Fields of the timeline widget
enum
{
	TIMELINE_NAME,
	TIMELINE_TYPE,
	TIMELINE_VISIBILITY,
	TIMELINE_START,
	TIMELINE_FINAL,
	TIMELINE_X_OFF,
	TIMELINE_Y_OFF,
	TIMELINE_TEXT,
	TIMELINE_N_COLUMNS
};

// Defines the object and properties making up a layer
typedef struct
{
	guint			object_type;
	GObject			*object_data;
	guint			start_frame;
	guint			final_frame;
	GString			*name;
	GtkTreeIter		*row_iter;
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
	GString			*image_path;
	GdkPixbuf		*image_data;
} layer_image;

// Defines the properties making up a text layer
typedef struct
{
	gint				x_offset_start;
	gint				y_offset_start;
	gint				x_offset_finish;
	gint				y_offset_finish;
	guint			rendered_width;
	guint			rendered_height;
	GdkColor			text_color;
	gfloat			font_size;
	GtkTextBuffer	*text_buffer;
} layer_text;

// Defines the collection of objects and properties making up a slide
typedef struct
{
	GtkWidget		*event_box;
	GtkImage			*thumbnail;
	gulong			click_handler;
	GList			*layers;
	GtkTooltips		*tooltip;
	guint			number;
	GtkWidget		*timeline_widget;
	GtkListStore		*layer_store;
} slide;

// Defines the structure of a boundary box
typedef struct
{
	GdkRegion		*region_ptr;
	GList			*layer_ptr;
} boundary_box;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FLAME_TYPES_H__

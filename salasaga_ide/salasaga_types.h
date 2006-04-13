/*
 * $Id$
 * 
 * Flame Project: Include file for structures and similar used throughout the code
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.46  2006/04/11 09:52:20  jc
 * Incremented the application version number.
 *
 * Revision 1.45  2006/04/10 07:57:18  jc
 * Added a structure type for empty layers, presently just being the background color for it.
 *
 * Revision 1.44  2006/04/10 05:44:05  jc
 * Added an element to the slide structure to hold the event handler for the assigned event box click hander.
 *
 * Revision 1.43  2006/04/06 01:20:22  jc
 * Bumped up the application version number.
 *
 * Revision 1.42  2006/03/31 05:06:08  jc
 * Bumped application version number.
 *
 * Revision 1.41  2006/03/28 14:09:06  jc
 * Changed the font size in the text layer from an integer to a floating point.
 *
 * Revision 1.40  2006/03/23 16:15:30  jc
 * Adjusted the boundary_box definition, and added a GTK_TYPE_TREE_ITER() check (missing from GTK at present).
 *
 * Revision 1.39  2006/03/23 07:15:10  jc
 * + Added the boundary_box structure type.
 * + Bumped the application version number.
 *
 * Revision 1.38  2006/03/22 13:49:23  jc
 * Added fields to the text layer structure, to store the rendered width and height it covers.
 *
 * Revision 1.37  2006/03/21 08:10:07  jc
 * + Text layer data structure now use a GtkTextBuffer for the text data.
 * + Text layer dialogs now use a GtkTextView.
 * + Added the code to edit an image layer.
 *
 * Revision 1.36  2006/03/21 05:14:04  jc
 * Converting the text layer to use a GdkColor and color picker instead of three separate component values.
 *
 * Revision 1.35  2006/03/19 10:59:35  jc
 * Bumped up the version number.
 *
 * Revision 1.34  2006/03/18 01:09:47  jc
 * Expanded  the x and y offsets for the layer types to have both start and finish positions.
 *
 * Revision 1.33  2006/03/13 04:05:59  jc
 * Bumped up the version number.
 *
 * Revision 1.32  2006/03/12 04:13:39  jc
 * Expanded the layer_image type to include the image data and image path.
 *
 * Revision 1.31  2006/03/12 03:48:08  jc
 * Added an initial layer_image structure;
 *
 * Revision 1.30  2006/03/07 14:19:28  jc
 * + Updated layer_delete code to correctly remove the selected layer.
 * + Tweaked layer creation code to ensure layers are created in the correct order.
 *
 * Revision 1.29  2006/03/06 10:46:04  jc
 * Bumped the version number slightly.
 *
 * Revision 1.28  2006/03/05 09:34:38  jc
 * + Added first working code to render slide layers using cairo.
 * + Bumped up the application version number.
 *
 * Revision 1.27  2006/03/04 11:52:08  jc
 * Started moving backend functions into new source backend.c and header backend.h files.
 *
 * Revision 1.26  2006/03/02 09:45:36  jc
 * Incremented the version number.
 *
 * Revision 1.25  2006/02/23 10:21:10  jc
 * Bumped the application version number to 0.61.
 *
 * Revision 1.24  2006/02/21 09:46:48  jc
 * Added the HIGHLIGHT layer type.
 *
 * Revision 1.23  2006/02/20 08:27:51  jc
 * Bumped the application version number to 0.60.
 *
 * Revision 1.22  2006/02/20 08:10:15  jc
 * Updated the timeline list store widget so it displays the text in a text layer.
 *
 * Revision 1.21  2006/02/19 02:05:10  jc
 * Added initial working code to prompt for and store the details of the new text layer.
 *
 * Revision 1.20  2006/02/18 09:01:53  jc
 * Started adding a layer button to add a text layer.
 *
 * Revision 1.19  2006/02/18 08:22:10  jc
 * Added a pointer to a Gtk list store to the slide definition.
 *
 * Revision 1.18  2006/02/16 09:32:33  jc
 * Started converting the timeline widget to a GtkTreeView.
 *
 * Revision 1.17  2006/02/15 20:38:42  jc
 * Timeline widget now works properly again, with layer creation and destruction functions simplified.
 *
 * Revision 1.16  2006/02/14 09:49:09  jc
 * Making progress with the debugging... it turns out I was accidentally adding a viewport to the container each time, and this needs to be removed with each timeline redraw as well.
 *
 * Revision 1.15  2006/02/12 00:52:04  jc
 * Changing how timeline widgets are used.  Something is grossly unhappy.
 *
 * Revision 1.14  2006/02/12 00:04:35  jc
 * Simplified the layer data structure.
 *
 * Revision 1.13  2006/02/09 11:20:18  jc
 * Bumped up the version number.
 *
 * Revision 1.12  2006/02/09 10:01:10  jc
 * Started adding code to add a new layer when the user clicks the Add button in the time line toolbar.
 *
 * Revision 1.11  2006/02/07 10:26:30  jc
 * Added an initial name attribute to the layer structure.
 *
 * Revision 1.10  2006/02/05 06:59:54  jc
 * + Updated naming of things to be "folder" rather than "path".
 * + Added FileChooserButtons into the Preferences box, but am getting assertion errors at runtime.
 *
 * Revision 1.9  2006/01/29 05:49:10  jc
 * Added initial working code to correctly output the starting and finishing times in the output SVG file.
 *
 * Revision 1.8  2006/01/29 05:09:19  jc
 * + Added code for changing the output resolution via the resolution selector.
 * + Added initial working code (incomplete) for creating the output SVG animation (but not its component png).
 *
 * Revision 1.7  2006/01/26 10:39:21  jc
 * Added a working (basic) About dialog box.
 *
 * Revision 1.6  2006/01/26 09:54:29  jc
 * + Added a rudimentary Fit to Width zoom level.
 * + Incremented the version number to 0.54.
 *
 * Revision 1.5  2006/01/22 04:27:27  jc
 * Added a 1 pixel black border to the slide displayed in the working area.
 *
 * Revision 1.4  2006/01/22 02:03:07  jc
 * Updated the FlameObject type to hold gdk_pixmaps ok.
 *
 * Revision 1.3  2006/01/21 08:10:36  jc
 * Started adding code to display the current slide in the drawing area.
 *
 * Revision 1.2  2006/01/21 06:54:02  jc
 * Altered some of the initial typedefs and enums.
 *
 * Revision 1.1  2006/01/21 05:13:05  jc
 * Moved the typedef and enum definitions into a common file.
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
#define	APP_VERSION "0.71"

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

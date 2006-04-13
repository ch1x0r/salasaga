/*
 * $Id$
 * 
 * Flame Project: Header file for all non specific GUI related funtions
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.48  2006/04/11 09:51:27  jc
 * Updated the parameter list for the thumbnail drawing function.
 *
 * Revision 1.47  2006/04/11 06:30:00  jc
 * Added working code for cropping image layers.
 *
 * Revision 1.46  2006/04/09 12:14:50  jc
 * + Moved flame-types.h to the top of our own header include list.
 * + Moved the film_strip_slide_clicked function into the callbacks file.
 * + Added initial working code to the slide_delete function.  It works, but should be rounded out.
 *
 * Revision 1.45  2006/04/08 07:28:53  jc
 * Added a new function, refresh_film_strip, for putting the slide thumbnails into the film strip area.
 *
 * Revision 1.44  2006/04/06 03:33:15  jc
 * + Moved slide_insert(), slide_delete(), slide_move_up(), and slide_move_down() here.
 * + Added missing declaration for film_strip_slide_clicked().
 *
 * Revision 1.43  2006/03/26 05:59:09  jc
 * + Simplified the operation of the compress_layers function, it should also be slightly faster now.
 * + Added a draw_thumbnail function, called whenever layers are changed, so that the slide thumbnails are kept in sync.
 *
 * Revision 1.42  2006/03/24 05:30:30  jc
 * Added the draw_bounding_box() function - not presently used - and made other small updates.
 *
 * Revision 1.41  2006/03/22 07:11:44  jc
 * Updated with renamed functions, and callbacks that have been moved out.
 *
 * Revision 1.40  2006/03/21 08:10:07  jc
 * + Text layer data structure now use a GtkTextBuffer for the text data.
 * + Text layer dialogs now use a GtkTextView.
 * + Added the code to edit an image layer.
 *
 * Revision 1.39  2006/03/20 06:20:19  jc
 * Added working code for adding an image layer.  Still need to adjust the workspace drawing code to handle it though.
 *
 * Revision 1.38  2006/03/19 10:39:33  jc
 * Moved the highlight box drawing code into its own function, so it can be called by other things.
 *
 * Revision 1.37  2006/03/18 05:14:20  jc
 * Highlight layers are now properly updated through a dialog box for the user.
 *
 * Revision 1.36  2006/03/18 04:36:22  jc
 * Text layers are now properly updated through a dialog box for the user.
 *
 * Revision 1.35  2006/03/14 10:03:54  jc
 * + Changed the Add Empty Layer button into an Edit Layer button (more useful).  No code behind it though.
 * + Added buttons (no code behind them) to move a layer up, and move a layer down.
 *
 * Revision 1.34  2006/03/06 10:44:01  jc
 * + Began adding code to delete the layer selected in the timeline.
 * + Removed a few redraws of the timeline that weren't needed.
 *
 * Revision 1.33  2006/03/05 09:34:38  jc
 * + Added first working code to render slide layers using cairo.
 * + Bumped up the application version number.
 *
 * Revision 1.32  2006/03/05 04:17:20  jc
 * Added initial play-around code testing out cairo functions.
 *
 * Revision 1.31  2006/03/04 11:52:08  jc
 * Started moving backend functions into new source backend.c and header backend.h files.
 *
 * Revision 1.30  2006/02/27 18:23:57  jc
 * Began breaking out the slide output code in the menu_export_svg_animation function, so it runs in a g_list_foreach compatible way.
 *
 * Revision 1.29  2006/02/23 10:20:55  jc
 * + Added main toolbar icons for Screenshot Capture & File Open.
 * + Changed the main toolbar icons for Export of SVG and Flash.
 * + Added main menubar options for Screenshot Capture, Screenshot Import, File Open, Layer Add Empty, Layer Delete, Layer Add Text, Layer Add Hightlight.
 * + Added many function stubs.
 *
 * Revision 1.28  2006/02/21 09:58:17  jc
 * Added timeline toolbar buttons for adding a highlight layer, and cropping the project.
 *
 * Revision 1.27  2006/02/18 09:01:53  jc
 * Started adding a layer button to add a text layer.
 *
 * Revision 1.26  2006/02/18 08:22:27  jc
 * Converted the timeline widget to a GtkListStore.
 *
 * Revision 1.25  2006/02/15 20:38:42  jc
 * Timeline widget now works properly again, with layer creation and destruction functions simplified.
 *
 * Revision 1.24  2006/02/12 01:36:01  jc
 * Debugging underway so the timeline widget does not get destroyed each time the timeline is refreshed.
 *
 * Revision 1.23  2006/02/12 01:09:13  jc
 * Fixed a missing semi-colon, which was causing massive amounts of errors.
 *
 * Revision 1.22  2006/02/12 00:52:04  jc
 * Changing how timeline widgets are used.  Something is grossly unhappy.
 *
 * Revision 1.21  2006/02/09 10:01:10  jc
 * Started adding code to add a new layer when the user clicks the Add button in the time line toolbar.
 *
 * Revision 1.20  2006/02/07 10:58:34  jc
 * Added very initial code show the layer information in the time line.
 * Still needs a lot of work.
 *
 * Revision 1.19  2006/02/06 08:44:47  jc
 * Started creating the draw_timeline() function, which will display the layers of the selected slide in the time line area.
 *
 * Revision 1.18  2006/02/04 06:41:41  jc
 * Began adding working code for the Preferences dialog.
 *
 * Revision 1.17  2006/02/02 09:31:26  jc
 * + Added code to launch a browser for the web based Help menu links.
 * + Added a check to the SVG export code, so it only runs if there's a project loaded.
 *
 * Revision 1.16  2006/01/31 10:46:06  jc
 * Updated to use an ItemFactory for the menu creation, rather than the initial manual approach.
 *
 * Revision 1.15  2006/01/30 08:21:00  jc
 * Added an initial (untested) error handling function.
 *
 * Revision 1.14  2006/01/29 05:09:19  jc
 * + Added code for changing the output resolution via the resolution selector.
 * + Added initial working code (incomplete) for creating the output SVG animation (but not its component png).
 *
 * Revision 1.13  2006/01/28 08:55:11  jc
 * + Updated the default zoom level to be Fit to Window.
 * + Added code to automatically adjust the zoom level when the window is resized.
 *
 * Revision 1.12  2006/01/26 10:39:21  jc
 * Added a working (basic) About dialog box.
 *
 * Revision 1.11  2006/01/26 05:28:38  jc
 * Added initial working code for a zoom level selector and an output resolution selector.  The zoom level selector works ok, but the message bar area needs work, and the callback function for the resolution selector also still needs writing.
 *
 * Revision 1.10  2006/01/24 10:11:58  jc
 * Added a stub function for exporting flash animation as well.
 *
 * Revision 1.9  2006/01/23 09:24:26  jc
 * Added a blank stub function for exporting an SVG Animation.
 *
 * Revision 1.8  2006/01/22 00:18:19  jc
 * Started getting things ready for the code to compress the layers to an image.
 *
 * Revision 1.7  2006/01/21 08:27:04  jc
 * Started rearranging code into a stub function for creating an image from a slide's layers.
 *
 * Revision 1.6  2006/01/21 08:10:36  jc
 * Started adding code to display the current slide in the drawing area.
 *
 * Revision 1.5  2006/01/21 05:12:42  jc
 * Fixed a small inconsistency.
 *
 * Revision 1.4  2006/01/15 05:51:21  jc
 * + Added a new File -> New menu item and tied the existing screenshot loading code to that instead
 * + Altered the application title to Flame Project Editing GUI
 * + Gave an informational message dialog to indicate how many screenshots were found in the project folder
 *
 * Revision 1.3  2006/01/15 04:21:04  jc
 * Got the gui program to compile with GUI functions in their own files.
 *
 * Revision 1.2  2006/01/15 03:54:26  jc
 * Trying to get the damn project to compile.
 *
 * Revision 1.1  2006/01/15 03:38:36  jc
 * Added a place for generic GUI related code, and a first function, to handle when a user clicks File -> Open from the top menu.
 *
 */

#ifndef __GUIFUNCTIONS_H__
#define __GUIFUNCTIONS_H__

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Include required GTK widgets, in case this is used from something that hasn't already
#include <gtk/gtkwidget.h>
#include <gtk/gtkvbox.h>

// Menu functions
void menu_edit_preferences(void);
void menu_export_flash_animation(void);
void menu_export_svg_animation(void);
void menu_file_new(void);
void menu_file_open(void);
void menu_file_save(void);
void menu_file_save_as(void);
void menu_help_about(void);
void menu_help_register(void);
void menu_help_survey(void);
void menu_help_website(void);
void menu_screenshots_capture(void);
void menu_screenshots_import(void);

// Other functions
GdkPixbuf *compress_layers(GList *which_slide, guint width, guint height);
void compress_layers_inner(gpointer element, gpointer user_data);
GtkWidget *construct_timeline_widget(slide *slide_data);
gboolean display_dialog_highlight(layer *tmp_layer, gchar *dialog_title);
gboolean display_dialog_image(layer *tmp_layer, gchar *dialog_title, gboolean request_file);
gboolean display_dialog_text(layer *tmp_layer, gchar *dialog_title);
gint display_warning(gchar *warning_string);
void draw_bounding_box(GtkWidget *widget, GdkRegion *region);
void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color);
void draw_thumbnail(GList *which_slide);
void draw_timeline(void);
void draw_workspace(void);
void image_crop(void);
void layer_delete(void);
void layer_edit(void);
void layer_move_down(void);
void layer_move_up(void);
void layer_new_highlight(void);
void layer_new_image(void);
void layer_new_text(void);
void project_crop(void);
void refresh_film_strip(void);
void slide_insert(void);
void slide_delete(void);
void slide_move_up(void);
void slide_move_down(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUIFUNCTIONS_H__

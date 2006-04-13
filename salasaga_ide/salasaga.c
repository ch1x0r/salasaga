/*
 * $Id$
 * 
 * Flame Project: Editing GUI
 * 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 *
 * Revision 1.111  2006/04/13 15:25:46  jc
 * Now initialises sound.
 *
 * Revision 1.110  2006/04/11 06:29:33  jc
 * Added an image crop button.
 *
 * Revision 1.109  2006/04/10 08:17:37  jc
 * Greyed out the project menu items at program startup again.
 *
 * Revision 1.108  2006/04/09 12:12:36  jc
 * Moved flame-types.h to the top of our own header include list.
 *
 * Revision 1.107  2006/04/07 00:54:35  jc
 * Moved much of the film strip setup code into create_film_strip().
 *
 * Revision 1.106  2006/04/06 01:19:58  jc
 * Simplified the greying out and re-enabling of menu items.
 *
 * Revision 1.105  2006/04/05 17:21:26  jc
 * Updated the create_menu_bar function to use the menu_bar global, and also create most initial menu items greyed out.
 *
 * Revision 1.104  2006/04/05 17:01:19  jc
 * Change menu_bar into a global variable.
 *
 * Revision 1.103  2006/04/04 13:32:12  jc
 * Updated the slide menu options to point to the stub functions.  Added the move up and move down options too.
 *
 * Revision 1.102  2006/04/02 15:29:20  jc
 * Added menu items to create and delete slides.
 *
 * Revision 1.101  2006/04/01 16:46:37  jc
 * Automatically assigns Control-Printscreen to run flame-capture now.
 *
 * Revision 1.100  2006/03/31 15:57:21  jc
 * + Updated the toolbar icons for New, Open, Save, and Quit to use the GTK stock icons.
 * + Updated the remaining toolbar icons to use the icons I just created.
 *
 * Revision 1.99  2006/03/31 05:05:44  jc
 * + Updated global variables.
 * + Added mouse button release hander to working area.
 *
 * Revision 1.98  2006/03/26 23:41:51  jc
 * + Small code clean ups.
 * + Added code for the frames_per_second variable.
 *
 * Revision 1.97  2006/03/25 05:18:24  jc
 * Now calculates the project width and height from the screen size we're displaying upon.
 *
 * Revision 1.96  2006/03/24 05:29:24  jc
 * Updated the externs and adjusted the drawing area event mask.
 *
 * Revision 1.95  2006/03/23 07:15:51  jc
 * Adding further collision detection callbacks and code.
 *
 * Revision 1.94  2006/03/22 09:24:36  jc
 * Added an initial (stub) handler for capturing mouse motion events on the drawing area.
 *
 * Revision 1.93  2006/03/22 07:33:32  jc
 * Added an event handler and initial working code to the drawing area, for catching mouse clicks.
 *
 * Revision 1.92  2006/03/22 07:10:59  jc
 * + Changed the background color to use a GdkColor.
 * + Changed the working area to be a drawing_area widget.
 *
 * Revision 1.91  2006/03/20 06:18:53  jc
 * + Added an icon for creating a new image layer.
 * + Updated the existing icons to use the new ones I created instead.
 *
 * Revision 1.90  2006/03/16 12:34:46  jc
 * Updated to use some of the icons from openclipart.org.
 *
 * Revision 1.89  2006/03/14 11:48:38  jc
 * Added a note to look on openclipart.org for icons.
 *
 * Revision 1.88  2006/03/14 10:03:54  jc
 * + Changed the Add Empty Layer button into an Edit Layer button (more useful).  No code behind it though.
 * + Added buttons (no code behind them) to move a layer up, and move a layer down.
 *
 * Revision 1.87  2006/03/13 04:04:05  jc
 * Updated the includes.
 *
 * Revision 1.86  2006/03/12 10:12:57  jc
 * Updated to save application defaults via GConf at shutdown, and restore them at startup.
 *
 * Revision 1.85  2006/03/07 14:19:28  jc
 * + Updated layer_delete code to correctly remove the selected layer.
 * + Tweaked layer creation code to ensure layers are created in the correct order.
 *
 * Revision 1.84  2006/03/06 10:44:01  jc
 * + Began adding code to delete the layer selected in the timeline.
 * + Removed a few redraws of the timeline that weren't needed.
 *
 * Revision 1.83  2006/03/05 02:37:02  jc
 * + Merged all of the statusbar contexts into a single global one.
 * + Updated the SVG slide export function so it again works correctly, and improved it.
 *
 * Revision 1.82  2006/03/05 01:38:21  jc
 * Added a global variable to hold the dedicated screenshots folder.
 *
 * Revision 1.81  2006/03/04 11:52:08  jc
 * Started moving backend functions into new source backend.c and header backend.h files.
 *
 * Revision 1.80  2006/02/28 14:28:43  jc
 * Added a note for how to - more properly - bind the printscreen key to flame-capture.
 *
 * Revision 1.79  2006/02/23 10:20:55  jc
 * + Added main toolbar icons for Screenshot Capture & File Open.
 * + Changed the main toolbar icons for Export of SVG and Flash.
 * + Added main menubar options for Screenshot Capture, Screenshot Import, File Open, Layer Add Empty, Layer Delete, Layer Add Text, Layer Add Hightlight.
 * + Added many function stubs.
 *
 * Revision 1.78  2006/02/22 11:55:37  jc
 * + Moved the Crop icon to the main toolbar.
 * + Added icons for Save, Save As, Import, and Export Flash, to the main toolbar.
 *
 * Revision 1.77  2006/02/21 09:58:17  jc
 * Added timeline toolbar buttons for adding a highlight layer, and cropping the project.
 *
 * Revision 1.76  2006/02/18 09:01:53  jc
 * Started adding a layer button to add a text layer.
 *
 * Revision 1.75  2006/02/18 08:35:01  jc
 * Simplified the timeline widget container, fixing a bug.
 *
 * Revision 1.74  2006/02/12 00:52:04  jc
 * Changing how timeline widgets are used.  Something is grossly unhappy.
 *
 * Revision 1.73  2006/02/11 12:52:47  jc
 * Added scroll bars to the time line widget.
 *
 * Revision 1.72  2006/02/11 12:38:20  jc
 * Adjusted the time line table layout to be top left and more predictable.
 *
 * Revision 1.71  2006/02/09 11:20:07  jc
 * Trying a GtkAlignment widget instead.  Did not seem to work.  Doh.
 *
 * Revision 1.70  2006/02/09 11:05:51  jc
 * Adding code to try and force the table to remain at the top of the time line.
 *
 * Revision 1.69  2006/02/09 10:01:10  jc
 * Started adding code to add a new layer when the user clicks the Add button in the time line toolbar.
 *
 * Revision 1.68  2006/02/08 08:56:25  jc
 * Added a toolbar to the time line area.
 *
 * Revision 1.67  2006/02/06 08:44:47  jc
 * Started creating the draw_timeline() function, which will display the layers of the selected slide in the time line area.
 *
 * Revision 1.66  2006/02/05 06:59:54  jc
 * + Updated naming of things to be "folder" rather than "path".
 * + Added FileChooserButtons into the Preferences box, but am getting assertion errors at runtime.
 *
 * Revision 1.65  2006/02/05 01:33:52  jc
 * Added default output quality, default slide length, default background colour, and scaling quality to the Preferences dialog.
 *
 * Revision 1.64  2006/02/04 15:37:54  jc
 * Added fields for default output width and height to the Preferences dialog.
 *
 * Revision 1.63  2006/02/04 06:41:41  jc
 * Began adding working code for the Preferences dialog.
 *
 * Revision 1.62  2006/02/02 09:31:26  jc
 * + Added code to launch a browser for the web based Help menu links.
 * + Added a check to the SVG export code, so it only runs if there's a project loaded.
 *
 * Revision 1.61  2006/01/31 10:46:06  jc
 * Updated to use an ItemFactory for the menu creation, rather than the initial manual approach.
 *
 * Revision 1.60  2006/01/29 10:16:52  jc
 * + Added a toolbar icon to export as an SVG.
 * + Small cleanups.
 *
 * Revision 1.59  2006/01/29 05:57:54  jc
 * Changed a few defaults to assist development usability.
 *
 * Revision 1.58  2006/01/29 05:35:12  jc
 * Added initial working code that saves the component png files.
 *
 * Revision 1.57  2006/01/29 05:09:19  jc
 * + Added code for changing the output resolution via the resolution selector.
 * + Added initial working code (incomplete) for creating the output SVG animation (but not its component png).
 *
 * Revision 1.56  2006/01/28 12:32:04  jc
 * Turned the default slide length into a variable, rather than having it hard coded.
 *
 * Revision 1.55  2006/01/28 08:55:11  jc
 * + Updated the default zoom level to be Fit to Window.
 * + Added code to automatically adjust the zoom level when the window is resized.
 *
 * Revision 1.54  2006/01/28 01:30:31  jc
 * + Added default values for the zoom and resolution selectors.
 * + Removed the hard coded default zoom, so it uses whatever's selected in the zoom selector instead.
 *
 * Revision 1.53  2006/01/26 12:00:08  jc
 * Added a Ready message to the status bar upon loading.
 *
 * Revision 1.52  2006/01/26 11:54:39  jc
 * Tweaked to work out a better Fit to Width zoom level.
 *
 * Revision 1.51  2006/01/26 10:39:21  jc
 * Added a working (basic) About dialog box.
 *
 * Revision 1.50  2006/01/26 09:54:29  jc
 * + Added a rudimentary Fit to Width zoom level.
 * + Incremented the version number to 0.54.
 *
 * Revision 1.49  2006/01/26 06:14:06  jc
 * The layout for the status bar, zoom selector, and resolution selector, is reasonable now (though could be better).
 *
 * Revision 1.48  2006/01/26 05:51:38  jc
 * Refining the construction and placement of the status bar, zoom, and resolution selectors.
 *
 * Revision 1.47  2006/01/26 05:28:38  jc
 * Added initial working code for a zoom level selector and an output resolution selector.  The zoom level selector works ok, but the message bar area needs work, and the callback function for the resolution selector also still needs writing.
 *
 * Revision 1.46  2006/01/25 11:16:53  jc
 * Moved the configuration variables into two types, application level ones (including defaults), and project level ones.
 *
 * Revision 1.45  2006/01/24 10:11:58  jc
 * Added a stub function for exporting flash animation as well.
 *
 * Revision 1.44  2006/01/23 09:24:26  jc
 * Added a blank stub function for exporting an SVG Animation.
 *
 * Revision 1.43  2006/01/23 07:56:21  jc
 * Added an output_path global variable.
 *
 * Revision 1.42  2006/01/22 02:02:29  jc
 * Added a default background color for the preferences menu.
 *
 * Revision 1.41  2006/01/22 01:18:50  jc
 * Added initial working code for scaling and displaying the screenshot into the working area.
 *
 * Revision 1.40  2006/01/22 00:18:19  jc
 * Started getting things ready for the code to compress the layers to an image.
 *
 * Revision 1.39  2006/01/21 08:16:52  jc
 * Moved the code for creating an aspect frame into the working_area creation function.
 *
 * Revision 1.38  2006/01/21 08:10:36  jc
 * Started adding code to display the current slide in the drawing area.
 *
 * Revision 1.37  2006/01/21 07:29:22  jc
 * Updated the working area to use an Aspect Frame.
 *
 * Revision 1.36  2006/01/21 06:52:52  jc
 * Updated the toolbar icons to load with a given - configurable - height, and changed the icons to be SVG files.
 *
 * Revision 1.35  2006/01/21 05:13:05  jc
 * Moved the typedef and enum definitions into a common file.
 *
 * Revision 1.34  2006/01/21 05:04:55  jc
 * Added an initial enum defining known object types.
 *
 * Revision 1.33  2006/01/21 01:25:02  jc
 * Adding the foundational structures and code for using layers.
 *
 * Revision 1.32  2006/01/20 12:13:57  jc
 * Moved the image thumbnails and event boxes into a global GList.
 *
 * Revision 1.31  2006/01/20 11:26:10  jc
 * Added the slide typedef, and a slides GList.
 *
 * Revision 1.30  2006/01/19 11:38:02  jc
 * Updated the thumbnail loading code to give status bar messages about the number of images loaded.
 *
 * Revision 1.29  2006/01/18 11:21:44  jc
 * Added a Quit icon to the toolbar, and pointed the icons to some present on my Gentoo box (gnome ones I think).
 *
 * Revision 1.28  2006/01/18 11:10:56  jc
 * Added an initial working toolbar, with an initial working New button.
 *
 * Revision 1.27  2006/01/16 08:36:19  jc
 * Added Edit and Help menu items.
 *
 * Revision 1.26  2006/01/15 09:46:45  jc
 * Widened the film strip area slightly, to show all of the preview.  It's not automatically calculating the width of the gutter yet.
 *
 * Revision 1.25  2006/01/15 09:24:53  jc
 * Changed the container of the film strip and right side into a HPaned, as it gives better control of the width of previews.  All seems working so far.
 *
 * Revision 1.24  2006/01/15 08:45:47  jc
 * The initial screenshot previewing code now works.
 *
 * Revision 1.23  2006/01/15 08:36:55  jc
 * Fixed a few small warnings.
 *
 * Revision 1.22  2006/01/15 08:27:53  jc
 * Added initial code (untested) for loading and displaying the screenshots as thumbnails in the film strip area.
 *
 * Revision 1.21  2006/01/15 05:51:21  jc
 * + Added a new File -> New menu item and tied the existing screenshot loading code to that instead
 * + Altered the application title to Flame Project Editing GUI
 * + Gave an informational message dialog to indicate how many screenshots were found in the project folder
 *
 * Revision 1.20  2006/01/15 05:39:49  jc
 * Began individualising the application exit codes.
 * Connected the File -> Quit menu item to the application quit function.
 * Bumped the application version number to 0.51.
 * Removed some of the X includes that turned out to not be needed for this particular app.
 *
 * Revision 1.19  2006/01/15 04:43:27  jc
 * Removed some crufty code.
 *
 * Revision 1.18  2006/01/15 04:42:34  jc
 * Updated to make the dialog modal.
 *
 * Revision 1.17  2006/01/15 04:21:04  jc
 * Got the gui program to compile with GUI functions in their own files.
 *
 * Revision 1.16  2005/12/03 22:20:48  jc
 * Added a thought about using a Notebook widget.
 *
 * Revision 1.15  2005/11/28 09:54:29  jc
 * Updated to include the timeline widget code.
 *
 * Revision 1.14  2005/11/26 23:17:27  jc
 * Added stub layout code for the time line and working area.
 *
 * Revision 1.13  2005/11/26 14:28:07  jc
 * Updated the main area so it expands to fill the available window space, and changed the right side area into a VPaned.
 *
 * Revision 1.12  2005/11/24 10:47:53  jc
 * Moved the film strip creation code into its own function, and also changed its default scroll bar policy.
 *
 * Revision 1.11  2005/11/23 09:15:04  jc
 * Adjusted the code to place the film strip and right side parts into the correct container.
 *
 * Revision 1.10  2005/11/23 09:12:35  jc
 * Added the initial code to create a status bar.
 *
 * Revision 1.9  2005/11/23 09:01:22  jc
 * Added a note to use ItemFactory for creating the menu bar instead of the manual approach.
 *
 * Revision 1.8  2005/11/21 10:45:51  jc
 * Moved menu bar creation into its own function.
 *
 * Revision 1.7  2005/11/21 10:16:53  jc
 * Initial code to create the menu bar.
 *
 * Revision 1.6  2005/11/20 11:57:38  jc
 * Started adding comments and code outlining the area's we'll need onscreen.
 *
 * Revision 1.5  2005/11/20 02:07:59  jc
 * Added a note to consider including 3D.
 *
 * Revision 1.4  2005/11/20 01:29:24  jc
 * Added further notes.
 *
 * Revision 1.3  2005/11/20 01:17:13  jc
 * Added some commented notes about things to include.
 *
 * Revision 1.2  2005/11/20 01:08:16  jc
 * Removed the screenshot code, as it's not needed here.
 *
 * Revision 1.1  2005/11/17 10:45:56  jc
 * Initial commit.  Copied the main.c from flame-capture.
 *
 */


// Standard includes
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>

// GDK includes
#include <gdk/gdkx.h>

// GTK includes
#include <gtk/gtk.h>

// GConf includes
#include <gconf/gconf.h>

// Gnome include (for sound)
#include <libgnome/libgnome.h>

// XML includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// Ming includes
#include <ming.h>

// Flame Edit includes
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"


// Global variables
GtkWidget			*main_window;			// Widget for the main window
GdkPixmap			*backing_store;			// Pixel buffer most drawing operations are done on
GtkWidget			*right_side;				// Widget for the right side area
GtkWidget			*film_strip;				// The film strip area
GtkScrolledWindow	*film_strip_container;	// Container for the film strip
GtkWidget			*time_line_container;	// Scrolled window widget, to add scroll bars to the time line widget
GtkWidget			*time_line_vbox;			// VBox widget holding all of the time line elements
GtkWidget			*working;				// Widget for the working area
GtkWidget			*main_drawing_area;		// Widget for the drawing area
GtkItemFactory		*menu_bar = NULL;		// Widget for the menu bar
GtkWidget			*status_bar;				// Widget for the status bar
GtkComboBox			*zoom_selector;			// Widget for the zoom selector
GtkComboBox			*resolution_selector;	// Widget for the resolution selector
GList				*slides = NULL;			// Linked list holding the slide info
GList				*current_slide = NULL;	// Pointer to the presently selected slide
GIOChannel			*output_file;			// The output file handle
guint				working_width;			// Width of the display portion of the working area in pixels
guint				working_height;			// Height of the display portion of the working area in pixels
GList				*boundary_list = NULL;	// Stores a linked list of layer object boundaries
gint					stored_x;				// X co-ordinate of the mouse last click
gint					stored_y;				// Y co-ordinate of the mouse last click
guint				frames_per_second;		// Number of frames per second
guint				zoom;					// Percentage zoom to use in the drawing area
gboolean				mouse_dragging = FALSE;	// Is the mouse being dragged?

// Application default preferences
GString				*default_project_folder;	// Application default save path for project folders
GString				*default_output_folder;	// Application default save path for exporting animations
guint				default_output_width;	// Application default for how wide to create project output
guint				default_output_height;	// Application default for how high to create project output
guint				default_output_quality;	// Application default quality level [0-9] to save output png files with
GString				*screenshots_folder;		// Application default for where to store screenshots
guint				preview_width = 300;		// Width in pixel for the film strip preview (might turn into a structure later)
guint				icon_height = 30;		// Height in pixels for the toolbar icons (they're scalable SVG's)
guint				default_slide_length;	// Default length of all new slides, in frames
GdkColor				default_bg_colour;		// Default background color for slides
guint				scaling_quality;			// Default image scaling quality used

// Project preferences
GString				*project_name;			// The name of the project
GString				*project_folder;			// The path to the project folder
GString				*output_folder;			// Where to export animated SVG files too
guint				output_width;			// How wide to create project output
guint				output_height;			// How high to create project output
guint				output_quality;			// The quality level [0-9] to save output png files with
guint				project_width;			// The width of the project in pixels
guint				project_height;			// The height of the project in pixels
guint				slide_length;			// Length of all new slides, in frames

// Various globals
guint				statusbar_context;		// Context id for the status bar messages


// Callback to exit the application
gint quit_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	save_preferences_and_exit();
	return FALSE;
}

// Function to create the top menu bar
void create_menu_bar()
{
	// Purpose: To create the main application menu bar
	//
	// Inputs: None
	//
	// Returns: A pointer to a Gtk menu bar Widget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *menu_bar;
	//		menu_bar = create_menu_bar(menu_bar);
	//

	// Local variables
	GtkAccelGroup				*accel_group;		// Holds the accelerator (shortcut) keys
	static GtkItemFactoryEntry	menu_items[] =
	{
		{"/_File",					NULL,					NULL,							0,	"<Branch>"},
		{"/File/_New",				"<control>N",			menu_file_new,					0,	"<Item>"},
		{"/File/_Open",				"<control>O",			menu_file_open,					0,	"<Item>"},
		{"/File/_Save",				"<control>S",			menu_file_save,					0,	"<Item>"},
		{"/File/Save _As",			NULL,					menu_file_save_as,				0,	"<Item>"},
		{"/File/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/File/_Quit",				"<control>Q",			save_preferences_and_exit,		0,	"<Item>"},

		{"/_Edit",					NULL,					NULL,							0,	"<Branch>"},
		{"/Edit/_Preferences",		NULL,					menu_edit_preferences,			0,	"<Item>"},

		{"/_Screenshots",			NULL,					NULL,							0,	"<Branch>"},
		{"/Screenshots/_Capture",	NULL,					menu_screenshots_capture,		0,	"<Item>"},
		{"/Screenshots/_Import",		NULL,					menu_screenshots_import,			0,	"<Item>"},

		{"/S_lide",					NULL,					NULL,							0,	"<Branch>"},
		{"/Slide/_Insert",			NULL,					slide_insert,					0,	"<Item>"},
		{"/Slide/De_lete",			NULL,					slide_delete,					0,	"<Item>"},
		{"/Slide/Move _up",			NULL,					slide_move_up,					0,	"<Item>"},
		{"/Slide/Move _down",		NULL,					slide_move_down,					0,	"<Item>"},

		{"/_Layer",					NULL,					NULL,							0,	"<Branch>"},
		{"/Layer/_Edit",				NULL,					layer_edit,						0,	"<Item>"},
		{"/Layer/_Delete",			NULL,					NULL,							0,	"<Item>"},
		{"/Layer/Add _Text",			NULL,					layer_new_text,					0,	"<Item>"},
		{"/Layer/Add _Highlight",	NULL,					layer_new_highlight,				0,	"<Item>"},
		{"/Layer/Add _Image",		NULL,					layer_new_image,					0,	"<Item>"},

		{"/E_xport",					NULL,					NULL,							0,	"<Branch>"},
		{"/Export/_Flash Animation",	"<control><shift>F",		menu_export_flash_animation,		0,	"<Item>"},
		{"/Export/_SVG Animation",	"<control><shift>S",		menu_export_svg_animation,		0,	"<Item>"},

		{"/_Help",					NULL,					NULL,							0,	"<LastBranch>"},
		{"/_Help/_About",			NULL,					menu_help_about,					0,	"<Item>"	},
		{"/_Help/Sep",				NULL,					NULL,							0,	"<Separator>"},
		{"/_Help/_Register",			NULL,					menu_help_register,				0,	"<Item>"	},
		{"/_Help/_Survey",			NULL,					menu_help_survey,				0,	"<Item>"	},
		{"/_Help/_Website",			NULL,					menu_help_website,				0,	"<Item>"	},
	};  // The menu structure
	static gint					num_items = sizeof(menu_items) / sizeof(menu_items[0]);	// The number of menu items


	// Create an accelerator group
	accel_group = gtk_accel_group_new();

	// Attach the accelerator group to the main window
	gtk_window_add_accel_group(GTK_WINDOW(main_window),accel_group);

	// Create the ItemFactory menu bar
	menu_bar = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

	// Create the menu items
	gtk_item_factory_create_items(menu_bar, num_items, menu_items, NULL);

	// Grey out the project menu items
	menu_enable("/Slide", FALSE);
	menu_enable("/Layer", FALSE);
	menu_enable("/Export", FALSE);
}


// Function to create the main application toolbar
GtkWidget *create_toolbar(GtkWidget *inner_toolbar)
{
	// Purpose: To create the toolbar
	//
	// Inputs: Takes a pointer to an empty GtkWidget
	//
	// Returns: A pointer to the toolbar GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *toolbar;
	//		toolbar = create_toolbar(toolbar);
	//

	// Local variables
	GtkWidget		*capture_widget, *capture_button;
	GtkWidget		*crop_widget, *crop_button;
	GtkWidget		*export_flash_widget, *export_flash_button;
	GtkWidget		*export_svg_widget, *export_svg_button;
	GtkWidget		*import_widget, *import_button;
	GtkWidget		*new_widget, *new_button;
	GtkWidget		*open_widget, *open_button;
	GtkWidget		*quit_widget, *quit_button;
	GtkWidget		*save_widget, *save_button;

	GdkPixbuf		*tmp_gdk_pixbuf;				// Temporary GDK Pixbuf


	// Create the toolbar widget
	inner_toolbar = gtk_toolbar_new();

	// Create the icons with a given size

	// Create the New button
	new_widget = gtk_image_new_from_stock(GTK_STOCK_NEW, icon_height);
	new_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"New",  // Button label
										"Begin a new project",  // Tooltip
										"Private",  // Tooltip private info
										new_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_new),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Open button
	open_widget = gtk_image_new_from_stock(GTK_STOCK_OPEN, icon_height);
	open_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Open",  // Button label
										"Open an existing project",  // Tooltip
										"Private",  // Tooltip private info
										open_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_open),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Save button
	save_widget = gtk_image_new_from_stock(GTK_STOCK_SAVE, icon_height);
	save_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Save",  // Button label
										"Save the project",  // Tooltip
										"Private",  // Tooltip private info
										save_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_file_save),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Quit button
	quit_widget = gtk_image_new_from_stock(GTK_STOCK_QUIT, icon_height);
	quit_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Quit",  // Button label
										"Quit the application",  // Tooltip
										"Private",  // Tooltip private info
										quit_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(quit_event),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

	// Create the Capture button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/capture.svg", -1, icon_height, NULL);
	capture_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	capture_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Capture",  // Button label
										"Capture screenshots",  // Tooltip
										"Private",  // Tooltip private info
										capture_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_screenshots_capture),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Import button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/import.svg", -1, icon_height, NULL);
	import_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	import_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Import",  // Button label
										"Import screenshots",  // Tooltip
										"Private",  // Tooltip private info
										import_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(menu_screenshots_import),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Crop button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/crop.svg", -1, icon_height, NULL);
	crop_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	crop_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
										"Crop",  // Button label
										"Crop the project",  // Tooltip
										"Private",  // Tooltip private info
										crop_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(project_crop),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

	// Create the Export Flash button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/export_flash.svg", -1, icon_height, NULL);
	export_flash_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	export_flash_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
												"Flash",  // Button label
												"Export as a Flash animation",  // Tooltip
												"Private",  // Tooltip private info
												export_flash_widget,  // Image/icon widget
												GTK_SIGNAL_FUNC(menu_export_flash_animation),  // Function to call when clicked
												NULL);  // Our function doesn't need any data passed to it

	// Create the Export SVG button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/export_svg.svg", -1, icon_height, NULL);
	export_svg_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	export_svg_button = gtk_toolbar_append_item(GTK_TOOLBAR(inner_toolbar),
												"SVG",  // Button label
												"Export as an SVG animation",  // Tooltip
												"Private",  // Tooltip private info
												export_svg_widget,  // Image/icon widget
												GTK_SIGNAL_FUNC(menu_export_svg_animation),  // Function to call when clicked
												NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(inner_toolbar));

	return inner_toolbar;	
}


// Function to create the film strip area
void create_film_strip()
{
	// Purpose: To create the left film strip area
	//
	// Inputs: None
	//
	// Returns: None
	//
	// Important variables:
	//							film_strip
	//							film_strip_container
	//
	// Example:
	//
	//		GtkWidget 			*film_strip;
	//		GtkScrolledWindow	*film_strip_container;
	//
	//		create_film_strip();
	//

	// Create the film strip top widget
	film_strip_container = GTK_SCROLLED_WINDOW(gtk_scrolled_window_new(NULL, NULL));

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(film_strip_container), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	// * Create a VBox for storing the film strip thumbnails *
	film_strip = gtk_vbox_new(FALSE, 2);

	// Check if the film strip widget already contains a vbox.  If so, destroy the old one
//	container_children = gtk_container_get_children(GTK_CONTAINER(film_strip_container));
//	if (NULL != container_children)
//	{
//		// The film strip has an existing vbox in it, so destroy that one first
//		gtk_widget_destroy(container_children->data);
//	}

	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(film_strip_container), film_strip);
}


// Function to create the time line area
GtkWidget *create_time_line(void)
{
	// Purpose: To create the time line area
	//
	// Inputs: none
	//
	// Returns: A pointer to the time line GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		create_time_line();
	//

	// Local variables
	GtkWidget		*time_line_toolbar;						// Widget for holding the time line toolbar
	GtkWidget		*time_line_scrolled_window;				// Widget for holding the scrolled window

	// Toolbar variables
	GtkWidget		*crop_widget, *crop_button;			// Crop layer widgets
	GtkWidget		*delete_widget, *delete_button;			// Delete layer widgets
	GtkWidget		*edit_widget, *edit_button;				// Edit layer widgets
	GtkWidget		*highlight_widget, *highlight_button;	// Highlight layer widgets
	GtkWidget		*image_widget, *image_button;			// Highlight layer widgets
	GtkWidget		*move_down_widget, *move_down_button;	// Move layer down widgets
	GtkWidget		*move_up_widget, *move_up_button;		// Move layer up widgets
	GtkWidget		*text_widget, *text_button;				// Text layer widgets

	GdkPixbuf		*tmp_gdk_pixbuf;							// Temporary GDK Pixbuf


	// Create the VBox the time line elements are packed into
	time_line_vbox = gtk_vbox_new(FALSE, 0);

	// Create the scrolled window that the time line widgets are swapped into and out of
	time_line_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(time_line_scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(time_line_vbox), GTK_WIDGET(time_line_scrolled_window), TRUE, TRUE, 0);
	time_line_container = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(time_line_scrolled_window), GTK_WIDGET(time_line_container));

	// Create the time line toolbar
	time_line_toolbar = gtk_toolbar_new();
	gtk_box_pack_start(GTK_BOX(time_line_vbox), GTK_WIDGET(time_line_toolbar), FALSE, FALSE, 0);

	// * Create the layer toolbar icons *

	// Create the Edit Layer button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/edit.svg", -1, icon_height, NULL);
	edit_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	edit_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Edit",  // Button label
										"Edit layer",  // Tooltip
										"Private",  // Tooltip private info
										edit_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_edit),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Crop button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/crop.svg", -1, icon_height, NULL);
	crop_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	crop_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Crop",  // Button label
										"Crop image",  // Tooltip
										"Private",  // Tooltip private info
										crop_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(image_crop),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Delete layer button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/delete.svg", -1, icon_height, NULL);
	delete_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	delete_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Delete",  // Button label
										"Delete selected layer",  // Tooltip
										"Private",  // Tooltip private info
										delete_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_delete),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Move Layer Down button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/down_arrow.svg", -1, icon_height, NULL);
	move_down_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	move_down_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Down",  // Button label
										"Move layer down",  // Tooltip
										"Private",  // Tooltip private info
										move_down_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_move_down),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the Move Layer Up button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/up_arrow.svg", -1, icon_height, NULL);
	move_up_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	move_up_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Up",  // Button label
										"Move layer up",  // Tooltip
										"Private",  // Tooltip private info
										move_up_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_move_up),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Add a spacer to the toolbar
	gtk_toolbar_append_space(GTK_TOOLBAR(time_line_toolbar));

	// Create the add text layer button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/add_text.svg", -1, icon_height, NULL);
	text_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	text_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Text",  // Button label
										"Add a text layer",  // Tooltip
										"Private",  // Tooltip private info
										text_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_text),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the add highlight layer button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/add_highlight.svg", -1, icon_height, NULL);
	highlight_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	highlight_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Highlight",  // Button label
										"Add a highlight layer",  // Tooltip
										"Private",  // Tooltip private info
										highlight_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_highlight),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Create the add image layer button
	tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size("images/add_image.svg", -1, icon_height, NULL);
	image_widget = gtk_image_new_from_pixbuf(tmp_gdk_pixbuf);
	g_object_unref(tmp_gdk_pixbuf);
	image_button = gtk_toolbar_append_item(GTK_TOOLBAR(time_line_toolbar),
										"Image",  // Button label
										"Add an image layer",  // Tooltip
										"Private",  // Tooltip private info
										image_widget,  // Image/icon widget
										GTK_SIGNAL_FUNC(layer_new_image),  // Function to call when clicked
										NULL);  // Our function doesn't need any data passed to it

	// Return the handle of the time line container
	return time_line_vbox;
}


// Function to create the working area
GtkWidget *create_working_area(GtkWidget *working_frame)
{
	// Purpose: To create the working area
	//
	// Inputs: Takes a pointer to an empty GtkWidget
	//
	// Returns: A pointer to the working area GtkWidget (fully populated with items) or NULL on error
	//
	// Example:
	//
	//		GtkWidget *working;
	//		working = create_working_area(working);
	//

	// Create the top working area widget
	// fixme3: How about we remove the scroll bars and use Alt+mouse to drag/move instead?
	working_frame = gtk_scrolled_window_new(NULL, NULL);

	// Set the scroll bar settings
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(working_frame), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	// Create an aspect frame inside the working area's scrolled window
	working = gtk_aspect_frame_new(NULL, 0.5, 0.5, 1, TRUE);
	gtk_frame_set_shadow_type(GTK_FRAME(working), GTK_SHADOW_NONE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(working_frame), GTK_WIDGET(working));

	// Create the drawing area inside the aspect frame
	main_drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(working), GTK_WIDGET(main_drawing_area));

	// Hook up the signal handlers to the working area
	g_signal_connect(main_drawing_area, "button_press_event", G_CALLBACK(working_area_button_press_event), NULL);
	g_signal_connect(main_drawing_area, "button_release_event", G_CALLBACK(working_area_button_release_event), NULL);
	g_signal_connect(main_drawing_area, "expose_event", G_CALLBACK(working_area_expose_event), NULL);
	g_signal_connect(main_drawing_area, "motion_notify_event", G_CALLBACK(working_area_motion_notify_event), NULL);

	// Ensure we get the signals we want
	gtk_widget_set_events(main_drawing_area, gtk_widget_get_events(main_drawing_area)
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_BUTTON1_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK);

	// Display the working area
	gtk_widget_show_all(GTK_WIDGET(working_frame));

	return working_frame;
}


// The main program loop
gint main(gint argc, gchar *argv[])
{
	// Local variables
	GString				*command_key;			// Used to work out paths into the GConf structure
	GError				*error = NULL;			// Pointer to error return structure
	GConfEngine			*gconf_engine;			// GConf engine
	gboolean				key_already_set = FALSE;// Used to work out which metacity run command is unassigned
	GtkWidget			*main_area;				// Widget for the onscreen display
	GtkTable				*message_bar;			// Widget for message bar
	GtkWidget			*outer_box;				// Widget for the onscreen display
	GtkLabel				*resolution_label;		// Widget for the resolution selector label
	gboolean				should_maximise = FALSE;	// Briefly keeps track of whether the window should be maximised
	GtkWidget			*toolbar = NULL;			// Widget for the toolbar
	guint				unused_num = 0;			// Used to work out which metacity run command is unassigned
	GdkScreen			*which_screen;			// Gets given the screen the monitor is on
	gchar				wintitle[40];			// Stores the window title
	GtkLabel				*zoom_label;				// Widget for the zoom selector label

	gboolean				tmp_boolean;				// Temporary boolean
	guint				tmp_guint;				// Temporary guint
	guint				tmp_int;					// Temporary guint
	GString				*tmp_gstring;			// Temporary GString
	GtkWidget			*tmp_widget;				// Temporary widget


	// Initialise various things
	default_output_folder = g_string_new(NULL);
	default_project_folder = g_string_new(NULL);
	output_folder = g_string_new(NULL);
	project_folder = g_string_new(NULL);
	project_name = g_string_new(NULL);
	screenshots_folder = g_string_new(NULL);
	tmp_gstring = g_string_new(NULL);
	default_bg_colour.red = 0;
	default_bg_colour.green = 0;
	default_bg_colour.blue = 0;
	frames_per_second = 12;  // Half of 24 fps (film)

	// Initialise sound
	gnome_sound_init(NULL);

	// Check if we have a saved configuration in GConf
	gconf_engine = gconf_engine_get_default();
	tmp_boolean = gconf_engine_dir_exists(gconf_engine, "/apps/flame", &error);
	if (TRUE == tmp_boolean)
	{
		// Load the GConf configuration
		g_string_printf(default_project_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/project_folder", NULL));
		g_string_printf(screenshots_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/screenshots_folder", NULL));
		g_string_printf(default_output_folder, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/output_folder", NULL));
		g_string_printf(project_name, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/project_name", NULL));
		project_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_width", NULL);
		project_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/project_height", NULL);
		default_output_width = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_width", NULL);
		default_output_height = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_height", NULL);
		default_slide_length = slide_length = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/slide_length", NULL);
		default_output_quality = output_quality = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/output_quality", NULL);
		frames_per_second = gconf_engine_get_int(gconf_engine, "/apps/flame/defaults/frames_per_second", NULL);

		// Determine the stored scaling quality preference
		scaling_quality = GDK_INTERP_BILINEAR;  // A backup, just in case someone sets the GConf to an invalid value
		g_string_printf(tmp_gstring, "%s", gconf_engine_get_string(gconf_engine, "/apps/flame/defaults/scaling_quality", NULL));
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "N", 1))
		{
			// Scaling quality should be GDK_INTERP_NEAREST
			scaling_quality = GDK_INTERP_NEAREST;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "T", 1))
		{
			// Scaling quality should be GDK_INTERP_TILES
			scaling_quality = GDK_INTERP_TILES;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "B", 1))
		{
			// Scaling quality should be GDK_INTERP_BILINEAR
			scaling_quality = GDK_INTERP_BILINEAR;
		}
		if (0 == g_ascii_strncasecmp(tmp_gstring->str, "H", 1))
		{
			// Scaling quality should be GDK_INTERP_HYPER
			scaling_quality = GDK_INTERP_HYPER;
		}
		g_string_free(tmp_gstring, TRUE);

		// Check if the application should start maximised or not
		should_maximise = gconf_engine_get_bool(gconf_engine, "/apps/flame/defaults/window_maximised", NULL);
	} else
	{
		// Which monitor are we displaying on?
		which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

		// Initialise the application variables to sensible defaults
		g_string_printf(default_project_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "projects");
		g_string_printf(screenshots_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "screenshots");
		g_string_printf(default_output_folder, "%s%c%s", g_get_home_dir(), G_DIR_SEPARATOR, "output");
		g_string_printf(project_name, "%s", "New Project");
		project_width = gdk_screen_get_width(which_screen);
		project_height = gdk_screen_get_height(which_screen);
		default_output_width = 320;
		default_output_height = 240;
		default_slide_length = slide_length = 2;  // Default number of frames to use in new slides
		default_output_quality = output_quality = 9;  // Default quality to save png images with
		scaling_quality =  GDK_INTERP_TILES;  // Hyper is VERY, VERY slow with large high res images [GDK_INTERP_NEAREST | GDK_INTERP_TILES | GDK_INTERP_BILINEAR | GDK_INTERP_HYPER]
	}

	// Set various required defaults that will be overwritten by the first project loaded
	g_string_printf(project_folder, "%s", default_project_folder->str);
	g_string_printf(output_folder, "%s", default_output_folder->str);

	// * Setup the Control-Printscreen key to capture screenshots *

	// Search for the first unused run command
	command_key = g_string_new(NULL);
	for (tmp_guint = 10; tmp_guint >= 1; tmp_guint--)
	{
		// Create the name of the key to check
		g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", tmp_guint);

		// Get the value for the key
		g_string_printf(tmp_gstring, "%s", gconf_engine_get_string(gconf_engine, command_key->str, NULL));

		// Check if the key is unused
		tmp_int = g_ascii_strncasecmp(tmp_gstring->str, "", 1);
		if (0 == tmp_int)
		{
			// Yes it's unused, so make a note of it
			unused_num = tmp_guint;
		} else
		{
			// This command is being used, so check if it's already assigned to flame-capture
			tmp_int = g_ascii_strncasecmp(tmp_gstring->str, "flame-capture", 13);
			if (0 == tmp_int)
			{
				key_already_set = TRUE;
			}
		}
	}

	// If an unused run command was found and we haven't already assigned a screenshot key, then assign one
	if (FALSE == key_already_set)
	{
		if (0 != unused_num)
		{
			g_string_printf(command_key, "%s%u", "/apps/metacity/keybinding_commands/command_", unused_num);
			gconf_engine_set_string(gconf_engine, command_key->str, "flame-capture", NULL);
			g_string_printf(command_key, "%s%u", "/apps/metacity/global_keybindings/run_command_", unused_num);
			gconf_engine_set_string(gconf_engine, command_key->str, "<Control>Print", NULL);
		}
	}
	g_string_free(command_key, TRUE);

	// Free our GConf engine
	gconf_engine_unref(gconf_engine);

	// Start up the GUI part of things	
	gtk_init(&argc, &argv);
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 1024, 768);
	gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

	// Maximise the window if our saved configuration says to
	if (TRUE == should_maximise)
	{
		// Maximise the window
		gtk_window_maximize(GTK_WINDOW(main_window));
	}

	// Set the application title
	snprintf(wintitle, 40, "%s v%s\n", APP_NAME, APP_VERSION);

	// Set the window title and border
	gtk_window_set_title(GTK_WINDOW(main_window), wintitle);
	gtk_container_set_border_width(GTK_CONTAINER (main_window), 0);

	// Set a handler for destroy event to immediately exit GTK
	g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(quit_event), NULL);

	// Create the outermost container that everything (menu, main, status bar) fits into
	outer_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(outer_box));

	// * Create the menu *
	create_menu_bar();
	if (FALSE == menu_bar)
	{
		// Something went wrong when creating the menu bar
		g_error("Error ED01: Something went wrong when creating the menu bar");
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(gtk_item_factory_get_widget(menu_bar, "<main>")), FALSE, FALSE, 0);

	// * Create the toolbar *
	toolbar = create_toolbar(toolbar);
	if (FALSE == toolbar)
	{
		// Something went wrong when creating the toolbar
		g_error("Error ED07: Something went wrong when creating the toolbar");
		exit(1);
	}
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(toolbar), FALSE, FALSE, 0);

	// * Create a HPaned to pack the film strip into, and a vbox for the time line plus working area *
	main_area = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(main_area), preview_width + 20); // fixme3: Should read the handle-size of the HPaned (or something) rather than assume 10
	gtk_box_pack_start_defaults(GTK_BOX(outer_box), GTK_WIDGET(main_area));

	// * Create a table for the status bar, zoom selector, and resolution selectors to go in *
	// fixme4: Might be better going back to a HBox, and using filling (on for the status bar, off for the others), to achieve a better result
	message_bar = GTK_TABLE(gtk_table_new(1, 9, TRUE));
	gtk_box_pack_start(GTK_BOX(outer_box), GTK_WIDGET(message_bar), FALSE, FALSE, 0);

	// Create the status bar
	status_bar = gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(status_bar), FALSE);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(status_bar), 0, 5, 0, 1);

	// Create the zoom selector label
	zoom_label = GTK_LABEL(gtk_label_new("Zoom: "));
	gtk_misc_set_alignment(GTK_MISC(zoom_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_label), 5, 6, 0, 1);

	// Create the zoom selector
	zoom_selector = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(zoom_selector), FALSE);
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "1600%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "800%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "400%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "300%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "200%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "150%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "100%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "75%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "50%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "25%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "10%");
	gtk_combo_box_append_text(GTK_COMBO_BOX(zoom_selector), "Fit to width");
	gtk_combo_box_set_active(GTK_COMBO_BOX(zoom_selector), 11);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(zoom_selector), 6, 7, 0, 1);

	// Link the zoom selector to the function that recalculates the zoom and redraws the working area
	g_signal_connect(G_OBJECT(zoom_selector), "changed", G_CALLBACK(zoom_selector_changed), (gpointer) NULL);

	// Create the resolution selector label
	resolution_label = GTK_LABEL(gtk_label_new("Output: "));
	gtk_misc_set_alignment(GTK_MISC(resolution_label), 1, 0.5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_label), 7, 8, 0, 1);

	// Create the resolution selector
	resolution_selector = GTK_COMBO_BOX(gtk_combo_box_new_text());
	gtk_combo_box_set_focus_on_click(GTK_COMBO_BOX(resolution_selector), FALSE);
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1600x1200 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1280x1024 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "1024x768 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "800x600 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "640x480 px");
	gtk_combo_box_append_text(GTK_COMBO_BOX(resolution_selector), "320x240 px");
	gtk_combo_box_set_active(GTK_COMBO_BOX(resolution_selector), 5);
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 8, 9, 0, 1);

	// Link the resolution selector to the function that stores the new values in global variables
	g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);

	// * Create the film strip area *
	create_film_strip();
	if (FALSE == film_strip_container)
	{
		// Something went wrong when creating the film strip
		g_error("Error ED02: Something went wrong when creating the film strip");
		exit(2);
	}
	gtk_paned_add1(GTK_PANED(main_area), GTK_WIDGET(film_strip_container));

	// * Create the vertical box to pack the time line and working area into *
	right_side = gtk_vpaned_new();
	create_time_line();  // Create the time line
	tmp_widget = create_working_area(tmp_widget);  // Create the working area

	gtk_paned_add1(GTK_PANED(right_side), GTK_WIDGET(time_line_vbox));
	gtk_paned_add2(GTK_PANED(right_side), GTK_WIDGET(tmp_widget));
	gtk_paned_set_position(GTK_PANED(right_side), 250);
	gtk_paned_add2(GTK_PANED(main_area), GTK_WIDGET(right_side));

	// Create the general global status bar context
	statusbar_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(status_bar), "Status bar messages");

	// Display a "Ready" message in the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, " Ready");

	// Calculate the zoom and drawing area, and initialise the project dimensions
	zoom_selector_changed(GTK_WIDGET(zoom_selector), NULL, (gpointer) NULL);
	resolution_selector_changed(GTK_WIDGET(resolution_selector), NULL, (gpointer) NULL);

	// Catch when the window is resized, to automatically recalculate the zoom and redraw the drawing area
	g_signal_connect(G_OBJECT(right_side), "size-allocate", G_CALLBACK(event_size_allocate_received), (gpointer) NULL);

	// Things we need:
	//  + Menu bar at the top for standard items
	//    + Have this toggleable, to enable the saving of screen real estate
	//  + Time line (top?)
	//    + Output file frame rate selector (i.e. 15 fps)
	//    + Layers
	//  + Preview/working area (main?)
	//  + Icons to change working state (top? right?)
	//  + Key bindings to make common tasks efficient
	//    + One handed keyboard operation possible (very efficient)?
	//      + Use keyboard modifiers for right mouse button, etc?
	//    + Maya/SoftImage spacebar/overlay thing?
	//  + Support wheel mouse especially in some way down the track?
	//    + Maya style, whereby shift/ctrl/alt changes scroll wheel meaning?
	//  + Status bar of some sort
	//    + Have this toggleable, to enable the saving of screen real estate
	//  + Killer feature? 3D

	// Windows we'll need
	//  + Sprite/Object window
	//  + Layers

	// Basic layout:
	//  + Show slides on the left
	//  + Show selected slide in working area
	//  + Show time line up top
	//    + Layers
	//  + Status bar

	// Display the main window
	gtk_widget_show_all(main_window);

	gtk_main();

	// Exit
	exit(0);
}

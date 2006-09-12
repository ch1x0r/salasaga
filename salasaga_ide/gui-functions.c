/*
 * $Id$
 *
 * Flame Project: Source file for all non specific GUI related funtions
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

// Standard includes
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

// GTK includes
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
//#include <libxml/encoding.h>
//#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>
//#include <libxml/parser.h>

// Flame Edit includes
#include "flame-types.h"
#include "backend.h"
#include "callbacks.h"
#include "gui-functions.h"
#include "externs.h"


// Function that gets all of the layers for a slide, then creates a pixel buffer from them
GdkPixbuf *compress_layers(GList *which_slide, guint width, guint height)
{
	// Local variables
	GdkPixbuf			*bg_pixbuf;				// Points to the background layer
	GdkPixbuf			*backing_pixbuf;		// Pixbuf buffer things draw onto
	layer				*layer_data;			// Pointer to the layer data
	GList				*layer_pointer;			// Pointer to the layer GList
	GdkPixbuf			*scaled_pixbuf;			// Smaller pixbuf
	slide				*slide_ref;				// Pointer to the slide data


	// Simplify various pointers
	slide_ref = (slide *) which_slide->data;
	layer_pointer = g_list_last(slide_ref->layers);  // The background layer is always the last (bottom) layer
	layer_data = layer_pointer->data;

	// Check if this is an empty slide
	if (TYPE_EMPTY == layer_data->object_type)
	{
		// This is an empty layer
		bg_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, project_width, project_height);
		gdk_pixbuf_fill(bg_pixbuf, ((((layer_empty *) layer_data->object_data)->bg_color.red / 255) << 24)
			+ ((((layer_empty *) layer_data->object_data)->bg_color.green / 255) << 16)
			+ ((((layer_empty *) layer_data->object_data)->bg_color.blue / 255) << 8) + 0xff);
	} else
	{
		bg_pixbuf = GDK_PIXBUF((GObject *) ((layer_image *) layer_data->object_data)->image_data);
	}

	// Copy the background image onto the drawing buffer (backing_pixbuf)
	backing_pixbuf = gdk_pixbuf_add_alpha(bg_pixbuf, FALSE, 0, 0, 0);

	// Process each layer's data in turn.  We do the reverse() twice so things are drawn in visually correct order
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_reverse(layer_pointer);
	g_list_foreach(layer_pointer, compress_layers_inner, backing_pixbuf);
	layer_pointer = g_list_reverse(layer_pointer);

	// Scale the composited pixbuf down to the desired size
	scaled_pixbuf = gdk_pixbuf_scale_simple(backing_pixbuf, width, height, scaling_quality);

	// Free the various pixbufs, pixmaps, etc
	g_object_unref(backing_pixbuf);
	if (TYPE_EMPTY == layer_data->object_type)
	{
		g_object_unref(bg_pixbuf);
	}

	return scaled_pixbuf;
}


// Function to process the data for a layer, adding it to the main image being built
void compress_layers_inner(gpointer element, gpointer user_data)
{
	// Local variables
	GdkGC				*graphics_context;		// GDK graphics context
	GdkScreen			*output_screen;			//
	layer				*layer_pointer;			// Points to the data for this layer

	PangoContext		*pango_context;			// Pango context used for text rendering
	PangoFontDescription	*font_description;	// Pango font description used for text rendering
	PangoLayout			*pango_layout;			// Pango layout used for text rendering
	gint				pango_height;			// Height of the Pango layout
	gint				pango_width;			// Width of the Pango layout

	GtkTextIter			text_start;				// The start position of the text buffer
	GtkTextIter			text_end;				// The end position of the text buffer

	gint				pixbuf_width;			// Width of the backing pixbuf
	gint				pixbuf_height;			// Height of the backing pixbuf
	gint				x_offset;				//
	gint				y_offset;				//
	gint				width;					//
	gint				height;					//

	GdkColormap			*tmp_colormap;			// Temporary colormap
	gint				tmp_int;				// Temporary integer
	GdkPixbuf			*tmp_pixbuf;			// GDK Pixbuf
	GdkPixmap			*tmp_pixmap;			// GDK Pixmap

// fixme4:  Stuff not present out-of-the-box for Solaris
#ifndef __sun
	PangoMatrix			pango_matrix = PANGO_MATRIX_INIT;  // Required for positioning the pango layout
	PangoRenderer		*pango_renderer;		// Pango renderer
#endif


	// Initialise various pointers
	layer_pointer = element;
	tmp_pixbuf = user_data;  // The backing pixbuf

	// Determine the type of layer we're dealing with (we ignore background layers)
	switch (layer_pointer->object_type)
	{
		case TYPE_GDK_PIXBUF:
			// * Image *

			// Determine if we're processing the background image (which we ignore) or an image layer
			tmp_int = g_ascii_strncasecmp(layer_pointer->name->str, "Background", 10);
			if (0 == tmp_int)
			{
				// We're processing a background layer, so return
				return;
			}

			// * We're processing an image layer *

			// Calculate how much of the source image will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_image *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_image *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_image *) layer_pointer->object_data)->width;
			height = ((layer_image *) layer_pointer->object_data)->height;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the image onto the backing pixbuf
			gdk_pixbuf_composite(((layer_image *) layer_pointer->object_data)->image_data,	// Source pixbuf
			tmp_pixbuf,				// Destination pixbuf
			x_offset,				// X offset
			y_offset,				// Y offset
			width,					// Width
			height,					// Height
			x_offset,				// Source offsets
			y_offset,				// Source offsets
			1, 1,					// Scale factor (1 == no scale)
			GDK_INTERP_NEAREST,		// Scaling type
			255);					// Alpha
			user_data = (GdkPixbuf *) tmp_pixbuf;

			// All done
			return;

		case TYPE_MOUSE_CURSOR:
			// * Composite the mouse pointer image onto the backing pixmap *

			// Calculate how much of the source image will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_mouse *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_mouse *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_mouse *) layer_pointer->object_data)->width;
			height = ((layer_mouse *) layer_pointer->object_data)->height;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the mouse pointer onto the backing pixbuf
			gdk_pixbuf_composite(mouse_ptr_pixbuf,	// Source pixbuf
			tmp_pixbuf,				// Destination pixbuf
			x_offset,				// X offset
			y_offset,				// Y offset
			width,					// Width
			height,					// Height
			x_offset,				// Source offsets
			y_offset,				// Source offsets
			1, 1,					// Scale factor (1 == no scale)
			GDK_INTERP_NEAREST,		// Scaling type
			255);					// Alpha
			user_data = (GdkPixbuf *) tmp_pixbuf;
			return;

		case TYPE_EMPTY:
			// Empty layer, just return
			return;

		case TYPE_TEXT:
			// * Draw the text layer *

			// Prepare the text
			pango_context = gdk_pango_context_get();
			pango_layout = pango_layout_new(pango_context);
			font_description = pango_context_get_font_description(pango_context);
			pango_font_description_set_size(font_description,
				PANGO_SCALE * ((layer_text *) layer_pointer->object_data)->font_size);
			pango_font_description_set_family(font_description, "Bitstream Vera Sans");  // "Sans", "Serif", "Monospace"
			pango_font_description_set_style(font_description, PANGO_STYLE_NORMAL);
			pango_font_description_set_variant(font_description, PANGO_VARIANT_NORMAL);
			pango_font_description_set_weight(font_description, PANGO_WEIGHT_NORMAL);
			pango_font_description_set_stretch(font_description, PANGO_STRETCH_NORMAL);
			gtk_text_buffer_get_bounds(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end);
			pango_layout_set_markup(pango_layout,
				gtk_text_buffer_get_text(((layer_text *) layer_pointer->object_data)->text_buffer, &text_start, &text_end, FALSE),  // Point to the text in the object buffer
				-1);	  // Auto calculate string length

			// * Draw a background for the text layer *

			// Work out how big the rendered text will be
			pango_layout_get_size(pango_layout, &pango_width, &pango_height);

			// Calculate how much of the background will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_text *) layer_pointer->object_data)->x_offset_start - 10;
			y_offset = ((layer_text *) layer_pointer->object_data)->y_offset_start - 2;
			width = (pango_width / PANGO_SCALE) + 20;
			height = (pango_height / PANGO_SCALE) + 4;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset - 1;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset - 1;
			}
			if (x_offset < 0)
			{
				x_offset = 0;
			}
			if (y_offset < 0)
			{
				y_offset = 0;
			}

			// Store the calculated width and height so we can do collision detection in the event handler
			((layer_text *) layer_pointer->object_data)->rendered_width = width;
			((layer_text *) layer_pointer->object_data)->rendered_height = height;

			// Draw the highlight box (or as much as will fit)
			draw_highlight_box(tmp_pixbuf, x_offset, y_offset, width, height,
			0xFFFFCCFF,							// Fill color - light yellow
			0x000000FF);							// Border color - black

			// Turn the pixbuf into a pixmap
			tmp_colormap = gdk_colormap_get_system();
			tmp_pixmap = gdk_pixmap_new(NULL, gdk_pixbuf_get_width(GDK_PIXBUF(tmp_pixbuf)), gdk_pixbuf_get_height(GDK_PIXBUF(tmp_pixbuf)), tmp_colormap->visual->depth);
			gdk_drawable_set_colormap(GDK_DRAWABLE(tmp_pixmap), GDK_COLORMAP(tmp_colormap));
			gdk_draw_pixbuf(GDK_PIXMAP(tmp_pixmap), NULL, GDK_PIXBUF(tmp_pixbuf), 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
			graphics_context = gdk_gc_new(GDK_PIXMAP(tmp_pixmap));

// fixme4: Solaris 10 doesn't have many Pango functions.  Will need to re-code this stuff
#ifndef __sun
			// Set the color of the text
			output_screen = gdk_drawable_get_screen(GDK_DRAWABLE(tmp_pixmap));
			pango_renderer = gdk_pango_renderer_get_default(GDK_SCREEN(output_screen));
			gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(pango_renderer), GDK_DRAWABLE(tmp_pixmap));
			gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(pango_renderer), graphics_context);
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, &(((layer_text *) layer_pointer->object_data)->text_color));

			// Position the text
			pango_matrix_translate(&pango_matrix,
				((layer_text *) layer_pointer->object_data)->x_offset_start,
				((layer_text *) layer_pointer->object_data)->y_offset_start);
			pango_context_set_matrix(pango_context, &pango_matrix);

			// Render the text onto the pixmap
			pango_renderer_draw_layout(PANGO_RENDERER(pango_renderer), PANGO_LAYOUT(pango_layout), 0, 0);

			// Unset the renderer, as its shared
			gdk_pango_renderer_set_override_color(GDK_PANGO_RENDERER(pango_renderer), PANGO_RENDER_PART_FOREGROUND, NULL);
			gdk_pango_renderer_set_drawable(GDK_PANGO_RENDERER(pango_renderer), NULL);
			gdk_pango_renderer_set_gc(GDK_PANGO_RENDERER(pango_renderer), NULL);

#endif

			// Copy the pixmap back onto the backing pixbuf
			gdk_pixbuf_get_from_drawable(GDK_PIXBUF(tmp_pixbuf), GDK_PIXMAP(tmp_pixmap), NULL, 0, 0, 0, 0, -1, -1);	
			user_data = GDK_PIXBUF(tmp_pixbuf);

			// Free the memory allocated but no longer needed
			g_object_unref(pango_layout);
			g_object_unref(graphics_context);
			g_object_unref(tmp_pixmap);

			break;

		case TYPE_HIGHLIGHT:
			// * Draw the highlight *

			// 0x00FF0040 : Good fill color
			// 0x00FF00CC : Good border color

			// Calculate how much of the highlight will fit onto the backing pixmap
			pixbuf_width = gdk_pixbuf_get_width(tmp_pixbuf);
			pixbuf_height = gdk_pixbuf_get_height(tmp_pixbuf);
			x_offset = ((layer_highlight *) layer_pointer->object_data)->x_offset_start;
			y_offset = ((layer_highlight *) layer_pointer->object_data)->y_offset_start;
			width = ((layer_highlight *) layer_pointer->object_data)->width;
			height = ((layer_highlight *) layer_pointer->object_data)->height;
			if ((x_offset + width) > pixbuf_width)
			{
				width = pixbuf_width - x_offset;
			}
			if ((y_offset + height) > pixbuf_height)
			{
				height = pixbuf_height - y_offset;
			}

			// Draw the highlight box (or as much as will fit)
			draw_highlight_box(tmp_pixbuf, x_offset, y_offset, width, height,
			0x00FF0040,		// Fill color
			0x00FF00CC);		// Border color

			break;		

		default:
			display_warning("ED33: Unknown layer type\n");		
	}
}


// Construct the widget used to display the slide in the timeline
GtkWidget *construct_timeline_widget(slide *slide_data)
{
	// Local variables
	GtkCellRenderer		*timeline_renderer_text_name;
	GtkCellRenderer		*timeline_renderer_toggle;
	GtkCellRenderer		*timeline_renderer_text_start;
	GtkCellRenderer		*timeline_renderer_text_finish;
	GtkCellRenderer		*timeline_renderer_text_x_offset_start;
	GtkCellRenderer		*timeline_renderer_text_y_offset_start;
	GtkCellRenderer		*timeline_renderer_text_x_offset_finish;
	GtkCellRenderer		*timeline_renderer_text_y_offset_finish;
	GtkTreeViewColumn	*timeline_column_name;
	GtkTreeViewColumn	*timeline_column_visibility;
	GtkTreeViewColumn	*timeline_column_start;
	GtkTreeViewColumn	*timeline_column_finish;
	GtkTreeViewColumn	*timeline_column_x_off_start;
	GtkTreeViewColumn	*timeline_column_y_off_start;
	GtkTreeViewColumn	*timeline_column_x_off_finish;
	GtkTreeViewColumn	*timeline_column_y_off_finish;

	GtkTreePath			*tmp_path;					// Temporary GtkPath


	// * This function assumes the existing timeline widget has not been created, or has been destroyed *

	// Create the timeline widget for this slide
	slide_data->timeline_widget = gtk_tree_view_new_with_model(GTK_TREE_MODEL(slide_data->layer_store));

	// Create cell renderers
	timeline_renderer_text_name = gtk_cell_renderer_text_new();
	timeline_renderer_text_start = gtk_cell_renderer_text_new();
	timeline_renderer_text_finish = gtk_cell_renderer_text_new();
	timeline_renderer_text_x_offset_start = gtk_cell_renderer_text_new();
	timeline_renderer_text_y_offset_start = gtk_cell_renderer_text_new();
	timeline_renderer_text_x_offset_finish = gtk_cell_renderer_text_new();
	timeline_renderer_text_y_offset_finish = gtk_cell_renderer_text_new();
	timeline_renderer_toggle = gtk_cell_renderer_toggle_new();
	gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(timeline_renderer_toggle), FALSE);

	// Hook a signal handler to the name column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_name), "edited", G_CALLBACK(timeline_edited_name), NULL);

	// Hook a signal handler to the start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_start), "edited", G_CALLBACK(timeline_edited_start), NULL);

	// Hook a signal handler to the finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_finish), "edited", G_CALLBACK(timeline_edited_finish), NULL);

	// Hook a signal handler to the x offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_start), "edited", G_CALLBACK(timeline_edited_x_offset_start), NULL);

	// Hook a signal handler to the y offset start column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_start), "edited", G_CALLBACK(timeline_edited_y_offset_start), NULL);

	// Hook a signal handler to the x offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_x_offset_finish), "edited", G_CALLBACK(timeline_edited_x_offset_finish), NULL);

	// Hook a signal handler to the y offset finish column renderer
	g_signal_connect(G_OBJECT(timeline_renderer_text_y_offset_finish), "edited", G_CALLBACK(timeline_edited_y_offset_finish), NULL);

	// Create name column
	timeline_column_name = gtk_tree_view_column_new_with_attributes("Name", timeline_renderer_text_name, "text", TIMELINE_NAME, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_name);

	// Create visibility column
	timeline_column_visibility = gtk_tree_view_column_new_with_attributes("Visible", timeline_renderer_toggle, "active", TIMELINE_VISIBILITY, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_visibility);

	// Create start column
	timeline_column_start = gtk_tree_view_column_new_with_attributes("Start", timeline_renderer_text_start, "text", TIMELINE_START, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_start);

	// Create finish column
	timeline_column_finish = gtk_tree_view_column_new_with_attributes("Finish", timeline_renderer_text_finish, "text", TIMELINE_FINISH, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_finish);

	// Create X offset start column
	timeline_column_x_off_start = gtk_tree_view_column_new_with_attributes("Start X", timeline_renderer_text_x_offset_start, "text", TIMELINE_X_OFF_START, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_x_off_start);

	// Create Y offset start column
	timeline_column_y_off_start = gtk_tree_view_column_new_with_attributes("Start Y", timeline_renderer_text_y_offset_start, "text", TIMELINE_Y_OFF_START, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_y_off_start);

	// Create X offset finish column
	timeline_column_x_off_finish = gtk_tree_view_column_new_with_attributes("Finish X", timeline_renderer_text_x_offset_finish, "text", TIMELINE_X_OFF_FINISH, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_x_off_finish);

	// Create Y offset finish column
	timeline_column_y_off_finish = gtk_tree_view_column_new_with_attributes("Finish Y", timeline_renderer_text_y_offset_finish, "text", TIMELINE_Y_OFF_FINISH, "editable", TRUE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(slide_data->timeline_widget), timeline_column_y_off_finish);

	// Select the top row in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(slide_data->timeline_widget), tmp_path, NULL, FALSE);

	return slide_data->timeline_widget;
}


// Function to create the output resolution selector
GtkWidget *create_resolution_selector(ResolutionStructure *res_array, guint num_resolutions, guint initial_width, guint initial_height)
{
	// Local variables
	gboolean			match_found;			// Simple toggle used to indiate the requested resolution has been found
	gint				match_at;				// If a resolution match was found, this contains where
	guint				res_counter;			// Used as a simple counter
	GString				*res_string;			// Used for constructing resolution strings
	GtkWidget			*res_widget;			// ComboBox widget that gets returned


	// Initialise various things
	match_found = FALSE;
	match_at = -1;
	res_string = g_string_new(NULL);
	res_widget = gtk_combo_box_new_text();

	// Check if the requested resolution is present in the resolution array
	for (res_counter = 0; res_counter < num_resolutions; res_counter++)
	{
		if ((res_array + res_counter)->width == initial_width)
		{
			// There's a match on width, so check for a match on height
			if ((res_array + res_counter)->height == initial_height)
			{
				match_found = TRUE;
				match_at = res_counter;
			}
		}
	}

	// Construct the GtkComboBox from the supplied list of resolutions
	for (res_counter = 0; res_counter < num_resolutions; res_counter++)
	{
		g_string_printf(res_string, "%ux%u px", (res_array + res_counter)->width, (res_array + res_counter)->height);
		gtk_combo_box_append_text(GTK_COMBO_BOX(res_widget), res_string->str);
	}

	// If no match was found we add the requested resolution on to the end of the list
	if (TRUE != match_found)
	{
		g_string_printf(res_string, "%ux%u px", initial_width, initial_height);
		gtk_combo_box_append_text(GTK_COMBO_BOX(res_widget), res_string->str);
		match_at = num_resolutions;  // Point to the new entry
	}

	// Select the chosen resolution as the default
	gtk_combo_box_set_active(GTK_COMBO_BOX(res_widget), match_at);

	// Free memory allocated during this function
	g_string_free(res_string, TRUE);

	// Return the resolution widget
	return res_widget;
}


// Recreate the tooltips for the slides
void create_tooltips(void)
{
	// Local variables
	guint				counter;				// Used as a standard counter
	GString				*name_string;			// Used for generating a name string
	guint				num_slides;				// Number of slides in the whole slide list
	slide				*slide_data;			// Points to the actual data in the given slide


	// Initialise some things
	name_string = g_string_new(NULL);
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);

	// Loop through each slide, creating an appropriate tooltip for it
	for (counter = 0; counter < num_slides; counter++)
	{
		slide_data = g_list_nth_data(slides, counter);
		slide_data->tooltip = gtk_tooltips_new();
		if (NULL == slide_data->name)
		{
			// The user hasn't given a name to the slide, so use a default
			g_string_printf(name_string, "Slide %u", counter + 1);
			gtk_tooltips_set_tip(slide_data->tooltip, GTK_WIDGET(slide_data->event_box), name_string->str, NULL);
		} else
		{
			// The user has named the slide, so use the name
			gtk_tooltips_set_tip(slide_data->tooltip, GTK_WIDGET(slide_data->event_box), slide_data->name->str, NULL);
		}
	}
}


// Disables the layer toolbar buttons
void disable_layer_toolbar_buttons(void)
{
	// Disable the Edit Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_EDIT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_icons_gray[LAYER_EDIT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_tooltips, "Edit layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_EDIT]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_signals[LAYER_EDIT]);

	// Disable the Crop Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_CROP]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_CROP]), layer_toolbar_icons_gray[LAYER_CROP]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_CROP]), layer_toolbar_tooltips, "Crop layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_CROP]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_CROP]), layer_toolbar_signals[LAYER_CROP]);

	// Disable the Delete Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_DELETE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_icons_gray[LAYER_DELETE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_tooltips, "Delete layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DELETE]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_signals[LAYER_DELETE]);

	// Disable the Move Layer Down icon
	g_object_ref(layer_toolbar_icons[LAYER_DOWN]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_icons_gray[LAYER_DOWN]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_tooltips, "Move layer down disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DOWN]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_signals[LAYER_DOWN]);

	// Disable the Move Layer Up icon
	g_object_ref(layer_toolbar_icons[LAYER_UP]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_UP]), layer_toolbar_icons_gray[LAYER_UP]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_UP]), layer_toolbar_tooltips, "Move layer up disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_UP]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_UP]), layer_toolbar_signals[LAYER_UP]);

	// Disable the Add Mouse Pointer icon
	g_object_ref(layer_toolbar_icons[LAYER_MOUSE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_icons_gray[LAYER_MOUSE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_tooltips, "Add mouse pointer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_MOUSE]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_signals[LAYER_MOUSE]);

	// Disable the Add Text Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_TEXT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_icons_gray[LAYER_TEXT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_tooltips, "Add text layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_TEXT]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_signals[LAYER_TEXT]);

	// Disable the Add Highlight Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_HIGHLIGHT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_icons_gray[LAYER_HIGHLIGHT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_tooltips, "Add highlight layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_HIGHLIGHT]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_signals[LAYER_HIGHLIGHT]);

	// Disable the Add Image Layer icon
	g_object_ref(layer_toolbar_icons[LAYER_IMAGE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_icons_gray[LAYER_IMAGE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_tooltips, "Add image layer disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_IMAGE]));
	g_signal_handler_disconnect(G_OBJECT(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_signals[LAYER_IMAGE]);
}


// Disables the main toolbar buttons that can only be used when a project is loaded
void disable_main_toolbar_buttons(void)
{
	// Disable the Crop All icon
	g_object_ref(main_toolbar_icons[CROP_ALL]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[CROP_ALL]), main_toolbar_icons_gray[CROP_ALL]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CROP_ALL]), main_toolbar_tooltips, "Crop disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[CROP_ALL]));
	g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[CROP_ALL]), main_toolbar_signals[CROP_ALL]);

	// Disable the Export Flash icon
	g_object_ref(main_toolbar_icons[EXPORT_FLASH]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_FLASH]), main_toolbar_icons_gray[EXPORT_FLASH]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export to Flash disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_FLASH]));
	g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), main_toolbar_signals[EXPORT_FLASH]);

	// Disable the Export SVG icon
	g_object_ref(main_toolbar_icons[EXPORT_SVG]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_SVG]), main_toolbar_icons_gray[EXPORT_SVG]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_SVG]), main_toolbar_tooltips, "Export to SVG disabled: No project loaded", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_SVG]));
	g_signal_handler_disconnect(G_OBJECT(main_toolbar_items[EXPORT_SVG]), main_toolbar_signals[EXPORT_SVG]);
}


// Display a warning message to the user
gint display_warning(gchar *warning_string)
{
	// Local variables
	GtkWidget			*tmp_dialog;			// Temporary dialog box

	// Display the warning dialog
	tmp_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, warning_string);
	gtk_dialog_run(GTK_DIALOG(tmp_dialog));
	gtk_widget_destroy(tmp_dialog);

	// Send the warning to stdout as well
	g_warning(warning_string);

	return TRUE;
}


// Display a dialog box asking for highlight layer settings
gboolean display_dialog_highlight(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*highlight_dialog;		// Widget for the dialog
	GtkWidget			*highlight_table;		// Table used for neat layout of the dialog box
	gint				dialog_result;			// Catches the return code from the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to

	GtkWidget			*x_off_label_start;		// Label widget
	GtkWidget			*x_off_button_start;	//

	GtkWidget			*y_off_label_start;		// Label widget
	GtkWidget			*y_off_button_start;	//

	GtkWidget			*x_off_label_finish;	// Label widget
	GtkWidget			*x_off_button_finish;	//

	GtkWidget			*y_off_label_finish;	// Label widget
	GtkWidget			*y_off_button_finish;	//

	GtkWidget			*width_label;			// Label widget
	GtkWidget			*width_button;			//

	GtkWidget			*height_label;			// Label widget
	GtkWidget			*height_button;			//

	GtkWidget			*start_label;			// Label widget
	GtkWidget			*start_button;			//

	GtkWidget			*finish_label;			// Label widget
	GtkWidget			*finish_button;			//

	GtkWidget			*external_link_label;	// Label widget
	GtkWidget			*external_link_entry;	// Widget for accepting an external link for clicking on

	layer_highlight		*tmp_highlight_ob;		// Temporary highlight layer object


	// Initialise some things
	tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;

	// * Pop open a dialog box asking the user for the details of the new layer *

	// Create the dialog window, and table to hold its children
	highlight_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	highlight_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(highlight_dialog->vbox), GTK_WIDGET(highlight_table), FALSE, FALSE, 10);

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Starting X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_highlight_ob->x_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Starting Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_highlight_ob->y_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finishing X Offset:");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_highlight_ob->x_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finishing Y Offset:");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_highlight_ob->y_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the width
	width_label = gtk_label_new("Width:");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the width input
	width_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), tmp_highlight_ob->width);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the height
	height_label = gtk_label_new("Height:");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the height input
	height_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), tmp_highlight_ob->height);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach_defaults(GTK_TABLE(highlight_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(highlight_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(highlight_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(highlight_dialog));
		return FALSE;	
	}

	// Fill out the temporary layer with the requested details
	tmp_highlight_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_highlight_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_highlight_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_highlight_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_highlight_ob->width = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));
	tmp_highlight_ob->height = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(highlight_dialog));

	return TRUE;
}


// Display a dialog box asking for image layer settings
gboolean display_dialog_image(layer *tmp_layer, gchar *dialog_title, gboolean request_file)
{
	// Local variables
	GtkDialog			*image_dialog;			// Widget for the dialog
	GtkWidget			*image_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;			// Catches the return code from the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to

	GtkWidget			*path_widget;			// File selection widget
	GString				*path_gstring;			// Holds the file selection path

	GtkWidget			*image_label;			// Label widget
	GtkWidget			*image_entry;			//

	GtkWidget			*x_off_label_start;		// Label widget
	GtkWidget			*x_off_button_start;	//

	GtkWidget			*y_off_label_start;		// Label widget
	GtkWidget			*y_off_button_start;	//

	GtkWidget			*x_off_label_finish;	// Label widget
	GtkWidget			*x_off_button_finish;	//

	GtkWidget			*y_off_label_finish;	// Label widget
	GtkWidget			*y_off_button_finish;	//

	GtkWidget			*start_label;			// Label widget
	GtkWidget			*start_button;			//

	GtkWidget			*finish_label;			// Label widget
	GtkWidget			*finish_button;			//

	GtkWidget			*external_link_label;	// Label widget
	GtkWidget			*external_link_entry;	// Widget for accepting an external link for clicking on

	layer_image			*tmp_image_ob;			// Temporary layer object


	// Initialise some things
	path_gstring = g_string_new(NULL);
	tmp_image_ob = (layer_image *) tmp_layer->object_data;

	// If we're supposed to ask for a file, then do so
	if (TRUE == request_file)
	{
		// * Pop open a dialog allowing the user to select an image *
		path_widget = gtk_file_selection_new("Choose an image file");
		g_string_printf(path_gstring, "%s/", screenshots_folder->str);  //  Add a trailing slash to the folder name
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(path_widget), path_gstring->str);

		// Run the dialog
		gtk_widget_show_all(GTK_WIDGET(path_widget));
		dialog_result = gtk_dialog_run(GTK_DIALOG(path_widget));

		// Was the OK button pressed?
		if (GTK_RESPONSE_OK != dialog_result)  // Note this is different to GTK_RESPONSE_ACCEPT used normally
		{
			// * The user cancelled the dialog *

			// Destroy the dialog box
			gtk_widget_destroy(GTK_WIDGET(path_widget));

			// Free the memory allocated in this function
			g_string_free(path_gstring, TRUE);
			return FALSE;	
		}

		// Retrieve the selected image file path
		g_string_printf(path_gstring, "%s", gtk_file_selection_get_filename(GTK_FILE_SELECTION(path_widget)));

		// Destroy the file selection dialog box
		gtk_widget_destroy(GTK_WIDGET(path_widget));
	} else
	{
		// We've been instructed to not request a file, which only happens if we've already got one
		path_gstring = tmp_image_ob->image_path;
	}

	// * Pop open a dialog box asking the user for the details of the new layer *

	// Create the dialog window, and table to hold its children
	image_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	image_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(image_dialog->vbox), GTK_WIDGET(image_table), FALSE, FALSE, 10);

	// Create the label confirming the path to the image
	image_label = gtk_label_new("Image file:");
	gtk_misc_set_alignment(GTK_MISC(image_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(image_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry confirming the path to the image
	image_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(image_entry), 199);
	gtk_entry_set_text(GTK_ENTRY(image_entry), path_gstring->str);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(image_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_image_ob->x_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_image_ob->y_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_image_ob->x_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_image_ob->y_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach_defaults(GTK_TABLE(image_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(image_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(image_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(image_dialog));
		return FALSE;	
	}

	// fixme4: We should validate the user input here

	// Fill out the temporary layer with the requested details
	tmp_image_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_image_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_image_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_image_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	g_string_printf(path_gstring, "%s", gtk_entry_get_text(GTK_ENTRY(image_entry)));
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
	tmp_image_ob->image_path = path_gstring;

	// If we already have image data loaded, get rid of it
	if (TRUE != request_file)
	{
		g_object_unref(GDK_PIXBUF(tmp_image_ob->image_data));
	}

	tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_image_ob->image_path->str, NULL);  // Load the image
	tmp_image_ob->width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
	tmp_image_ob->height = gdk_pixbuf_get_height(tmp_image_ob->image_data);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(image_dialog));

	return TRUE;
}


// Display a dialog box asking for mouse pointer settings
gboolean display_dialog_mouse(layer *tmp_layer, gchar *dialog_title, gboolean request_file)
{
	// Local variables
	GtkDialog			*mouse_dialog;			// Widget for the dialog
	GtkWidget			*mouse_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;			// Catches the return code from the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to

	GtkWidget			*x_off_label_start;		// Label widget
	GtkWidget			*x_off_button_start;	//

	GtkWidget			*y_off_label_start;		// Label widget
	GtkWidget			*y_off_button_start;	//

	GtkWidget			*x_off_label_finish;	// Label widget
	GtkWidget			*x_off_button_finish;	//

	GtkWidget			*y_off_label_finish;	// Label widget
	GtkWidget			*y_off_button_finish;	//

	GtkWidget			*start_label;			// Label widget
	GtkWidget			*start_button;			//

	GtkWidget			*finish_label;			// Label widget
	GtkWidget			*finish_button;			//

	GtkWidget			*click_button;			// Label widget

	layer_mouse			*tmp_mouse_ob;			// Temporary layer object


	// Initialise some things
	tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;

	// * Pop open a dialog box asking the user for the details of the new layer *

	// Create the dialog window, and table to hold its children
	mouse_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	mouse_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(mouse_dialog->vbox), GTK_WIDGET(mouse_table), FALSE, FALSE, 10);

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_mouse_ob->x_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_mouse_ob->y_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_mouse_ob->x_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_mouse_ob->y_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the button asking if there should be a mouse click sound
	click_button = gtk_check_button_new_with_label("Include mouse click?");
	if (MOUSE_NONE == tmp_mouse_ob->click)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(click_button), FALSE);
	} else
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(click_button), TRUE);
	}
	gtk_table_attach_defaults(GTK_TABLE(mouse_table), GTK_WIDGET(click_button), 0, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(mouse_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(mouse_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(mouse_dialog));
		return FALSE;	
	}

	// fixme4: We should validate the user input here

	// Fill out the temporary layer with the requested details
	tmp_mouse_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_mouse_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_mouse_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_mouse_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	if (TRUE == gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(click_button)))
	{
		tmp_mouse_ob->click = MOUSE_LEFT_ONE;
	} else
	{
		tmp_mouse_ob->click = MOUSE_NONE;
	}

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(mouse_dialog));

	return TRUE;
}


// Display a dialog box asking for text layer settings
gboolean display_dialog_text(layer *tmp_layer, gchar *dialog_title)
{
	// Local variables
	GtkDialog			*text_dialog;			// Widget for the text dialog
	GtkWidget			*text_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;			// Catches the return code from the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to

	GtkWidget			*text_frame;			// Frame to go around the text widget
	GtkWidget			*text_view;				// Widget for accepting the new text data

	GtkWidget			*name_label;			// Label widget
	GtkWidget			*name_entry;			// Widget for accepting the name of the new layer

	GtkWidget			*color_label;			// Label widget
	GtkWidget			*color_button;			// Color selection button

	GtkWidget			*x_off_label_start;		// Label widget
	GtkWidget			*x_off_button_start;	//

	GtkWidget			*y_off_label_start;		// Label widget
	GtkWidget			*y_off_button_start;	//

	GtkWidget			*x_off_label_finish;	// Label widget
	GtkWidget			*x_off_button_finish;	//

	GtkWidget			*y_off_label_finish;	// Label widget
	GtkWidget			*y_off_button_finish;	//

	GtkWidget			*font_label;			// Label widget
	GtkWidget			*font_button;			//

	GtkWidget			*start_label;			// Label widget
	GtkWidget			*start_button;			//

	GtkWidget			*finish_label;			// Label widget
	GtkWidget			*finish_button;			//

	GtkWidget			*external_link_label;	// Label widget
	GtkWidget			*external_link_entry;	// Widget for accepting an external link for clicking on

	layer_text			*tmp_text_ob;			// Temporary text layer object


	// Initialise some things
	tmp_text_ob = (layer_text *) tmp_layer->object_data;

	// * Pop open a dialog box asking the user for the details of the new text layer *

	// Create the dialog window, and table to hold its children
	text_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons(dialog_title, GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	text_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(text_dialog->vbox), GTK_WIDGET(text_table), FALSE, FALSE, 0);

	// Create the text view that accepts the new text data
	text_frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(text_frame), 2);
	gtk_frame_set_shadow_type(GTK_FRAME(text_frame), GTK_SHADOW_OUT);
	text_view = gtk_text_view_new_with_buffer(tmp_text_ob->text_buffer);
	gtk_widget_set_size_request(GTK_WIDGET(text_view), 0, 100);
	gtk_container_add(GTK_CONTAINER(text_frame), text_view);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(text_frame), 0, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label next to the color swatch
	color_label = gtk_label_new("Color: ");
	gtk_misc_set_alignment(GTK_MISC(color_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(color_label), 0, 1, row_counter, row_counter + 1);

	// Create the color selection button
    color_button = gtk_color_button_new_with_color(&(tmp_text_ob->text_color));
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_button), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(color_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting X Offset
	x_off_label_start = gtk_label_new("Start X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(x_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting X Offset input
	x_off_button_start = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_start), tmp_text_ob->x_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(x_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting Y Offset
	y_off_label_start = gtk_label_new("Start Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_start), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(y_off_label_start), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting Y Offset input
	y_off_button_start = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_start), tmp_text_ob->y_offset_start);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(y_off_button_start), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing X Offset
	x_off_label_finish = gtk_label_new("Finish X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(x_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing X Offset input
	x_off_button_finish = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_off_button_finish), tmp_text_ob->x_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(x_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing Y Offset
	y_off_label_finish = gtk_label_new("Finish Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_off_label_finish), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(y_off_label_finish), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing Y Offset input
	y_off_button_finish = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_off_button_finish), tmp_text_ob->y_offset_finish);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(y_off_button_finish), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the font size
	font_label = gtk_label_new("Font size: ");
	gtk_misc_set_alignment(GTK_MISC(font_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(font_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the font size input
	font_button = gtk_spin_button_new_with_range(0, 200, 10);  // fixme5: 200 was just plucked from the air
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(font_button), tmp_text_ob->font_size);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(font_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the new layer name
	name_label = gtk_label_new("Layer name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_layer->name->str);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the starting frame
	start_label = gtk_label_new("Start frame: ");
	gtk_misc_set_alignment(GTK_MISC(start_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(start_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the starting frame input
	start_button = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(start_button), tmp_layer->start_frame);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(start_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the finishing frame
	finish_label = gtk_label_new("Finish frame: ");
	gtk_misc_set_alignment(GTK_MISC(finish_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(finish_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the finishing frame input
	finish_button = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(finish_button), tmp_layer->finish_frame);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(finish_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for an external link
	external_link_label = gtk_label_new("External link: ");
	gtk_misc_set_alignment(GTK_MISC(external_link_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(external_link_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts an external link
	external_link_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(external_link_entry), 50);
	gtk_entry_set_text(GTK_ENTRY(external_link_entry), tmp_layer->external_link->str);
	gtk_table_attach_defaults(GTK_TABLE(text_table), GTK_WIDGET(external_link_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(text_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(text_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box then return
		gtk_widget_destroy(GTK_WIDGET(text_dialog));
		return FALSE;
	}

	// fixme4: We should validate the user input (layer name) here

	// Updated the layer in memory with the requested details
	tmp_text_ob->x_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_start));
	tmp_text_ob->y_offset_start = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_start));
	tmp_text_ob->x_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_off_button_finish));
	tmp_text_ob->y_offset_finish = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_off_button_finish));
	tmp_text_ob->font_size = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(font_button));
	tmp_layer->start_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(start_button));
	tmp_layer->finish_frame = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(finish_button));
	g_string_printf(tmp_layer->name, "%s", gtk_entry_get_text(GTK_ENTRY(name_entry)));
	g_string_printf(tmp_layer->external_link, "%s", gtk_entry_get_text(GTK_ENTRY(external_link_entry)));
	gtk_color_button_get_color(GTK_COLOR_BUTTON(color_button), &(tmp_text_ob->text_color));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(text_dialog));

	// Let the caller know that things went ok
	return TRUE;
}


// Function to draw a bounding box in the drawing area, via the backing store
void draw_bounding_box(GtkWidget *widget, GdkRegion *region)
{
	// Local variables
	GdkPoint				point_array[5];				// Holds the boundary points of the lines to draw

	GdkRectangle			*tmp_rectangle_array[1];
	gint					tmp_gint;
	

	gdk_region_offset(region, 1, 2);
	gdk_region_get_rectangles(region, tmp_rectangle_array, &tmp_gint);

//g_printerr("Number of rectangles in region: %d\n", tmp_gint);	
//g_printerr("Rectangle X: %d\n", tmp_rectangle_array[0]->x);
//g_printerr("Rectangle Y: %d\n", tmp_rectangle_array[0]->y);
//g_printerr("Rectangle Width: %d\n", tmp_rectangle_array[0]->width);
//g_printerr("Rectangle Height: %d\n", tmp_rectangle_array[0]->height);

	// Fill in the values for the point array
	point_array[0].x = tmp_rectangle_array[0]->x;
	point_array[0].y = tmp_rectangle_array[0]->y;

	point_array[1].x = tmp_rectangle_array[0]->x + tmp_rectangle_array[0]->width - 1;
	point_array[1].y = tmp_rectangle_array[0]->y;

	point_array[2].x = tmp_rectangle_array[0]->x + tmp_rectangle_array[0]->width - 1;
	point_array[2].y = tmp_rectangle_array[0]->y + tmp_rectangle_array[0]->height - 1;

	point_array[3].x = tmp_rectangle_array[0]->x;
	point_array[3].y = tmp_rectangle_array[0]->y + tmp_rectangle_array[0]->height - 1;

	point_array[4].x = tmp_rectangle_array[0]->x;
	point_array[4].y = tmp_rectangle_array[0]->y;

//draw_workspace();

	// Draw the bounding line around the object
	gdk_draw_lines(backing_store, widget->style->black_gc, &point_array[0], 5);

	// Cause an expose event to happen, writing the (updated) backing store to the screen
	gdk_window_invalidate_rect(widget->window, tmp_rectangle_array[0], FALSE);

	// Free the memory allocated in this function
	g_free(tmp_rectangle_array[0]);
}


// Function to draw a highlight box on a GDK pixbuf
void draw_highlight_box(GdkPixbuf *tmp_pixbuf, gint x_offset, gint y_offset, gint width, gint height, guint32 fill_color, guint32 border_color)
{
	// Local variables
	GdkPixbuf				*highlight_pixbuf;	// GDK Pixbuf used for highlighting


	// Create a horizontal line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, 2);
	if (NULL == highlight_pixbuf)
	{
		display_warning("ED52: Not enough memory for pixbuf allocation");
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the backing store - top line
	gdk_pixbuf_composite(highlight_pixbuf,		// Source pixbuf
		tmp_pixbuf,								// Destination pixbuf
		x_offset,								// X offset
		y_offset,								// Y offset
		width,									// Width
		2,										// Height
		0, 0,									// Source offsets
		1, 1,									// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);									// Alpha

	// Composite the line onto the backing store - bottom line
	gdk_pixbuf_composite(highlight_pixbuf,		// Source pixbuf
		tmp_pixbuf,								// Destination pixbuf
		x_offset,								// X offset
		y_offset + height - 1,					// Y offset
		width,									// Width
		2,										// Height
		0, 0,									// Source offsets
		1, 1,									// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);									// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);

	// Create a vertical line
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 2, height);
	if (NULL == highlight_pixbuf)
	{
		display_warning("ED53: Not enough memory for pixbuf allocation");
		return;
	}
	gdk_pixbuf_fill(highlight_pixbuf, border_color);

	// Composite the line onto the backing store - left side
	gdk_pixbuf_composite(highlight_pixbuf,		// Source pixbuf
		tmp_pixbuf,								// Destination pixbuf
		x_offset,								// X offset
		y_offset,								// Y offset
		2,										// Width
		height,									// Height
		0, 0,									// Source offsets
		1, 1,									// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);									// Alpha

	// Composite the line onto the backing store - right side
	gdk_pixbuf_composite(highlight_pixbuf,		// Source pixbuf
		tmp_pixbuf,								// Destination pixbuf
		x_offset + width - 1,					// X offset
		y_offset,								// Y offset
		2,										// Width
		height,									// Height
		0, 0,									// Source offsets
		1, 1,									// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);									// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);

	// Create the inner highlight box
	highlight_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width - 2, height - 2);
	gdk_pixbuf_fill(highlight_pixbuf, fill_color);

	// Composite the inner highlight box onto the backing store
	gdk_pixbuf_composite(highlight_pixbuf,		// Source pixbuf
		tmp_pixbuf,								// Destination pixbuf
		x_offset + 1,							// X offset
		y_offset + 1,							// Y offset
		width - 2,								// Width
		height - 2,								// Height
		0, 0,									// Source offsets
		1, 1,									// Scale factor (1 == no scale)
		GDK_INTERP_NEAREST,						// Scaling type
		255);									// Alpha

	// Free the temporary pixbuf
	g_object_unref(highlight_pixbuf);
	return;
}


// Function that redraws a film strip thumbnail
void draw_thumbnail(GList *which_slide)
{
	// Local variables
	gint				current_height;			// The height of the current thumbnail

	GList				*tmp_glist;				// Temporary GList
	GtkImage			*tmp_image;				//
	GdkPixbuf			*tmp_pixbuf;			//


	// If the current slide hasn't been initialised, don't run this function
	if (NULL == current_slide)
	{
		return;
	}

	// Get the dimensions of the existing thumbnail
	tmp_pixbuf = gtk_image_get_pixbuf(((slide *) which_slide->data)->thumbnail);
	current_height = gdk_pixbuf_get_height(tmp_pixbuf);

	// Create the new thumbnail and add it to the slide
	tmp_pixbuf = compress_layers(which_slide, preview_width, current_height);
	tmp_image = ((slide *) which_slide->data)->thumbnail;
	((slide *) which_slide->data)->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(GDK_PIXBUF(tmp_pixbuf)));

	// Remove the present thumbnail from the film strip slides' event box
	tmp_glist = gtk_container_get_children(GTK_CONTAINER(((slide *) which_slide->data)->event_box));
	if (NULL != tmp_glist)
	{
		// Remove the thumbnail from the container (this also frees up the thumbnail memory)
		gtk_container_remove(GTK_CONTAINER(((slide *) which_slide->data)->event_box), GTK_WIDGET(tmp_glist->data));
	}

	// Add the new thumbnail to the film strip slides' event box
	gtk_container_add(GTK_CONTAINER(((slide *) which_slide->data)->event_box), GTK_WIDGET(((slide *) which_slide->data)->thumbnail));
	gtk_widget_show_all(GTK_WIDGET(((slide *) which_slide->data)->thumbnail));
}


// Function that redraws the timeline
void draw_timeline(void)
{
	// Local variables
	slide				*slide_pointer;			// Points to the presently processing slide

	GList				*tmp_glist;				// Temporary GList


	// If the slide doesn't have a timeline widget constructed for it yet, then make one
	slide_pointer = (slide *) current_slide->data;
	if (NULL == slide_pointer->timeline_widget)
	{
		// Construct the widget used to display the slide in the timeline
		construct_timeline_widget(slide_pointer);
	}

	// Remove the present table from the timeline area, this will also reduce it's reference count by one
	tmp_glist = gtk_container_get_children(GTK_CONTAINER(time_line_container));
	if (NULL != tmp_glist)
	{
			// Remove timeline widget from the container
			gtk_container_remove(GTK_CONTAINER(time_line_container), GTK_WIDGET(tmp_glist->data));
	}

	// Increase the reference count for the timeline widget, so it's not destroyed when this function is next called and it's removed
	g_object_ref(slide_pointer->timeline_widget);

	// Add the timeline widget to the onscreen timeline area
	gtk_container_add(GTK_CONTAINER(time_line_container), GTK_WIDGET(slide_pointer->timeline_widget));

	// Show all of the widgets in the timeline
	gtk_widget_show_all(GTK_WIDGET(time_line_container));
}


// Function that redraws the workspace
void draw_workspace(void)
{
	// Local variables
	GdkPixbuf				*bordered_pixbuf;	// Final pixbuf

	GdkColormap				*tmp_colormap;		// Temporary colormap
	GdkPixbuf				*tmp_pixbuf;		//
	GdkPixmap				*tmp_pixmap;		//
	GdkRectangle			tmp_rectangle;		//


	// If the current slide hasn't been initialised, don't run this function
	if (NULL == current_slide)
	{
		return;
	}

	// Create a new pixbuf with all the layers of the current slide
	tmp_pixmap = backing_store;
	tmp_pixbuf = compress_layers(current_slide, working_width - 2, working_height - 2);

	// Make a 1 pixel edge around the pixbuf, to separate it visually from its background
	bordered_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, working_width, working_height);
	gdk_pixbuf_fill(bordered_pixbuf, 0x000000FF); // fixme4: Drawing four lines around the edges would probably be faster than a bulk memory set
	gdk_pixbuf_copy_area(tmp_pixbuf, 0, 0, working_width - 2, working_height - 2, bordered_pixbuf, 1, 1);

	// Turn the pixbuf into a pixmap, then update the backing store with it
	tmp_colormap = gdk_colormap_get_system();
	// fixme3: Would it be better to just go over the existing backing store memory?
	backing_store = gdk_pixmap_new(NULL, working_width, working_height, tmp_colormap->visual->depth);
	gdk_drawable_set_colormap(GDK_DRAWABLE(backing_store), GDK_COLORMAP(tmp_colormap));
	gdk_draw_pixbuf(GDK_DRAWABLE(backing_store), NULL, GDK_PIXBUF(bordered_pixbuf), 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);

	// Tell the window system to redraw the working area
	tmp_rectangle.x = 0;
	tmp_rectangle.y = 0;
	tmp_rectangle.width = working_width;
	tmp_rectangle.height = working_height;
	gdk_window_invalidate_rect(main_drawing_area->window, &tmp_rectangle, TRUE);

	// Update the workspace
	gtk_widget_queue_draw(GTK_WIDGET(main_drawing_area));

	// Free allocated memory
	g_object_unref(tmp_pixbuf);
	if (NULL != tmp_pixmap)
	{
		g_object_unref(tmp_pixmap);
	}

	// Update the collision detection boundaries
	calculate_object_boundaries();
}


// Enables the layer toolbar buttons
void enable_layer_toolbar_buttons(void)
{
	// Enable the Edit Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_EDIT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_icons[LAYER_EDIT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_EDIT]), layer_toolbar_tooltips, "Edit layer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_EDIT]));
	layer_toolbar_signals[LAYER_EDIT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_EDIT]), "clicked", G_CALLBACK(layer_edit), (gpointer) NULL);

	// Enable the Crop Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_CROP]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_CROP]), layer_toolbar_icons[LAYER_CROP]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_CROP]), layer_toolbar_tooltips, "Crop image", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_CROP]));
	layer_toolbar_signals[LAYER_CROP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_CROP]), "clicked", G_CALLBACK(image_crop), (gpointer) NULL);

	// Enable the Delete Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_DELETE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_icons[LAYER_DELETE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DELETE]), layer_toolbar_tooltips, "Delete selected layer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DELETE]));
	layer_toolbar_signals[LAYER_DELETE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DELETE]), "clicked", G_CALLBACK(layer_delete), (gpointer) NULL);

	// Enable the Move Layer Down icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_DOWN]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_icons[LAYER_DOWN]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_DOWN]), layer_toolbar_tooltips, "Move layer down", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_DOWN]));
	layer_toolbar_signals[LAYER_DOWN] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_DOWN]), "clicked", G_CALLBACK(layer_move_down), (gpointer) NULL);

	// Enable the Move Layer Up icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_UP]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_UP]), layer_toolbar_icons[LAYER_UP]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_UP]), layer_toolbar_tooltips, "Move layer up", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_UP]));
	layer_toolbar_signals[LAYER_UP] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_UP]), "clicked", G_CALLBACK(layer_move_up), (gpointer) NULL);

	// Enable the Add Mouse Pointer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_MOUSE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_icons[LAYER_MOUSE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_MOUSE]), layer_toolbar_tooltips, "Add a mouse pointer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_MOUSE]));
	layer_toolbar_signals[LAYER_MOUSE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_MOUSE]), "clicked", G_CALLBACK(layer_new_mouse), (gpointer) NULL);

	// Enable the Add Text Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_TEXT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_icons[LAYER_TEXT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_TEXT]), layer_toolbar_tooltips, "Add a text layer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_TEXT]));
	layer_toolbar_signals[LAYER_TEXT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_TEXT]), "clicked", G_CALLBACK(layer_new_text), (gpointer) NULL);

	// Enable the Add Highlight Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_HIGHLIGHT]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_icons[LAYER_HIGHLIGHT]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_HIGHLIGHT]), layer_toolbar_tooltips, "Add a highlight layer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_HIGHLIGHT]));
	layer_toolbar_signals[LAYER_HIGHLIGHT] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_HIGHLIGHT]), "clicked", G_CALLBACK(layer_new_highlight), (gpointer) NULL);

	// Enable the Add Image Layer icon
	g_object_ref(layer_toolbar_icons_gray[LAYER_IMAGE]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_icons[LAYER_IMAGE]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(layer_toolbar_items[LAYER_IMAGE]), layer_toolbar_tooltips, "Add an image layer", "Private");
	gtk_widget_show_all(GTK_WIDGET(layer_toolbar_items[LAYER_IMAGE]));
	layer_toolbar_signals[LAYER_IMAGE] = g_signal_connect(G_OBJECT(layer_toolbar_items[LAYER_IMAGE]), "clicked", G_CALLBACK(layer_new_image), (gpointer) NULL);
}


// Enables the main toolbar buttons that can only be used when a project is loaded
void enable_main_toolbar_buttons(void)
{
	// Enable the Crop All icon
	g_object_ref(main_toolbar_icons_gray[CROP_ALL]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[CROP_ALL]), main_toolbar_icons[CROP_ALL]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[CROP_ALL]), main_toolbar_tooltips, "Crop all slides in the project", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[CROP_ALL]));
	main_toolbar_signals[CROP_ALL] = g_signal_connect(G_OBJECT(main_toolbar_items[CROP_ALL]), "clicked", G_CALLBACK(project_crop), (gpointer) NULL);

	// Enable the Export Flash icon
	g_object_ref(main_toolbar_icons_gray[EXPORT_FLASH]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_FLASH]), main_toolbar_icons[EXPORT_FLASH]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_FLASH]), main_toolbar_tooltips, "Export as a Flash animation", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_FLASH]));
	main_toolbar_signals[EXPORT_FLASH] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_FLASH]), "clicked", G_CALLBACK(menu_export_flash_animation), (gpointer) NULL);

	// Enable the Export SVG icon
	g_object_ref(main_toolbar_icons_gray[EXPORT_SVG]);
	gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(main_toolbar_items[EXPORT_SVG]), main_toolbar_icons[EXPORT_SVG]);
	gtk_tool_item_set_tooltip(GTK_TOOL_ITEM(main_toolbar_items[EXPORT_SVG]), main_toolbar_tooltips, "Export as an SVG animation", "Private");
	gtk_widget_show_all(GTK_WIDGET(main_toolbar_items[EXPORT_SVG]));
	main_toolbar_signals[EXPORT_SVG] = g_signal_connect(G_OBJECT(main_toolbar_items[EXPORT_SVG]), "clicked", G_CALLBACK(menu_export_svg_animation), (gpointer) NULL);
}


// Function called when the user clicks the Crop Image toolbar button
void image_crop(void)
{
	// Local variables
	GtkDialog			*crop_dialog;			// Widget for the dialog
	GtkWidget			*crop_table;			// Table used for neat layout of the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to
	gint				dialog_result;			// Catches the return code from the dialog box

	GList				*layer_pointer;			// Points to the layers in the selected slide
	GtkWidget			*list_widget;			// Points to the timeline widget
	gint				new_height;				// Hold the height of the cropped area
	GdkPixbuf			*new_pixbuf;			// Holds the cropped image data
	gint				new_width;				// Hold the width of the cropped area
	guint				selected_row;			// Holds the number of the row that is selected
	layer				*this_layer;			// Temporary layer

	GtkWidget			*left_label;			// Label widget
	GtkWidget			*left_button;			//

	GtkWidget			*right_label;			// Label widget
	GtkWidget			*right_button;			//

	GtkWidget			*top_label;				// Label widget
	GtkWidget			*top_button;			//

	GtkWidget			*bottom_label;			// Label widget
	GtkWidget			*bottom_button;			//

	GtkTreeViewColumn	*tmp_column;			// Temporary column
	gint				tmp_int;				// Temporary int
	GtkTreePath			*tmp_path;				// Temporary path
	GdkPixbuf			*tmp_pixbuf;			// Temporary pixbuf


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;
	list_widget = ((slide *) current_slide->data)->timeline_widget;

	// * Check if the selected layer is an image *

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	layer_pointer = g_list_first(layer_pointer);
	this_layer = g_list_nth_data(layer_pointer, selected_row);

	// Is this layer an image?
	if (TYPE_GDK_PIXBUF != this_layer->object_type)
	{
		// Give the user feedback
		sound_beep();
		display_warning("ED36: Only Image layers can be cropped");
		return;
	}

	// Is this layer the background?
	tmp_int = g_ascii_strncasecmp(this_layer->name->str, "Background", 10);
	if (0 == tmp_int)
	{
		// Give the user feedback
		sound_beep();
		display_warning("ED37: Background layers can not be cropped");
		return;
	}

	// * Pop open a dialog box asking the user how much to crop off each side of the image *

	// Create the dialog window, and table to hold its children
	crop_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Crop image layer", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	crop_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(crop_dialog->vbox), GTK_WIDGET(crop_table), FALSE, FALSE, 10);

	// Create the label asking for the left side crop amount
	left_label = gtk_label_new("Left crop: ");
	gtk_misc_set_alignment(GTK_MISC(left_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	left_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(left_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	right_label = gtk_label_new("Right crop: ");
	gtk_misc_set_alignment(GTK_MISC(right_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	right_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(right_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the top crop amount
	top_label = gtk_label_new("Top crop: ");
	gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	top_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(top_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	bottom_label = gtk_label_new("Bottom crop: ");
	gtk_misc_set_alignment(GTK_MISC(bottom_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	bottom_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bottom_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(crop_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(crop_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(crop_dialog));
		return;
	}

	// Create a new pixbuf, for storing the cropped image in
	new_height = gdk_pixbuf_get_height(((layer_image *) this_layer->object_data)->image_data)
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button))
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(bottom_button));
	new_width = gdk_pixbuf_get_width(((layer_image *) this_layer->object_data)->image_data)
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button))
		- (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(right_button));
	new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, new_width, new_height);

	// Create a new pixbuf, having just the cropped image data in it
	gdk_pixbuf_copy_area(((layer_image *) this_layer->object_data)->image_data, // Source pixbuf
		(gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button)),  // Left crop
		(gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button)),  // Top crop
		new_width,  // Width
		new_height,  // Height
		new_pixbuf,  // Destination
		0, 0);

	// Update the layer with the new cropped data
	tmp_pixbuf = ((layer_image *) this_layer->object_data)->image_data;
	((layer_image *) this_layer->object_data)->image_data = new_pixbuf;
	((layer_image *) this_layer->object_data)->width = new_width;
	((layer_image *) this_layer->object_data)->height = new_height;
	g_string_assign(((layer_image *) this_layer->object_data)->image_path, "");
	((layer_image *) this_layer->object_data)->modified = TRUE;

	// Free the memory used by the old pixbuf
	g_object_unref(tmp_pixbuf);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(crop_dialog));

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();
}


// Function which deletes the layer the user presently has selected
void layer_delete(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	GtkListStore		*list_pointer;			//
	GtkWidget			*list_widget;			// Points to the timeline widget
	guint				num_layers;				// Number of layers
	guint				selected_row;			// Holds the number of the row that is selected

	gboolean			tmp_bool;				// Temporary boolean
	GtkTreeViewColumn	*tmp_column;			// Temporary column
	GtkTreePath			*tmp_path;				// Temporary path
	layer				*tmp_layer;				// Temporary layer


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;
	list_pointer = ((slide *) current_slide->data)->layer_store;
	list_widget = ((slide *) current_slide->data)->timeline_widget;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));

	// If the background layer is selected, don't delete it
	if (1 == (num_layers - selected_row))
	{
		// Give the user a warning, then return
		sound_beep();
		display_warning("ED38: Background layers can not be deleted\n");
		return;
	}

	// Remove the layer from the TreeView widget
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);
	tmp_bool = gtk_list_store_remove(list_pointer, tmp_layer->row_iter);

	// Remove the layer from the layer structure
	layer_pointer = g_list_remove(layer_pointer, tmp_layer);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Make the row above in the timeline widget selected
	if (0 != selected_row)
	{
		selected_row = selected_row - 1;
	}
	tmp_path = gtk_tree_path_new_from_indices(selected_row, -1);
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(list_widget), tmp_path, NULL, FALSE);

	// Redraw the workspace area
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Free the storage allocated by this function
	gtk_tree_path_free(tmp_path);

	return;
}


// Function called when the user clicks the Edit Layer toolbar button
void layer_edit(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	GtkListStore		*list_pointer;			//
	GtkWidget			*list_widget;			// Points to the timeline widget
	guint				num_layers;				// Number of layers
	guint				selected_row;			// Holds the row that is selected
	gboolean			return_code;			// Did the edit dialog return ok?

	GtkTextIter			text_start;				// The start position of the text buffer
	GtkTextIter			text_end;				// The end position of the text buffer

	GtkTreeViewColumn	*tmp_column;			// Temporary column
	layer_highlight		*tmp_highlight_ob;		// Temporary highlight layer object
	layer_image			*tmp_image_ob;			// Temporary image layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	layer_mouse			*tmp_mouse_ob;			// Temporary mouse layer object
	layer_text			*tmp_text_ob;			// Temporary text layer object
	GtkTreePath			*tmp_path;				// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise some variables
	layer_pointer = ((slide *) current_slide->data)->layers;
	list_pointer = ((slide *) current_slide->data)->layer_store;
	list_widget = ((slide *) current_slide->data)->timeline_widget;

	// Determine the number of layers present in this slide
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(list_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	tmp_layer = g_list_nth_data(layer_pointer, selected_row);

	// If the background layer is selected, don't edit it
	if (1 == (num_layers - selected_row))
	{
		// Give the user a warning, then return
		sound_beep();
		display_warning("ED39: Background layers can not be edited\n");
		return;
	}

	// * Open a dialog box showing the existing values, asking for the new ones *

	switch (tmp_layer->object_type)
	{
		case TYPE_GDK_PIXBUF:
			// Open a dialog box for the user to edit the image layer values
			return_code = display_dialog_image(tmp_layer, "Edit image layer", FALSE);
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully, so update the slide list store with the new values *
				tmp_image_ob = (layer_image *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
							TIMELINE_START, tmp_layer->start_frame,
							TIMELINE_FINISH, tmp_layer->finish_frame,
							TIMELINE_X_OFF_START, tmp_image_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_image_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_image_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_image_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_MOUSE_CURSOR:
			// Open a dialog box for the user to edit the mouse pointer values
			return_code = display_dialog_mouse(tmp_layer, "Edit mouse pointer", FALSE);
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully, so update the slide list store with the new values *
				tmp_mouse_ob = (layer_mouse *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
							TIMELINE_START, tmp_layer->start_frame,
							TIMELINE_FINISH, tmp_layer->finish_frame,
							TIMELINE_X_OFF_START, tmp_mouse_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_mouse_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_mouse_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_mouse_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_TEXT:
			// Open a dialog box for the user to edit the text layer values
			return_code = display_dialog_text(tmp_layer, "Edit text layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully, so update the slide list store with the new values *
				tmp_text_ob = (layer_text *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_text_buffer_get_bounds(((layer_text *) tmp_layer->object_data)->text_buffer, &text_start, &text_end);
				gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
							TIMELINE_NAME, tmp_layer->name->str,
							TIMELINE_START, tmp_layer->start_frame,
							TIMELINE_FINISH, tmp_layer->finish_frame,
							TIMELINE_X_OFF_START, tmp_text_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_text_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_text_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_text_ob->y_offset_finish,
							-1);
			}
			break;

		case TYPE_HIGHLIGHT:
			// Open a dialog box for the user to edit the highlight layer values
			return_code = display_dialog_highlight(tmp_layer, "Edit highlight layer");
			if (TRUE == return_code)
			{
				// * The dialog box returned successfully, so update the slide list store with the new values *
				tmp_highlight_ob = (layer_highlight *) tmp_layer->object_data;
				tmp_iter = tmp_layer->row_iter;
				gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
							TIMELINE_START, tmp_layer->start_frame,
							TIMELINE_FINISH, tmp_layer->finish_frame,
							TIMELINE_X_OFF_START, tmp_highlight_ob->x_offset_start,
							TIMELINE_Y_OFF_START, tmp_highlight_ob->y_offset_start,
							TIMELINE_X_OFF_FINISH, tmp_highlight_ob->x_offset_finish,
							TIMELINE_Y_OFF_FINISH, tmp_highlight_ob->y_offset_finish,
							-1);
			}
			break;

		default:
			display_warning("ED34: Unknown layer type\n");
			return;
	}

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the user clicks the Move Layer Down toolbar button
void layer_move_down(void)
{
	// Local variables
	GList				*below_layer;			// The layer below the selected one
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gint				num_layers;				// The number of layers in the select slide
	GList				*our_layer;				// The selected layer
	gint				selected_row;			// Holds the row that is selected

	GtkTreeViewColumn	*tmp_column;			// Temporary column
	GList				*tmp_glist;				// Temporary GList
	GtkTreePath			*tmp_path;				// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise various things
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	num_layers = g_list_length(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	if (num_layers - 2 <= selected_row)
	{
		// We're already at the bottom of the list or the background layer is selected, so return
		sound_beep();
		display_warning("ED40: This layer is already at the bottom\n");
		return;
	}

	// Get details of the layers we're moving around
	our_layer = g_list_nth(layer_pointer, selected_row);
	below_layer = g_list_nth(layer_pointer, selected_row + 1);

	// Move the row down one in the timeline widget
	gtk_list_store_move_before(((slide *) current_slide->data)->layer_store, ((layer *) below_layer->data)->row_iter, ((layer *) our_layer->data)->row_iter);

	// Move the row down one in the layer list
	tmp_glist = g_list_remove_link(layer_pointer, below_layer);
	layer_pointer = g_list_insert_before(layer_pointer, our_layer, below_layer->data);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the user clicks the Move Layer Up toolbar button
void layer_move_up(void)
{
	// Local variables
	GList				*above_layer;			// The layer above the selected one
	GList				*layer_pointer;			// Points to the layers in the selected slide
	GList				*our_layer;				// The selected layer
	guint				selected_row;			// Holds the row that is selected

	GtkTreeViewColumn	*tmp_column;			// Temporary column
	GList				*tmp_glist;				// Temporary GList
	gint				tmp_int;				// Temporary int
	GtkTreePath			*tmp_path;				// Temporary path


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// Initialise various things
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);

	// Determine which layer the user has selected in the timeline
	tmp_path = gtk_tree_path_new();
	tmp_column = gtk_tree_view_column_new();
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), &tmp_path, &tmp_column);
	selected_row = atoi(gtk_tree_path_to_string(tmp_path));
	if (0 == selected_row)
	{
		// We're already at the top of the list, so return
		sound_beep();
		display_warning("ED41: This layer is already at the top\n");
		return;
	}

	// Get details of the layers we're moving around
	our_layer = g_list_nth(layer_pointer, selected_row);
	above_layer = g_list_nth(layer_pointer, selected_row - 1);

	// Check if the user has selected the Background layer, if so we return (ignoring this move request)
	// fixme3: Might be better to beep or give some kind of feedback (warning/error?) to the user
	tmp_int = g_ascii_strncasecmp(((layer *) our_layer->data)->name->str, "Background", 10);
	if (0 == tmp_int)
	{
		// We're processing a background layer, so return
		return;
	}

	// Move the row up one in the timeline widget
	gtk_list_store_move_before(((slide *) current_slide->data)->layer_store, ((layer *) our_layer->data)->row_iter, ((layer *) above_layer->data)->row_iter);

	// Move the row up one in the layer list
	tmp_glist = g_list_remove_link(layer_pointer, our_layer);
	layer_pointer = g_list_insert_before(layer_pointer, above_layer, our_layer->data);
	((slide *) current_slide->data)->layers = layer_pointer;

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);
}


// Function called when the user clicks the Add Highlight Layer toolbar button
void layer_new_highlight(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gboolean			return_code;			// Catches a TRUE/FALSE return value

	layer_highlight		*tmp_highlight_ob;		// Temporary highlight layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	GtkTreePath			*tmp_path;				// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// * Create a new highlight layer in memory using reasonable defaults *

	// Create the highlight layer data
	tmp_highlight_ob = g_new(layer_highlight, 1);
	tmp_highlight_ob->x_offset_start = 100;
	tmp_highlight_ob->y_offset_start = 100;
	tmp_highlight_ob->x_offset_finish = 100;
	tmp_highlight_ob->y_offset_finish = 100;
	tmp_highlight_ob->width = 400;
	tmp_highlight_ob->height = 300;

	// Constuct the new highlight layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_HIGHLIGHT;
	tmp_layer->object_data = (GObject *) tmp_highlight_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Highlight");
	tmp_layer->external_link = g_string_new(NULL);

	// Display a dialog box to edit these values, using our new highlight layer object
	return_code = display_dialog_highlight(tmp_layer, "Add new highlight layer");
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_free(tmp_layer);
		g_free(tmp_highlight_ob);
		return;
	}

	// * To get here, the user must have clicked OK in the create highlight layer dialog box, so we process the results *

	// Add the new layer to the slide
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(((slide *) current_slide->data)->layer_store, tmp_iter);
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_START, tmp_layer->start_frame,
						TIMELINE_FINISH, tmp_layer->finish_frame,
						TIMELINE_X_OFF_START, tmp_highlight_ob->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_highlight_ob->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_highlight_ob->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_highlight_ob->y_offset_finish,
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);
}


// Function called when the user clicks the Add Image Layer toolbar button
void layer_new_image(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gboolean			return_code;			// Catches a TRUE/FALSE return value

	layer_image			*tmp_image_ob;			// Temporary image layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	GtkTreePath			*tmp_path;				// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// * Create a new image layer in memory using reasonable defaults *

	// Create the image layer data
	tmp_image_ob = g_new(layer_image, 1);
	tmp_image_ob->x_offset_start = 100;
	tmp_image_ob->y_offset_start = 100;
	tmp_image_ob->x_offset_finish = 100;
	tmp_image_ob->y_offset_finish = 100;
	tmp_image_ob->width = 400;
	tmp_image_ob->height = 300;
	tmp_image_ob->modified = FALSE;

	// Constuct the new image layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_GDK_PIXBUF;
	tmp_layer->object_data = (GObject *) tmp_image_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Image");
	tmp_layer->external_link = g_string_new(NULL);

	// Display a dialog box to edit these values, using our new image layer object
	return_code = display_dialog_image(tmp_layer, "Add new image layer", TRUE);
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_free(tmp_layer);
		g_free(tmp_image_ob);
		return;
	}

	// * To get here, the user must have clicked OK in the dialog box, so we process the results *

	// Add the new layer to the slide
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(((slide *) current_slide->data)->layer_store, tmp_iter);
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_START, tmp_layer->start_frame,
						TIMELINE_FINISH, tmp_layer->finish_frame,
						TIMELINE_X_OFF_START, tmp_image_ob->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_image_ob->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_image_ob->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_image_ob->y_offset_finish,						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);
}


// Function called when the user clicks the Add Mouse Pointer toolbar button
void layer_new_mouse(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gboolean			return_code;			// Catches a TRUE/FALSE return value

	layer_mouse			*tmp_mouse_ob;			// Temporary mouse layer object
	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	GtkTreePath			*tmp_path;				// Temporary GtkPath


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// * Create a new mouse layer in memory using reasonable defaults *

	// Create the mouse pointer layer data
	tmp_mouse_ob = g_new(layer_mouse, 1);
	tmp_mouse_ob->x_offset_start = 100;
	tmp_mouse_ob->y_offset_start = 100;
	tmp_mouse_ob->x_offset_finish = 100;
	tmp_mouse_ob->y_offset_finish = 100;
	tmp_mouse_ob->width = 22;
	tmp_mouse_ob->height = 32;
	tmp_mouse_ob->click = MOUSE_NONE;
	tmp_mouse_ob->image_path = g_string_new("");

	// Constuct the new mouse pointer layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_MOUSE_CURSOR;
	tmp_layer->object_data = (GObject *) tmp_mouse_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Mouse Pointer");
	tmp_layer->external_link = g_string_new(NULL);

	// Display a dialog box to edit these values, using our new mouse pointer layer object
	return_code = display_dialog_mouse(tmp_layer, "Add mouse pointer", TRUE);
	if (TRUE != return_code)
	{
		// The user cancelled out of the dialog box, so destroy our new layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_free(tmp_layer);
		g_free(tmp_mouse_ob);
		return;
	}

	// * To get here, the user must have clicked OK in the dialog box, so we process the results *

	// Add the new layer to the slide
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Add the new layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(((slide *) current_slide->data)->layer_store, tmp_iter);
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_START, tmp_layer->start_frame,
						TIMELINE_FINISH, tmp_layer->finish_frame,
						TIMELINE_X_OFF_START, tmp_mouse_ob->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_mouse_ob->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_mouse_ob->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_mouse_ob->y_offset_finish,						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);
}


// Function called when the user clicks the Add Text Layer toolbar button
void layer_new_text(void)
{
	// Local variables
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gboolean			return_code;			// Catches a TRUE/FALSE return value

	GtkTextIter			text_start;				// The start position of the text buffer
	GtkTextIter			text_end;				// The end position of the text buffer

	GtkTreeIter			*tmp_iter;				// Temporary iter
	layer				*tmp_layer;				// Temporary layer
	GtkTreePath			*tmp_path;				// Temporary GtkPath
	layer_text			*tmp_text_ob;			// Temporary text layer object


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		sound_beep();
		return;
	}

	// * Prepare a new text layer object in memory, with reasonable defaults *

	// Create the text layer data
	tmp_text_ob = g_new(layer_text, 1);
	tmp_text_ob->x_offset_start = 100;
	tmp_text_ob->y_offset_start = 100;
	tmp_text_ob->x_offset_finish = 100;
	tmp_text_ob->y_offset_finish = 100;
	tmp_text_ob->text_color.red = 0;
	tmp_text_ob->text_color.green = 0;
	tmp_text_ob->text_color.blue = 0;
	tmp_text_ob->font_size = 40;
	tmp_text_ob->text_buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(tmp_text_ob->text_buffer), "New text...", -1);

	// Construct a new text layer
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_TEXT;
	tmp_layer->object_data = (GObject *) tmp_text_ob;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Text layer");
	tmp_layer->external_link = g_string_new(NULL);

	// Display a dialog box to edit these values, using our new text layer object
	return_code = display_dialog_text(tmp_layer, "Add new text layer");
	if (FALSE == return_code)
	{
		// The user cancelled out of the create text layer dialog box, so destroy our new text layer and return
		g_string_free(tmp_layer->name, TRUE);
		g_free(tmp_layer);
		g_object_unref(tmp_text_ob->text_buffer);
		g_free(tmp_text_ob);
		return;
	}

	// * To get here, the user must have clicked OK in the create text layer dialog box, so we process the results *

	// Add the new text layer to the slide
	layer_pointer = ((slide *) current_slide->data)->layers;
	layer_pointer = g_list_first(layer_pointer);
	layer_pointer = g_list_prepend(layer_pointer, tmp_layer);

	// Add the new text layer to slide list store
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	gtk_list_store_prepend(((slide *) current_slide->data)->layer_store, tmp_iter);
	gtk_text_buffer_get_bounds(tmp_text_ob->text_buffer, &text_start, &text_end);
	gtk_list_store_set(((slide *) current_slide->data)->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_START, tmp_layer->start_frame,
						TIMELINE_FINISH, tmp_layer->finish_frame,
						TIMELINE_X_OFF_START, tmp_text_ob->x_offset_start,
						TIMELINE_Y_OFF_START, tmp_text_ob->y_offset_start,
						TIMELINE_X_OFF_FINISH, tmp_text_ob->x_offset_finish,
						TIMELINE_Y_OFF_FINISH, tmp_text_ob->y_offset_finish,
						-1);

	// Redraw the workspace
	draw_workspace();

	// Redraw the film strip thumbnail
	draw_thumbnail(current_slide);

	// Select the new layer in the timeline widget
	tmp_path = gtk_tree_path_new_first();
	gtk_tree_view_set_cursor(GTK_TREE_VIEW(((slide *) current_slide->data)->timeline_widget), tmp_path, NULL, FALSE);
}


// Function called when the user selects Edit -> Preferences from the top menu
void menu_edit_preferences(void)
{
	// Local variables
	GtkDialog			*main_dialog;						// Widget for the main dialog
	GtkWidget			*main_notebook;						// Widget for the main notebook
	GtkWidget			*app_dialog_table;					// Table used for neat layout of the labels and fields in application preferences
	GtkWidget			*proj_dialog_table;					// Table used for neat layout of the labels and fields in project preferences
	GtkWidget			*app_dialog_label;					// Tab text for the application tab of the dialog box
	GtkWidget			*proj_dialog_label;					// Tab text for the project tab of the dialog box
	gint				app_row_counter;					// Used when building the application preferences dialog box
	gint				proj_row_counter;					// Used when building the project preferences dialog box
	gint				dialog_result;						// Catches the return code from the dialog box

	// * Application variables *

	GtkWidget			*label_default_project_folder;		// Default Project Folder
	GtkWidget			*button_default_project_folder;		//

	GtkWidget			*label_screenshot_folder;			// Screenshot Folder
	GtkWidget			*button_screenshot_folder;			//

	GtkWidget			*label_default_output_folder;		// Default Output path
	GtkWidget			*button_default_output_folder;		//

	GtkWidget			*label_default_output_width;		// Default Output Width
	GtkWidget			*button_default_output_width;		//

	GtkWidget			*label_default_output_height;		// Default Output Height
	GtkWidget			*button_default_output_height;		//

	GtkWidget			*label_default_output_quality;		// Default Output Quality
	GtkWidget			*button_default_output_quality;		//

	GtkWidget			*label_default_slide_length;		// Default Slide Length
	GtkWidget			*button_default_slide_length;		//

	GtkWidget			*label_preview_width;				// Preview Width
	GtkWidget			*button_preview_width;				//

	GtkWidget			*label_icon_height;					// Icon Height
	GtkWidget			*button_icon_height;				//

	GtkWidget			*label_default_zoom_level;			// Default Zoom Level
	GtkWidget			*entry_default_zoom_level;			//

	GtkWidget			*label_default_bg_colour;			// Default background colour
	GtkWidget			*button_default_bg_colour;			// Color button

	GtkWidget			*label_scaling_quality;				// Scaling quality
	GtkWidget			*button_scaling_quality;			//

	// * Project variables *

	GtkWidget			*label_project_name;				// Project Name
	GtkWidget			*entry_project_name;				//

	GtkWidget			*label_project_folder;				// Project Folder
	GtkWidget			*button_project_folder;				//

	GtkWidget			*label_output_folder;				// Output Folder
	GtkWidget			*button_output_folder;				//

	GtkWidget			*label_output_quality;				// Output Quality
	GtkWidget			*button_output_quality;				//

	GtkWidget			*label_slide_length;				// Slide Length
	GtkWidget			*button_slide_length;				//

	GtkWidget			*label_frames_per_second;			// Slide Length
	GtkWidget			*button_frames_per_second;			//


	// Initialise various things
	app_row_counter = 0;
	proj_row_counter = 0;

	// Create the main dialog window
	main_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Preferences", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	main_notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(main_notebook), GTK_POS_TOP);
	app_dialog_table = gtk_table_new(10, 3, FALSE);
	app_dialog_label = gtk_label_new("General");
	proj_dialog_table = gtk_table_new(10, 3, FALSE);
	proj_dialog_label = gtk_label_new("This Project");
	gtk_notebook_append_page(GTK_NOTEBOOK(main_notebook), app_dialog_table, app_dialog_label);
	gtk_notebook_append_page(GTK_NOTEBOOK(main_notebook), proj_dialog_table, proj_dialog_label);
	gtk_box_pack_start(GTK_BOX(main_dialog->vbox), GTK_WIDGET(main_notebook), FALSE, FALSE, 5);


	// * Create Application preferences tab *

// fixme4: gtk_file_chooser_button functions aren't present in GTK 2.4.x (shipped with Solaris 10)
#ifndef __sun

	// Default Project Folder
	label_default_project_folder = gtk_label_new("Default Project Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_project_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_project_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_project_folder = gtk_file_chooser_button_new("Select the Default Project Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_project_folder), default_project_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_project_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Screenshot Folder
	label_screenshot_folder = gtk_label_new("Screenshots Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_screenshot_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_screenshot_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_screenshot_folder = gtk_file_chooser_button_new("Select the Screenshot Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder), screenshots_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_screenshot_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Folder
	label_default_output_folder = gtk_label_new("Default Output Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_folder), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_folder = gtk_file_chooser_button_new("Select the Default Output Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_default_output_folder), default_output_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_folder), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

#endif

	// Default Output Width
	label_default_output_width = gtk_label_new("Default Output Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_width), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_width), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_width = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_width), default_output_width);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_width), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Height
	label_default_output_height = gtk_label_new("Default Output Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_height), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_height), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_height = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_height), default_output_height);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_height), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Output Quality
	label_default_output_quality = gtk_label_new("Default Output Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_output_quality), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_output_quality), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_output_quality = gtk_spin_button_new_with_range(0, 9, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_output_quality), default_output_quality);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_output_quality), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Default Slide Length
	label_default_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_slide_length), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_slide_length), 0, 1, app_row_counter, app_row_counter + 1);
	button_default_slide_length = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_default_slide_length), default_slide_length);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_slide_length), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Preview width
	label_preview_width = gtk_label_new("Preview Width: ");
	gtk_misc_set_alignment(GTK_MISC(label_preview_width), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_preview_width), 0, 1, app_row_counter, app_row_counter + 1);
	button_preview_width = gtk_spin_button_new_with_range(0, 1200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_preview_width), preview_width);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_preview_width), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Icon Height
	label_icon_height = gtk_label_new("Icon Height: ");
	gtk_misc_set_alignment(GTK_MISC(label_icon_height), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_icon_height), 0, 1, app_row_counter, app_row_counter + 1);
	button_icon_height = gtk_spin_button_new_with_range(0, 200, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_icon_height), icon_height);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_icon_height), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

// fixme4: gtk_combo_box_get_active_text function isn't present in GTK 2.4.x (shipped with Solaris 10)
#ifndef __sun

	// Default Zoom level
	label_default_zoom_level = gtk_label_new("Default Zoom Level: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_zoom_level), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_zoom_level), 0, 1, app_row_counter, app_row_counter + 1);
	// fixme3: We don't have a selector for Default Zoom Level yet, so this is just stub code for now
	entry_default_zoom_level = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_default_zoom_level), 14);
	gtk_entry_set_text(GTK_ENTRY(entry_default_zoom_level), gtk_combo_box_get_active_text(GTK_COMBO_BOX(zoom_selector)));
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(entry_default_zoom_level), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

#endif

	// Default Background Colour
	label_default_bg_colour = gtk_label_new("Default Background Colour: ");
	gtk_misc_set_alignment(GTK_MISC(label_default_bg_colour), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_default_bg_colour), 0, 1, app_row_counter, app_row_counter + 1);
    button_default_bg_colour = gtk_color_button_new_with_color(&default_bg_colour);
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(button_default_bg_colour), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_default_bg_colour), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// Scaling Quality
	label_scaling_quality = gtk_label_new("Scaling Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_scaling_quality), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(label_scaling_quality), 0, 1, app_row_counter, app_row_counter + 1);
	button_scaling_quality = gtk_spin_button_new_with_range(0, 3, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_scaling_quality), scaling_quality);
	gtk_table_attach_defaults(GTK_TABLE(app_dialog_table), GTK_WIDGET(button_scaling_quality), 2, 3, app_row_counter, app_row_counter + 1);
	app_row_counter = app_row_counter + 1;

	// * Create Project Preferences tab *

	// Project Name
	label_project_name = gtk_label_new("Project Name: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_name), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_name), 0, 1, proj_row_counter, proj_row_counter + 1);
	entry_project_name = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry_project_name), 50);
	gtk_entry_set_text(GTK_ENTRY(entry_project_name), (gchar *) project_name->str);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(entry_project_name), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

// fixme4: gtk_file_chooser_button functions aren't present in GTK 2.4.x (shipped with Solaris 10)
#ifndef __sun

	// Project Folder
	label_project_folder = gtk_label_new("Project Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_project_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_project_folder), 0, 1, proj_row_counter, proj_row_counter + 1);
	button_project_folder = gtk_file_chooser_button_new("Select the Project Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_project_folder), project_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_project_folder), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

	// Output Folder
	label_output_folder = gtk_label_new("Output Folder: ");
	gtk_misc_set_alignment(GTK_MISC(label_output_folder), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_output_folder), 0, 1, proj_row_counter, proj_row_counter + 1);
	button_output_folder = gtk_file_chooser_button_new("Select the Output Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(button_output_folder), output_folder->str);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_output_folder), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

#endif

	// Output Quality
	label_output_quality = gtk_label_new("Output Quality: ");
	gtk_misc_set_alignment(GTK_MISC(label_output_quality), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_output_quality), 0, 1, proj_row_counter, proj_row_counter + 1);
	button_output_quality = gtk_spin_button_new_with_range(0, 9, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_output_quality), output_quality);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_output_quality), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

	// Slide Length
	label_slide_length = gtk_label_new("Default Slide Length: ");
	gtk_misc_set_alignment(GTK_MISC(label_slide_length), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_slide_length), 0, 1, proj_row_counter, proj_row_counter + 1);
	button_slide_length = gtk_spin_button_new_with_range(0, 1000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_slide_length), slide_length);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_slide_length), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

	// Frames per second
	label_frames_per_second = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(label_frames_per_second), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(label_frames_per_second), 0, 1, proj_row_counter, proj_row_counter + 1);
	button_frames_per_second = gtk_spin_button_new_with_range(0, 48, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(button_frames_per_second), frames_per_second);
	gtk_table_attach_defaults(GTK_TABLE(proj_dialog_table), GTK_WIDGET(button_frames_per_second), 2, 3, proj_row_counter, proj_row_counter + 1);
	proj_row_counter = proj_row_counter + 1;

	// Set the dialog going
	gtk_widget_show_all(GTK_WIDGET(main_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(main_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT == dialog_result)
	{
		// * Yes, so update the project variables with their new values *

		// ** Application Preferences **

		// fixme3: No validation is done on this input yet.  To make a secure application, it really needs to be

		// Default Project Folder
		default_project_folder = g_string_assign(default_project_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_project_folder)));

		// Default Project Folder
		screenshots_folder = g_string_assign(screenshots_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_screenshot_folder)));

		// Default Output Folder
		default_output_folder = g_string_assign(default_output_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_default_output_folder)));

		// Default Output Width
		// fixme2: This should be a selector, like the output selector
		default_output_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_width));

		// Default Output Height
		default_output_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_height));

		// Default Output Quality
		default_output_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_output_quality));

		// Default Slide Length
		default_slide_length = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_default_slide_length));

		// Preview width
		preview_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_preview_width));

		// Icon Height
		icon_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_icon_height));

		// Default Zoom Level
		// fixme3: Couldn't be bothered coding this yet
//		default_zoom_level = g_string_assign(default_zoom_level, gtk_entry_get_text(GTK_ENTRY(entry_default_zoom_level)));

		// Default Background Colour
		gtk_color_button_get_color(GTK_COLOR_BUTTON(button_default_bg_colour), &default_bg_colour);

		// Scaling Quality
		scaling_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_scaling_quality));

		// * Project Preferences *

		// Project Name
		project_name = g_string_assign(project_name, gtk_entry_get_text(GTK_ENTRY(entry_project_name)));

		// Project Folder
		project_folder = g_string_assign(project_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_project_folder)));

		// Output Folder
		output_folder = g_string_assign(output_folder, gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(button_output_folder)));

		// Output Quality
		output_quality = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_output_quality));

		// Slide Length
		slide_length = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_slide_length));

		// Frames per second
		frames_per_second = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button_frames_per_second));
	}

	// Free up the memory allocated in this function
	// fixme2: Once this function works properly, I should revisit this code and ensure nothing got missed in the cleanup
	gtk_widget_destroy(GTK_WIDGET(label_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_project_folder));
	gtk_widget_destroy(GTK_WIDGET(label_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(button_default_output_folder));
	gtk_widget_destroy(GTK_WIDGET(main_dialog));
}


// Function called when the user selects Export -> Flash Animation from the top menu
void menu_export_flash_animation(void)
{
	// Local variables
	GtkFileFilter		*all_filter;			// Filter for *.*
	GError				*error = NULL;			// Pointer to error return structure
	GtkWidget 			*export_dialog;			// Dialog widget
	gchar				*filename;				// Pointer to the chosen file name
	GtkFileFilter		*flash_filter;			// Filter for *.swf
	GIOStatus			return_value;			// Return value used in most GIOChannel functions
	GByteArray			*swf_buffer;			// Buffer for the swf output
	gboolean			unique_name;			// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;			// Widget for overwrite warning dialog

	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;			// Temporary GString
	gint				tmp_int;				// Temporary integer


	// Check if there is an active project
	if (NULL == slides)
	{
		// No project is active, so display a message and return
		// fixme4: This code should never be reached any more, as exporting is disabled while no project loaded
		sound_beep();
		display_warning("ED35: There is no project loaded\n");
		return;
	}

	// * Pop open a dialog asking the user for their desired filename *

	// Create the dialog asking the user for the name to save as
	export_dialog = gtk_file_chooser_dialog_new("Export as Flash",
						GTK_WINDOW(main_window),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);

	// Create the filter so only *.flash files are displayed
	flash_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flash_filter, "*.swf");
	gtk_file_filter_set_name(flash_filter, "Macromedia Flash (*.swf)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), flash_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), all_filter);

	// Set the name of the file to save as
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s.swf", project_name->str);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(export_dialog), tmp_gstring->str);

	// Change to the default output directory
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(export_dialog), output_folder->str);

	// Run the dialog and wait for user input
	unique_name = FALSE;
	while (TRUE != unique_name)
	{
		// Get the filename to export to
		if (gtk_dialog_run(GTK_DIALOG(export_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(export_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

		// Check if there's an existing file of this name, and give an Overwrite? type prompt if there is
		if (TRUE == g_file_test(filename, G_FILE_TEST_EXISTS))
		{
			// Something with this name already exists
			warn_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
								GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_YES_NO,
								"Overwrite existing file?");
			if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
			{
				// We've been told to overwrite the existing file
				unique_name = TRUE;
			}
			gtk_widget_destroy(warn_dialog);
		} else
		{
			// The indicated file name is unique, we're fine to save
			unique_name = TRUE;
		}
	}

	// * We only get to here if a file was chosen *

	// Get the filename from the dialog box
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(export_dialog);

	// Open output file for writing
	output_file = g_io_channel_new_file(filename, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED80: An error '%s' occured when opening '%s' for writing", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Work out how many slides there are in the whole project
	slides = g_list_first(slides);
	tmp_int = g_list_length(slides);

	// Export all slides to the swf buffer
	swf_buffer = g_byte_array_new();
	swf_buffer = menu_export_flash_inner(swf_buffer);
//	swf_buffer = menu_export_flash_inner(swf_buffer, 0, tmp_int - 1);

	// Add the end tag to the swf buffer
	// fixme3: For some unknown reason having \0 crashes out, whereas every other character works
	//         It's a pity the \0 is actually needed, so we'll need to add this later somehow
//	swf_buffer = g_byte_array_append(swf_buffer, "\0", 1);

	// Add the header to the swf buffer
	// fixme3: Need to expand this next bit to also include the frame size, frame rate, and frame count
	g_string_printf(tmp_gstring, "FWS%c%u", 0x06, (guint32) swf_buffer->len);  // 0x06 = swf version 6
	swf_buffer = g_byte_array_prepend(swf_buffer, tmp_gstring->str, tmp_gstring->len);

	// Write the swf data to the output file
	return_value = g_io_channel_write_chars(output_file, swf_buffer->data, swf_buffer->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the swf data to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED81: An error '%s' occured when writing the swf data to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Close the output file
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED82: An error '%s' occured when closing the output file '%s'", error->message, filename);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);
		return;
	}

	// Small update to the status bar, to show progress to the user
	g_string_printf(tmp_gstring, "Wrote Flash file '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

	// Free the swf buffer array
	g_byte_array_free(swf_buffer, TRUE);

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(tmp_gstring, TRUE);
}


// Function called when the user selects Export -> SVG Animation from the top menu
void menu_export_svg_animation(void)
{
	// Local variables
	GtkFileFilter		*all_filter;			// Filter for *.*
	GError				*error = NULL;			// Pointer to error return structure
	GtkWidget 			*export_dialog;			// Dialog widget
	gchar				*filename;				// Pointer to the chosen file name
	gfloat				control_bar_x_offset;	// X offset for the onscreen play button
	gfloat				control_bar_y_offset;	// Y offset for the onscreen play button
	GIOStatus			return_value;			// Return value used in most GIOChannel functions
	GList				*slide_pointer;			// Points to the presently processing slide
	GtkFileFilter		*svg_filter;			// Filter for *.svg
	gboolean			unique_name;			// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;			// Widget for overwrite warning dialog
	gfloat				x_scale;				// Width scale factor for the scene
	gfloat				y_scale;				// Height scale factor for the scene

	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;			// Temporary GString


	// Check if there is an active project
	if (NULL == slides)
	{
		// No project is active, so display a message and return
		// fixme4: This code should never be reached any more, as exporting is disabled while no project loaded
		sound_beep();
		display_warning("ED15: There is no project loaded\n");
		return;
	}

	// * Pop open a dialog asking the user for their desired filename *

	// Create the dialog asking the user for the name to save as
	export_dialog = gtk_file_chooser_dialog_new("Export as SVG",
						GTK_WINDOW(main_window),
						GTK_FILE_CHOOSER_ACTION_SAVE,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
						NULL);

	// Create the filter so only *.svg files are displayed
	svg_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(svg_filter, "*.svg");
	gtk_file_filter_set_name(svg_filter, "Scalable Vector Graphics (*.svg)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), svg_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(export_dialog), all_filter);

	// Set the name of the file to save as
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "%s.svg", project_name->str);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(export_dialog), tmp_gstring->str);

	// Change to the default output directory
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(export_dialog), output_folder->str);

	// Run the dialog and wait for user input
	unique_name = FALSE;
	while (TRUE != unique_name)
	{
		// Get the filename to export to
		if (gtk_dialog_run(GTK_DIALOG(export_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(export_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

		// Check if there's an existing file of this name, and give an Overwrite? type prompt if there is
		if (TRUE == g_file_test(filename, G_FILE_TEST_EXISTS))
		{
			// Something with this name already exists
			warn_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
								GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_YES_NO,
								"Overwrite existing file?");
			if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
			{
				// We've been told to overwrite the existing file
				unique_name = TRUE;
			}
			gtk_widget_destroy(warn_dialog);
		} else
		{
			// The indicated file name is unique, we're fine to save
			unique_name = TRUE;
		}
	}

	// * We only get to here if a file was chosen *

	// Get the filename from the dialog box
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(export_dialog));

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(export_dialog);

	// Open output file for writing
	output_file = g_io_channel_new_file(filename, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED08: An error '%s' occured when opening '%s' for writing", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Write the SVG header to the output file
	g_string_assign(tmp_gstring, "<svg version=\"1.1\"\n\tbaseProfile=\"full\"\n\txmlns=\"http://www.w3.org/2000/svg\"\n\txmlns:xlink=\"http://www.w3.org/1999/xlink\"\n\txmlns:ev=\"http://www.w3.org/2001/xml-events\"\n\tonload=\"init(evt)\">\n");
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG header to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED10: An error '%s' occured when writing the SVG header to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Write a tag to advertise the SVG was created with Flame
	g_string_assign(tmp_gstring, "<!-- Created with the Flame Project (http://www.flameproject.org) -->\n");
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG header to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED50: An error '%s' occured when writing to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// The control bar script for pausing and un-pausing the animation
	g_string_assign(tmp_gstring,
		"<script><![CDATA[\n"
		"\tvar SVGDocument = null;\n"
		"\tvar SVGRoot = null;\n"
		"\tvar svgns = 'http://www.w3.org/2000/svg';\n"
		"\tvar xlinkns = 'http://www.w3.org/1999/xlink';\n"
		"\tvar controls_button_pause = null;\n"
		"\tvar controls_button_play = null;\n"
		"\n"
		"\tfunction init(evt)\n"
		"\t{\n"
		"\t\tSVGDocument = evt.target.ownerDocument;\n"
		"\t\tSVGRoot = SVGDocument.documentElement;\n"
		"\t\tcontrols_button_pause = SVGDocument.getElementById('controls_pause');\n"
		"\t\tcontrols_button_play = SVGDocument.getElementById('controls_play');\n"
		"\t\tcontrol_pause();\n"
		"\t};\n"
		"\n"
		"\tfunction control_pause()\n"
		"\t{\n"
		"\t\tSVGRoot.pauseAnimations();\n"
		"\t\tcontrols_button_pause.setAttributeNS(null, 'display', 'none');\n"
		"\t\tcontrols_button_play.setAttributeNS(null, 'display', 'inline');\n"
		"\t};\n"
		"\n"
		"\tfunction control_rewind()\n"
		"\t{\n"
		"\t\tSVGRoot.setCurrentTime(0.0);\n"
		"\t\tSVGRoot.pauseAnimations();\n"
		"\t\tcontrols_button_pause.setAttributeNS(null, 'display', 'none');\n"
		"\t\tcontrols_button_play.setAttributeNS(null, 'display', 'inline');\n"
		"\t};\n"
		"\n"
		"\tfunction control_unpause()\n"
		"\t{\n"
		"\t\tSVGRoot.unpauseAnimations();\n"
		"\t\tcontrols_button_play.setAttributeNS(null, 'display', 'none');\n"
		"\t\tcontrols_button_pause.setAttributeNS(null, 'display', 'inline');\n"
		"\t};\n"
		"\n"
		"]]></script>\n"
		"\n");
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED61: An error '%s' occured when writing to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Calculate scaling amounts to use for the control bar
	x_scale = ((gfloat) output_width / project_width);
	y_scale = ((gfloat) output_height / project_height);

	// Position the play button
	control_bar_x_offset = (output_width / 2) - (x_scale * 45);
	control_bar_y_offset = output_height - (y_scale * 70);

	// Write element definitions to the output file
	g_string_printf(tmp_gstring, "<defs>\n"
		"\t<style type=\"text/css\">\n"
		"<![CDATA[ rect.highlight {\n"
		"\t\t\tfill: #0f0;\n"
		"\t\t\tfill-opacity: 0.25;\n"
		"\t\t\tstroke: #0f0;\n"
		"\t\t\tstroke-linejoin: round;\n"
		"\t\t\tstroke-dasharray: none;\n"
		"\t\t\tstroke-opacity: 0.8; }\n"

		"\t\trect.text {\n"
		"\t\t\tfill: #ffc;\n"
		"\t\t\tfill-opacity: 1.0;\n"
		"\t\t\tstroke: black;\n"
		"\t\t\tstroke-linejoin: round;\n"
		"\t\t\tstroke-dasharray: none;\n"
		"\t\t\tstroke-opacity: 0.8; }\n"

		"\t\ttext.text {\n"
		"\t\t\ttext-anchor: start;\n"
		"\t\t\talignment-baseline: baseline; }\n"
		"]]>\n"
		"\t</style>\n"

// fixme4: Need a way to read the font face from the font file and only embed the required glyphs in the output
// Standard Bitstream Vera
"<font horiz-adv-x=\"1038\" >\n"
"<font-face font-family=\"Bitstream Vera Sans svg\" units-per-em=\"2048\" panose-1=\"2 11 6 3 3 8 4 2 2 4\" ascent=\"1901\" descent=\"-483\" alphabetic=\"0\" />\n"
"<missing-glyph horiz-adv-x=\"1229\" d=\"M102 -362V1444H1126V-362H102ZM217 -248H1012V1329H217V-248Z\" />\n"
"<glyph unicode=\" \" glyph-name=\"space\" horiz-adv-x=\"651\" />\n"
"<glyph unicode=\"!\" glyph-name=\"exclam\" horiz-adv-x=\"821\" d=\"M309 254H512V0H309V254ZM309 1493H512V838L492 481H330L309 838V1493Z\" />\n"
"<glyph unicode=\"&quot;\" glyph-name=\"quotedbl\" horiz-adv-x=\"942\" d=\"M367 1493V938H197V1493H367ZM745 1493V938H575V1493H745Z\" />\n"
"<glyph unicode=\"#\" glyph-name=\"numbersign\" horiz-adv-x=\"1716\" d=\"M1047 901H756L672 567H965L1047 901ZM897 1470L793 1055H1085L1190 1470H1350L1247 1055H1559V901H1208L1126 567H1444V414H1087L983 0H823L926 414H633L530 0H369L473 414H158V567H510L594 901H272V1055H633L735 1470H897Z\" />\n"
"<glyph unicode=\"$\" glyph-name=\"dollar\" horiz-adv-x=\"1303\" d=\"M692 -301H592L591 0Q486 2 381 24T170 92V272Q272 208 376 176T592 142V598Q371 634 271 720T170 956Q170 1119 279 1213T592 1321V1556H692V1324Q785 1320 872 1305T1042 1262V1087Q959 1129 872 1152T692 1179V752Q919 717 1026 627T1133 381Q1133 212 1020 115T692 2V-301ZM592 770V1180Q476 1167 415 1114T354 973Q354 887 410 839T592 770ZM692 578V145Q819 162 883 217T948 362Q948 450 887 502T692 578Z\" />\n"
"<glyph unicode=\"%%\" glyph-name=\"percent\" horiz-adv-x=\"1946\" d=\"M1489 657Q1402 657 1353 583T1303 377Q1303 247 1352 173T1489 98Q1574 98 1623 172T1673 377Q1673 508 1624 582T1489 657ZM1489 784Q1647 784 1740 674T1833 377Q1833 190 1740 81T1489 -29Q1329 -29 1236 80T1143 377Q1143 565 1236 674T1489 784ZM457 1393Q371 1393 322 1319T272 1114Q272 982 321 908T457 834Q544 834 593 908T643 1114Q643 1243 593 1318T457 1393ZM1360 1520H1520L586 -29H426L1360 1520ZM457 1520Q615 1520 709 1411T803 1114Q803 925 710 816T457 707Q298 707 206 816T113 1114Q113 1300 206 1410T457 1520Z\" />\n"
"<glyph unicode=\"&amp;\" glyph-name=\"ampersand\" horiz-adv-x=\"1597\" d=\"M498 803Q407 722 365 642T322 473Q322 327 428 230T694 133Q789 133 872 164T1028 260L498 803ZM639 915L1147 395Q1206 484 1239 585T1278 801H1464Q1452 669 1400 540T1255 285L1534 0H1282L1139 147Q1035 58 921 15T676 -29Q435 -29 282 108T129 461Q129 589 196 701T397 913Q349 976 324 1038T299 1161Q299 1323 410 1421T705 1520Q788 1520 870 1502T1038 1448V1266Q951 1313 872 1337T725 1362Q620 1362 555 1307T489 1163Q489 1112 518 1061T639 915Z\" />\n"
"<glyph unicode=\"&apos;\" glyph-name=\"quotesingle\" horiz-adv-x=\"563\" d=\"M367 1493V938H197V1493H367Z\" />\n"
"<glyph unicode=\"(\" glyph-name=\"parenleft\" horiz-adv-x=\"799\" d=\"M635 1554Q501 1324 436 1099T371 643Q371 412 436 186T635 -270H475Q325 -35 251 192T176 643Q176 866 250 1092T475 1554H635Z\" />\n"
"<glyph unicode=\")\" glyph-name=\"parenright\" horiz-adv-x=\"799\" d=\"M164 1554H324Q474 1318 548 1092T623 643Q623 419 549 192T324 -270H164Q297 -41 362 185T428 643Q428 874 363 1099T164 1554Z\" />\n"
"<glyph unicode=\"*\" glyph-name=\"asterisk\" horiz-adv-x=\"1024\" d=\"M963 1247L604 1053L963 858L905 760L569 963V586H455V963L119 760L61 858L420 1053L61 1247L119 1346L455 1143V1520H569V1143L905 1346L963 1247Z\" />\n"
"<glyph unicode=\"+\" glyph-name=\"plus\" horiz-adv-x=\"1716\" d=\"M942 1284V727H1499V557H942V0H774V557H217V727H774V1284H942Z\" />\n"
"<glyph unicode=\",\" glyph-name=\"comma\" horiz-adv-x=\"651\" d=\"M240 254H451V82L287 -238H158L240 82V254Z\" />\n"
"<glyph unicode=\"-\" glyph-name=\"hyphen\" horiz-adv-x=\"739\" d=\"M100 643H639V479H100V643Z\" />\n"
"<glyph unicode=\".\" glyph-name=\"period\" horiz-adv-x=\"651\" d=\"M219 254H430V0H219V254Z\" />\n"
"<glyph unicode=\"/\" glyph-name=\"slash\" horiz-adv-x=\"690\" d=\"M520 1493H690L170 -190H0L520 1493Z\" />\n"
"<glyph unicode=\"0\" glyph-name=\"zero\" horiz-adv-x=\"1303\" d=\"M651 1360Q495 1360 417 1207T338 745Q338 438 416 285T651 131Q808 131 886 284T965 745Q965 1053 887 1206T651 1360ZM651 1520Q902 1520 1034 1322T1167 745Q1167 368 1035 170T651 -29Q400 -29 268 169T135 745Q135 1123 267 1321T651 1520Z\" />\n"
"<glyph unicode=\"1\" glyph-name=\"one\" horiz-adv-x=\"1303\" d=\"M254 170H584V1309L225 1237V1421L582 1493H784V170H1114V0H254V170Z\" />\n"
"<glyph unicode=\"2\" glyph-name=\"two\" horiz-adv-x=\"1303\" d=\"M393 170H1098V0H150V170Q265 289 463 489T713 748Q810 857 848 932T887 1081Q887 1200 804 1275T586 1350Q491 1350 386 1317T160 1217V1421Q282 1470 388 1495T582 1520Q814 1520 952 1404T1090 1094Q1090 1002 1056 920T930 725Q905 696 771 558T393 170Z\" />\n"
"<glyph unicode=\"3\" glyph-name=\"three\" horiz-adv-x=\"1303\" d=\"M831 805Q976 774 1057 676T1139 434Q1139 213 987 92T555 -29Q461 -29 362 -11T156 45V240Q240 191 340 166T549 141Q739 141 838 216T938 434Q938 566 846 640T588 715H414V881H596Q745 881 824 940T903 1112Q903 1227 822 1288T588 1350Q505 1350 410 1332T201 1276V1456Q316 1488 416 1504T606 1520Q836 1520 970 1416T1104 1133Q1104 1009 1033 924T831 805Z\" />\n"
"<glyph unicode=\"4\" glyph-name=\"four\" horiz-adv-x=\"1303\" d=\"M774 1317L264 520H774V1317ZM721 1493H975V520H1188V352H975V0H774V352H100V547L721 1493Z\" />\n"
"<glyph unicode=\"5\" glyph-name=\"five\" horiz-adv-x=\"1303\" d=\"M221 1493H1014V1323H406V957Q450 972 494 979T582 987Q832 987 978 850T1124 479Q1124 238 974 105T551 -29Q457 -29 360 -13T158 35V238Q248 189 344 165T547 141Q720 141 821 232T922 479Q922 635 821 726T547 817Q466 817 386 799T221 743V1493Z\" />\n"
"<glyph unicode=\"6\" glyph-name=\"six\" horiz-adv-x=\"1303\" d=\"M676 827Q540 827 461 734T381 479Q381 318 460 225T676 131Q812 131 891 224T971 479Q971 641 892 734T676 827ZM1077 1460V1276Q1001 1312 924 1331T770 1350Q570 1350 465 1215T344 807Q403 894 492 940T688 987Q913 987 1043 851T1174 479Q1174 249 1038 110T676 -29Q417 -29 280 169T143 745Q143 1099 311 1309T762 1520Q838 1520 915 1505T1077 1460Z\" />\n"
"<glyph unicode=\"7\" glyph-name=\"seven\" horiz-adv-x=\"1303\" d=\"M168 1493H1128V1407L586 0H375L885 1323H168V1493Z\" />\n"
"<glyph unicode=\"8\" glyph-name=\"eight\" horiz-adv-x=\"1303\" d=\"M651 709Q507 709 425 632T342 420Q342 285 424 208T651 131Q795 131 878 208T961 420Q961 555 879 632T651 709ZM449 795Q319 827 247 916T174 1133Q174 1312 301 1416T651 1520Q874 1520 1001 1416T1128 1133Q1128 1005 1056 916T854 795Q1000 761 1081 662T1163 420Q1163 203 1031 87T651 -29Q404 -29 272 87T139 420Q139 563 221 662T449 795ZM375 1114Q375 998 447 933T651 868Q781 868 854 933T928 1114Q928 1230 855 1295T651 1360Q520 1360 448 1295T375 1114Z\" />\n"
"<glyph unicode=\"9\" glyph-name=\"nine\" horiz-adv-x=\"1303\" d=\"M225 31V215Q301 179 379 160T532 141Q732 141 837 275T958 684Q900 598 811 552T614 506Q390 506 260 641T129 1012Q129 1242 265 1381T627 1520Q886 1520 1022 1322T1159 745Q1159 392 992 182T541 -29Q465 -29 387 -14T225 31ZM627 664Q763 664 842 757T922 1012Q922 1173 843 1266T627 1360Q491 1360 412 1267T332 1012Q332 850 411 757T627 664Z\" />\n"
"<glyph unicode=\":\" glyph-name=\"colon\" horiz-adv-x=\"690\" d=\"M240 254H451V0H240V254ZM240 1059H451V805H240V1059Z\" />\n"
"<glyph unicode=\";\" glyph-name=\"semicolon\" horiz-adv-x=\"690\" d=\"M240 1059H451V805H240V1059ZM240 254H451V82L287 -238H158L240 82V254Z\" />\n"
"<glyph unicode=\"&lt;\" glyph-name=\"less\" horiz-adv-x=\"1716\" d=\"M1499 1008L467 641L1499 276V94L217 559V725L1499 1190V1008Z\" />\n"
"<glyph unicode=\"=\" glyph-name=\"equal\" horiz-adv-x=\"1716\" d=\"M217 930H1499V762H217V930ZM217 522H1499V352H217V522Z\" />\n"
"<glyph unicode=\"&gt;\" glyph-name=\"greater\" horiz-adv-x=\"1716\" d=\"M217 1008V1190L1499 725V559L217 94V276L1247 641L217 1008Z\" />\n"
"<glyph unicode=\"?\" glyph-name=\"question\" horiz-adv-x=\"1087\" d=\"M391 254H594V0H391V254ZM588 401H397V555Q397 656 425 721T543 872L633 961Q690 1014 715 1061T741 1157Q741 1246 676 1301T502 1356Q423 1356 334 1321T147 1219V1407Q241 1464 337 1492T537 1520Q721 1520 832 1423T944 1167Q944 1091 908 1023T782 868L694 782Q647 735 628 709T600 657Q594 636 591 606T588 524V401Z\" />\n"
"<glyph unicode=\"@\" glyph-name=\"at\" horiz-adv-x=\"2048\" d=\"M762 537Q762 394 833 313T1028 231Q1151 231 1221 313T1292 537Q1292 677 1220 759T1026 842Q905 842 834 760T762 537ZM1307 238Q1247 161 1170 125T989 88Q817 88 710 212T602 537Q602 737 710 862T989 987Q1092 987 1170 950T1307 836V967H1450V231Q1596 253 1678 364T1761 653Q1761 760 1730 854T1634 1028Q1530 1159 1381 1228T1055 1298Q932 1298 819 1266T610 1169Q453 1067 365 902T276 543Q276 384 333 245T500 0Q605 -104 743 -158T1038 -213Q1167 -213 1291 -170T1520 -45L1610 -156Q1485 -253 1338 -304T1038 -356Q853 -356 689 -291T397 -100Q269 25 202 189T135 543Q135 725 203 890T397 1180Q526 1307 695 1374T1053 1442Q1265 1442 1446 1355T1751 1108Q1826 1010 1865 895T1905 657Q1905 394 1746 242T1307 84V238Z\" />\n"
"<glyph unicode=\"A\" glyph-name=\"A\" horiz-adv-x=\"1401\" d=\"M700 1294L426 551H975L700 1294ZM586 1493H815L1384 0H1174L1038 383H365L229 0H16L586 1493Z\" />\n"
"<glyph unicode=\"B\" glyph-name=\"B\" horiz-adv-x=\"1405\" d=\"M403 713V166H727Q890 166 968 233T1047 440Q1047 580 969 646T727 713H403ZM403 1327V877H702Q850 877 922 932T995 1102Q995 1215 923 1271T702 1327H403ZM201 1493H717Q948 1493 1073 1397T1198 1124Q1198 987 1134 906T946 805Q1095 773 1177 672T1260 418Q1260 218 1124 109T737 0H201V1493Z\" />\n"
"<glyph unicode=\"C\" glyph-name=\"C\" horiz-adv-x=\"1430\" d=\"M1319 1378V1165Q1217 1260 1102 1307T856 1354Q600 1354 464 1198T328 745Q328 450 464 294T856 137Q986 137 1101 184T1319 326V115Q1213 43 1095 7T844 -29Q505 -29 310 178T115 745Q115 1105 310 1312T844 1520Q978 1520 1096 1485T1319 1378Z\" />\n"
"<glyph unicode=\"D\" glyph-name=\"D\" horiz-adv-x=\"1577\" d=\"M403 1327V166H647Q956 166 1099 306T1243 748Q1243 1048 1100 1187T647 1327H403ZM201 1493H616Q1050 1493 1253 1313T1456 748Q1456 362 1252 181T616 0H201V1493Z\" />\n"
"<glyph unicode=\"E\" glyph-name=\"E\" horiz-adv-x=\"1294\" d=\"M201 1493H1145V1323H403V881H1114V711H403V170H1163V0H201V1493Z\" />\n"
"<glyph unicode=\"F\" glyph-name=\"F\" horiz-adv-x=\"1178\" d=\"M201 1493H1059V1323H403V883H995V713H403V0H201V1493Z\" />\n"
"<glyph unicode=\"G\" glyph-name=\"G\" horiz-adv-x=\"1587\" d=\"M1219 213V614H889V780H1419V139Q1302 56 1161 14T860 -29Q510 -29 313 175T115 745Q115 1111 312 1315T860 1520Q1006 1520 1137 1484T1380 1378V1163Q1268 1258 1142 1306T877 1354Q603 1354 466 1201T328 745Q328 443 465 290T877 137Q984 137 1068 155T1219 213Z\" />\n"
"<glyph unicode=\"H\" glyph-name=\"H\" horiz-adv-x=\"1540\" d=\"M201 1493H403V881H1137V1493H1339V0H1137V711H403V0H201V1493Z\" />\n"
"<glyph unicode=\"I\" glyph-name=\"I\" horiz-adv-x=\"604\" d=\"M201 1493H403V0H201V1493Z\" />\n"
"<glyph unicode=\"J\" glyph-name=\"J\" horiz-adv-x=\"604\" d=\"M201 1493H403V104Q403 -166 301 -288T-29 -410H-106V-240H-43Q91 -240 146 -165T201 104V1493Z\" />\n"
"<glyph unicode=\"K\" glyph-name=\"K\" horiz-adv-x=\"1343\" d=\"M201 1493H403V862L1073 1493H1333L592 797L1386 0H1120L403 719V0H201V1493Z\" />\n"
"<glyph unicode=\"L\" glyph-name=\"L\" horiz-adv-x=\"1141\" d=\"M201 1493H403V170H1130V0H201V1493Z\" />\n"
"<glyph unicode=\"M\" glyph-name=\"M\" horiz-adv-x=\"1767\" d=\"M201 1493H502L883 477L1266 1493H1567V0H1370V1311L985 287H782L397 1311V0H201V1493Z\" />\n"
"<glyph unicode=\"N\" glyph-name=\"N\" horiz-adv-x=\"1532\" d=\"M201 1493H473L1135 244V1493H1331V0H1059L397 1249V0H201V1493Z\" />\n"
"<glyph unicode=\"O\" glyph-name=\"O\" horiz-adv-x=\"1612\" d=\"M807 1356Q587 1356 458 1192T328 745Q328 463 457 299T807 135Q1027 135 1155 299T1284 745Q1284 1028 1156 1192T807 1356ZM807 1520Q1121 1520 1309 1310T1497 745Q1497 392 1309 182T807 -29Q492 -29 304 181T115 745Q115 1099 303 1309T807 1520Z\" />\n"
"<glyph unicode=\"P\" glyph-name=\"P\" horiz-adv-x=\"1235\" d=\"M403 1327V766H657Q798 766 875 839T952 1047Q952 1181 875 1254T657 1327H403ZM201 1493H657Q908 1493 1036 1380T1165 1047Q1165 826 1037 713T657 600H403V0H201V1493Z\" />\n"
"<glyph unicode=\"Q\" glyph-name=\"Q\" horiz-adv-x=\"1612\" d=\"M807 1356Q587 1356 458 1192T328 745Q328 463 457 299T807 135Q1027 135 1155 299T1284 745Q1284 1028 1156 1192T807 1356ZM1090 27L1356 -264H1112L891 -25Q858 -27 841 -28T807 -29Q492 -29 304 181T115 745Q115 1099 303 1309T807 1520Q1121 1520 1309 1310T1497 745Q1497 485 1393 300T1090 27Z\" />\n"
"<glyph unicode=\"R\" glyph-name=\"R\" horiz-adv-x=\"1423\" d=\"M909 700Q974 678 1035 606T1159 408L1364 0H1147L956 383Q882 533 813 582T623 631H403V0H201V1493H657Q913 1493 1039 1386T1165 1063Q1165 922 1100 829T909 700ZM403 1327V797H657Q803 797 877 864T952 1063Q952 1194 878 1260T657 1327H403Z\" />\n"
"<glyph unicode=\"S\" glyph-name=\"S\" horiz-adv-x=\"1300\" d=\"M1096 1444V1247Q981 1302 879 1329T682 1356Q517 1356 428 1292T338 1110Q338 1011 397 961T623 879L745 854Q971 811 1078 703T1186 412Q1186 195 1041 83T614 -29Q508 -29 389 -5T141 66V274Q264 205 382 170T614 135Q787 135 881 203T975 397Q975 507 908 569T686 662L563 686Q337 731 236 827T135 1094Q135 1292 274 1406T659 1520Q764 1520 873 1501T1096 1444Z\" />\n"
"<glyph unicode=\"T\" glyph-name=\"T\" horiz-adv-x=\"1251\" d=\"M-6 1493H1257V1323H727V0H524V1323H-6V1493Z\" />\n"
"<glyph unicode=\"U\" glyph-name=\"U\" horiz-adv-x=\"1499\" d=\"M178 1493H381V586Q381 346 468 241T750 135Q944 135 1031 240T1118 586V1493H1321V561Q1321 269 1177 120T750 -29Q467 -29 323 120T178 561V1493Z\" />\n"
"<glyph unicode=\"V\" glyph-name=\"V\" horiz-adv-x=\"1401\" d=\"M586 0L16 1493H227L700 236L1174 1493H1384L815 0H586Z\" />\n"
"<glyph unicode=\"W\" glyph-name=\"W\" horiz-adv-x=\"2025\" d=\"M68 1493H272L586 231L899 1493H1126L1440 231L1753 1493H1958L1583 0H1329L1014 1296L696 0H442L68 1493Z\" />\n"
"<glyph unicode=\"X\" glyph-name=\"X\" horiz-adv-x=\"1403\" d=\"M129 1493H346L717 938L1090 1493H1307L827 776L1339 0H1122L702 635L279 0H61L594 797L129 1493Z\" />\n"
"<glyph unicode=\"Y\" glyph-name=\"Y\" horiz-adv-x=\"1251\" d=\"M-4 1493H213L627 879L1038 1493H1255L727 711V0H524V711L-4 1493Z\" />\n"
"<glyph unicode=\"Z\" glyph-name=\"Z\" horiz-adv-x=\"1403\" d=\"M115 1493H1288V1339L344 170H1311V0H92V154L1036 1323H115V1493Z\" />\n"
"<glyph unicode=\"[\" glyph-name=\"bracketleft\" horiz-adv-x=\"799\" d=\"M176 1556H600V1413H360V-127H600V-270H176V1556Z\" />\n"
"<glyph unicode=\"\\\" glyph-name=\"backslash\" horiz-adv-x=\"690\" d=\"M170 1493L690 -190H520L0 1493H170Z\" />\n"
"<glyph unicode=\"]\" glyph-name=\"bracketright\" horiz-adv-x=\"799\" d=\"M623 1556V-270H199V-127H438V1413H199V1556H623Z\" />\n"
"<glyph unicode=\"^\" glyph-name=\"asciicircum\" horiz-adv-x=\"1716\" d=\"M956 1493L1499 936H1298L858 1331L418 936H217L760 1493H956Z\" />\n"
"<glyph unicode=\"_\" glyph-name=\"underscore\" horiz-adv-x=\"1024\" d=\"M1044 -340V-483H-20V-340H1044Z\" />\n"
"<glyph unicode=\"`\" glyph-name=\"grave\" horiz-adv-x=\"1024\" d=\"M367 1638L649 1264H496L170 1638H367Z\" />\n"
"<glyph unicode=\"a\" glyph-name=\"a\" horiz-adv-x=\"1255\" d=\"M702 563Q479 563 393 512T307 338Q307 240 371 183T547 125Q700 125 792 233T885 522V563H702ZM1069 639V0H885V170Q822 68 728 20T498 -29Q326 -29 225 67T123 326Q123 515 249 611T627 707H885V725Q885 852 802 921T567 991Q471 991 380 968T205 899V1069Q306 1108 401 1127T586 1147Q829 1147 949 1021T1069 639Z\" />\n"
"<glyph unicode=\"b\" glyph-name=\"b\" horiz-adv-x=\"1300\" d=\"M997 559Q997 762 914 877T684 993Q538 993 455 878T371 559Q371 356 454 241T684 125Q830 125 913 240T997 559ZM371 950Q429 1050 517 1098T729 1147Q933 1147 1060 985T1188 559Q1188 295 1061 133T729 -29Q606 -29 518 19T371 168V0H186V1556H371V950Z\" />\n"
"<glyph unicode=\"c\" glyph-name=\"c\" horiz-adv-x=\"1126\" d=\"M999 1077V905Q921 948 843 969T684 991Q505 991 406 878T307 559Q307 354 406 241T684 127Q764 127 842 148T999 213V43Q922 7 840 -11T664 -29Q411 -29 262 130T113 559Q113 833 263 990T676 1147Q761 1147 842 1130T999 1077Z\" />\n"
"<glyph unicode=\"d\" glyph-name=\"d\" horiz-adv-x=\"1300\" d=\"M930 950V1556H1114V0H930V168Q872 68 784 20T571 -29Q368 -29 241 133T113 559Q113 823 240 985T571 1147Q695 1147 783 1099T930 950ZM303 559Q303 356 386 241T616 125Q762 125 846 240T930 559Q930 762 846 877T616 993Q470 993 387 878T303 559Z\" />\n"
"<glyph unicode=\"e\" glyph-name=\"e\" horiz-adv-x=\"1260\" d=\"M1151 606V516H305Q317 326 419 227T705 127Q811 127 910 153T1108 231V57Q1009 15 905 -7T694 -29Q426 -29 270 127T113 549Q113 824 261 985T662 1147Q888 1147 1019 1002T1151 606ZM967 660Q965 811 883 901T664 991Q510 991 418 904T311 659L967 660Z\" />\n"
"<glyph unicode=\"f\" glyph-name=\"f\" horiz-adv-x=\"721\" d=\"M760 1556V1403H584Q485 1403 447 1363T408 1219V1120H711V977H408V0H223V977H47V1120H223V1198Q223 1385 310 1470T586 1556H760Z\" />\n"
"<glyph unicode=\"g\" glyph-name=\"g\" horiz-adv-x=\"1300\" d=\"M930 573Q930 773 848 883T616 993Q468 993 386 883T303 573Q303 374 385 264T616 154Q765 154 847 264T930 573ZM1114 139Q1114 -147 987 -286T598 -426Q501 -426 415 -412T248 -367V-188Q329 -232 408 -253T569 -274Q750 -274 840 -180T930 106V197Q873 98 784 49T571 0Q365 0 239 157T113 573Q113 833 239 990T571 1147Q695 1147 784 1098T930 950V1120H1114V139Z\" />\n"
"<glyph unicode=\"h\" glyph-name=\"h\" horiz-adv-x=\"1298\" d=\"M1124 676V0H940V670Q940 829 878 908T692 987Q543 987 457 892T371 633V0H186V1556H371V946Q437 1047 526 1097T733 1147Q926 1147 1025 1028T1124 676Z\" />\n"
"<glyph unicode=\"i\" glyph-name=\"i\" horiz-adv-x=\"569\" d=\"M193 1120H377V0H193V1120ZM193 1556H377V1323H193V1556Z\" />\n"
"<glyph unicode=\"j\" glyph-name=\"j\" horiz-adv-x=\"569\" d=\"M193 1120H377V-20Q377 -234 296 -330T33 -426H-37V-270H12Q117 -270 155 -222T193 -20V1120ZM193 1556H377V1323H193V1556Z\" />\n"
"<glyph unicode=\"k\" glyph-name=\"k\" horiz-adv-x=\"1186\" d=\"M186 1556H371V637L920 1120H1155L561 596L1180 0H940L371 547V0H186V1556Z\" />\n"
"<glyph unicode=\"l\" glyph-name=\"l\" horiz-adv-x=\"569\" d=\"M193 1556H377V0H193V1556Z\" />\n"
"<glyph unicode=\"m\" glyph-name=\"m\" horiz-adv-x=\"1995\" d=\"M1065 905Q1134 1029 1230 1088T1456 1147Q1631 1147 1726 1025T1821 676V0H1636V670Q1636 831 1579 909T1405 987Q1262 987 1179 892T1096 633V0H911V670Q911 832 854 909T678 987Q537 987 454 892T371 633V0H186V1120H371V946Q434 1049 522 1098T731 1147Q853 1147 938 1085T1065 905Z\" />\n"
"<glyph unicode=\"n\" glyph-name=\"n\" horiz-adv-x=\"1298\" d=\"M1124 676V0H940V670Q940 829 878 908T692 987Q543 987 457 892T371 633V0H186V1120H371V946Q437 1047 526 1097T733 1147Q926 1147 1025 1028T1124 676Z\" />\n"
"<glyph unicode=\"o\" glyph-name=\"o\" horiz-adv-x=\"1253\" d=\"M627 991Q479 991 393 876T307 559Q307 358 392 243T627 127Q774 127 860 243T946 559Q946 758 860 874T627 991ZM627 1147Q867 1147 1004 991T1141 559Q1141 284 1004 128T627 -29Q386 -29 250 127T113 559Q113 835 249 991T627 1147Z\" />\n"
"<glyph unicode=\"p\" glyph-name=\"p\" horiz-adv-x=\"1300\" d=\"M371 168V-426H186V1120H371V950Q429 1050 517 1098T729 1147Q933 1147 1060 985T1188 559Q1188 295 1061 133T729 -29Q606 -29 518 19T371 168ZM997 559Q997 762 914 877T684 993Q538 993 455 878T371 559Q371 356 454 241T684 125Q830 125 913 240T997 559Z\" />\n"
"<glyph unicode=\"q\" glyph-name=\"q\" horiz-adv-x=\"1300\" d=\"M303 559Q303 356 386 241T616 125Q762 125 846 240T930 559Q930 762 846 877T616 993Q470 993 387 878T303 559ZM930 168Q872 68 784 20T571 -29Q368 -29 241 133T113 559Q113 823 240 985T571 1147Q695 1147 783 1099T930 950V1120H1114V-426H930V168Z\" />\n"
"<glyph unicode=\"r\" glyph-name=\"r\" horiz-adv-x=\"842\" d=\"M842 948Q811 966 775 974T694 983Q538 983 455 882T371 590V0H186V1120H371V946Q429 1048 522 1097T748 1147Q767 1147 790 1145T841 1137L842 948Z\" />\n"
"<glyph unicode=\"s\" glyph-name=\"s\" horiz-adv-x=\"1067\" d=\"M907 1087V913Q829 953 745 973T571 993Q434 993 366 951T297 825Q297 761 346 725T543 655L606 641Q802 599 884 523T967 309Q967 153 844 62T504 -29Q414 -29 317 -12T111 41V231Q213 178 312 152T508 125Q638 125 708 169T778 295Q778 370 728 410T506 487L442 502Q271 538 195 612T119 817Q119 975 231 1061T549 1147Q651 1147 741 1132T907 1087Z\" />\n"
"<glyph unicode=\"t\" glyph-name=\"t\" horiz-adv-x=\"803\" d=\"M375 1438V1120H754V977H375V369Q375 232 412 193T565 154H754V0H565Q352 0 271 79T190 369V977H55V1120H190V1438H375Z\" />\n"
"<glyph unicode=\"u\" glyph-name=\"u\" horiz-adv-x=\"1298\" d=\"M174 442V1120H358V449Q358 290 420 211T606 131Q755 131 841 226T928 485V1120H1112V0H928V172Q861 70 773 21T567 -29Q374 -29 274 91T174 442Z\" />\n"
"<glyph unicode=\"v\" glyph-name=\"v\" horiz-adv-x=\"1212\" d=\"M61 1120H256L606 180L956 1120H1151L731 0H481L61 1120Z\" />\n"
"<glyph unicode=\"w\" glyph-name=\"w\" horiz-adv-x=\"1675\" d=\"M86 1120H270L500 246L729 1120H946L1176 246L1405 1120H1589L1296 0H1079L838 918L596 0H379L86 1120Z\" />\n"
"<glyph unicode=\"x\" glyph-name=\"x\" horiz-adv-x=\"1212\" d=\"M1124 1120L719 575L1145 0H928L602 440L276 0H59L494 586L96 1120H313L610 721L907 1120H1124Z\" />\n"
"<glyph unicode=\"y\" glyph-name=\"y\" horiz-adv-x=\"1212\" d=\"M659 -104Q581 -304 507 -365T309 -426H162V-272H270Q346 -272 388 -236T481 -66L514 18L61 1120H256L606 244L956 1120H1151L659 -104Z\" />\n"
"<glyph unicode=\"z\" glyph-name=\"z\" horiz-adv-x=\"1075\" d=\"M113 1120H987V952L295 147H987V0H88V168L780 973H113V1120Z\" />\n"
"<glyph unicode=\"{\" glyph-name=\"braceleft\" horiz-adv-x=\"1303\" d=\"M1047 -190V-334H985Q736 -334 652 -260T567 35V274Q567 425 513 483T317 541H256V684H317Q460 684 513 741T567 948V1188Q567 1409 651 1482T985 1556H1047V1413H979Q838 1413 795 1369T752 1184V936Q752 779 707 708T551 612Q662 585 707 514T752 287V39Q752 -102 795 -146T979 -190H1047Z\" />\n"
"<glyph unicode=\"|\" glyph-name=\"bar\" horiz-adv-x=\"690\" d=\"M430 1565V-483H260V1565H430Z\" />\n"
"<glyph unicode=\"}\" glyph-name=\"braceright\" horiz-adv-x=\"1303\" d=\"M256 -190H326Q466 -190 508 -147T551 39V287Q551 443 596 514T752 612Q641 637 596 708T551 936V1184Q551 1326 509 1369T326 1413H256V1556H319Q568 1556 651 1483T735 1188V948Q735 799 789 742T985 684H1047V541H985Q843 541 789 483T735 274V35Q735 -186 652 -260T319 -334H256V-190Z\" />\n"
"<glyph unicode=\"~\" glyph-name=\"asciitilde\" horiz-adv-x=\"1716\" d=\"M1499 817V639Q1394 560 1305 526T1118 492Q1008 492 862 551Q851 555 846 557Q839 560 824 565Q669 627 575 627Q487 627 401 589T217 467V645Q322 724 411 758T598 793Q708 793 855 733Q865 729 870 727Q878 724 892 719Q1047 657 1141 657Q1227 657 1311 695T1499 817Z\" />\n"
"</font>\n"

		// * Control bar button definitions *

		// Rewind button
		"<g id=\"controls_rewind\">\n"
		"\t<circle cx=\"%.4fpx\" cy=\"%.4fpx\" r=\"%.4fpx\""
		" stroke-width=\"%.4fpx\" stroke-opacity=\"0.0\""
		" fill=\"#555\" fill-opacity=\"1.0\"/>"
		"\t<path"
		" d=\"M %.4f,%.4f C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f z"
		" M %.4f,%.4f L %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f L %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f L %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f L %.4f,%.4f"
		" L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f L %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f z\""
		" font-size=\"12\" fill=\"#b3b3b3\" fill-opacity=\"1\" fill-rule=\"evenodd\""
		" stroke=\"#222\" stroke-width=\"%.4fpx\" stroke-dasharray=\"none\""
		" stroke-opacity=\"1\" stroke-linejoin=\"round\" stroke-linecap=\"round\" />\n"
		"</g>\n"

		// Play button
		"<g id=\"controls_play\">\n"
		"\t<circle cx=\"%.4fpx\" cy=\"%.4fpx\" r=\"%.4fpx\""
		" stroke-width=\"%.4fpx\" stroke-opacity=\"0.0\""
		" fill=\"#555\" fill-opacity=\"1.0\"/>"
		"\t<path"
		" d=\"M %.4f %.4f C %.4f %.4f %.4f %.4f %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f z"
		" M %.4f %.4f L %.4f %.4f L %.4f %.4f"
		" L %.4f %.4f L %.4f %.4f L %.4f %.4f z\""
		" font-size=\"12\" fill=\"#b3b3b3\" fill-opacity=\"1\" fill-rule=\"evenodd\""
		" stroke=\"#222\" stroke-width=\"%.4fpx\" stroke-linecap=\"round\""
		" stroke-linejoin=\"round\" stroke-dasharray=\"none\" />\n"
		"</g>\n"

		// Pause button
		"<g id=\"controls_pause\">\n"
		"\t<circle cx=\"%.4fpx\" cy=\"%.4fpx\" r=\"%.4fpx\""
		" stroke-width=\"%.4fpx\" stroke-opacity=\"0.0\""
		" fill=\"#555\" fill-opacity=\"1.0\"/>"
		"\t<path"
		" d=\"M %.4f,%.4f C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f"
		" C %.4f,%.4f %.4f,%.4f %.4f,%.4f z"
		" M %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f z"
		" M %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f L %.4f %.4f"
		" C %.4f %.4f %.4f %.4f %.4f %.4f z\"\n"
		" font-size=\"12\" fill=\"#b3b3b3\" fill-opacity=\"1\" fill-rule=\"evenodd\"\n"
		" stroke=\"#222\" stroke-width=\"%.4fpx\" stroke-dasharray=\"none\"\n"
		" stroke-opacity=\"1\" stroke-linejoin=\"round\" stroke-linecap=\"round\" />\n"
		"</g>\n"

		"</defs>\n",

		// Rewind button (circle backdrop)
		x_scale * 30.281246,  // cx
		y_scale * 30.281246,  // cy
		x_scale * 25.0,  // r
		x_scale * 1.5,  // stroke-width

		// Rewind button (symbol)
		x_scale * 30.281246, y_scale * 4.6562500,

		x_scale * 44.328640, y_scale * 4.6562500,
		x_scale * 55.718750, y_scale * 16.046356,
		x_scale * 55.718750, y_scale * 30.093750,

		x_scale * 55.718750, y_scale * 44.141144,
		x_scale * 44.328640, y_scale * 55.531251,
		x_scale * 30.281246, y_scale * 55.531250,

		x_scale * 16.233852, y_scale * 55.531250,
		x_scale * 4.8437460, y_scale * 44.141145,
		x_scale * 4.8437460, y_scale * 30.093750,

		x_scale * 4.8437460, y_scale * 16.046356,
		x_scale * 16.233851, y_scale * 4.6562500,
		x_scale * 30.281246, y_scale * 4.6562500,

		x_scale * 25.906246, y_scale * 12.812500,
		x_scale * 19.624996, y_scale * 12.812500,

		x_scale * 19.365394, y_scale * 12.812500,
		x_scale * 19.156246, y_scale * 13.021648,
		x_scale * 19.156246, y_scale * 13.281250,
		x_scale * 19.156246, y_scale * 47.031250,

		x_scale * 19.156246, y_scale * 47.290852,
		x_scale * 19.365392, y_scale * 47.500001,
		x_scale * 19.624996, y_scale * 47.500000,
		x_scale * 25.906246, y_scale * 47.500000,

		x_scale * 26.165848, y_scale * 47.500000,
		x_scale * 26.374996, y_scale * 47.290851,
		x_scale * 26.374996, y_scale * 47.031250,
		x_scale * 26.374996, y_scale * 33.625000,

		x_scale * 40.093746, y_scale * 47.343750,
		x_scale * 40.093746, y_scale * 13.000000,
		x_scale * 26.374996, y_scale * 26.937500,
		x_scale * 26.374996, y_scale * 13.281250,

		x_scale * 26.374996, y_scale * 13.021648,
		x_scale * 26.165846, y_scale * 12.812500,
		x_scale * 25.906246, y_scale * 12.812500,

		x_scale * 1.5,

		// Play button (circle backdrop)
		x_scale * 30.281246,  // cx
		y_scale * 30.281246,  // cy
		x_scale * 25.0,  // r
		x_scale * 1.5,  // stroke-width

		// Play button (symbol)
		x_scale * 4.6873499, y_scale * 30.212826,

		x_scale * 4.7397333, y_scale * 44.317606,
		x_scale * 16.239717, y_scale * 55.701347,
		x_scale * 30.344494, y_scale * 55.648962,

		x_scale * 44.449275, y_scale * 55.596577,
		x_scale * 55.833133, y_scale * 44.127847,
		x_scale * 55.780747, y_scale * 30.023066,

		x_scale * 55.728362, y_scale * 15.918286,
		x_scale * 44.259515, y_scale * 4.5031785,
		x_scale * 30.154735, y_scale * 4.5555647,

		x_scale * 16.049955, y_scale * 4.6079497,
		x_scale * 4.6349645, y_scale * 16.108047,
		x_scale * 4.6873499, y_scale * 30.212826,

		x_scale * 25.162631, y_scale * 48.761909,
		x_scale * 25.111912, y_scale * 35.105754,
		x_scale * 25.19571, y_scale * 24.011616,

		x_scale * 25.147545, y_scale * 11.042955,
		x_scale * 44.029667, y_scale * 29.754207,
		x_scale * 25.162631, y_scale * 48.761909,

		x_scale * 1.5,

		// Pause button (circle backdrop)
		x_scale * 30.281246,  // cx
		y_scale * 30.281246,  // cy
		x_scale * 25.0,  // r
		x_scale * 1.5,  // stroke-width

		// Pause button (symbol)
		x_scale * 30.281250, y_scale * 4.6562500,

		x_scale * 16.233856, y_scale * 4.6562500,
		x_scale * 4.8437500, y_scale * 16.046356,
		x_scale * 4.8437500, y_scale * 30.093750,

		x_scale * 4.8437500, y_scale * 44.141144,
		x_scale * 16.233856, y_scale * 55.531251,
		x_scale * 30.281250, y_scale * 55.531250,

		x_scale * 44.328644, y_scale * 55.531250,
		x_scale * 55.718751, y_scale * 44.141145,
		x_scale * 55.718750, y_scale * 30.093750,

		x_scale * 55.718750, y_scale * 16.046356,
		x_scale * 44.328645, y_scale * 4.6562500,
		x_scale * 30.281250, y_scale * 4.6562500,

		x_scale * 19.750000, y_scale * 12.812500,
		x_scale * 26.031250, y_scale * 12.812500,

		x_scale * 26.290852, y_scale * 12.812500,
		x_scale * 26.500000, y_scale * 13.021648,
		x_scale * 26.500000, y_scale * 13.281250,
		x_scale * 26.500000, y_scale * 47.031250,

		x_scale * 26.500000, y_scale * 47.290852,
		x_scale * 26.290852, y_scale * 47.500001,
		x_scale * 26.031250, y_scale * 47.500000,
		x_scale * 19.750000, y_scale * 47.500000,

		x_scale * 19.490398, y_scale * 47.500000,
		x_scale * 19.281251, y_scale * 47.290851,
		x_scale * 19.281250, y_scale * 47.031250,
		x_scale * 19.281250, y_scale * 13.281250,

		x_scale * 19.281250, y_scale * 13.021648,
		x_scale * 19.490399, y_scale * 12.812500,
		x_scale * 19.750000, y_scale * 12.812500,

		x_scale * 34.656250, y_scale * 12.812500,
		x_scale * 40.937500, y_scale * 12.812500,

		x_scale * 41.197102, y_scale * 12.812500,
		x_scale * 41.406248, y_scale * 13.021648,
		x_scale * 41.406250, y_scale * 13.281250,
		x_scale * 41.406250, y_scale * 47.031250,

		x_scale * 41.406250, y_scale * 47.290852,
		x_scale * 41.197102, y_scale * 47.500001,
		x_scale * 40.937500, y_scale * 47.500000,
		x_scale * 34.656250, y_scale * 47.500000,

		x_scale * 34.396648, y_scale * 47.500000,
		x_scale * 34.187499, y_scale * 47.290851,
		x_scale * 34.187500, y_scale * 47.031250,
		x_scale * 34.187500, y_scale * 13.281250,

		x_scale * 34.187500, y_scale * 13.021648,
		x_scale * 34.396649, y_scale * 12.812500,
		x_scale * 34.656250, y_scale * 12.812500,

		x_scale * 1.5

		);
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG definitions to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED56: An error '%s' occured when writing to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// * For each slide, process and write out it's layers *
	export_time_counter = 0;
	slide_pointer = g_list_first(slides);
	g_list_foreach(slide_pointer, menu_export_svg_animation_slide, NULL);

	// Add the playback control bar.  We do it last, so it's over the top of everything else
	g_string_printf(tmp_gstring,
		"<rect width=\"%.4fpx\" height=\"%.4fpx\" fill-opacity=\"1.0\""
		" x=\"%.4fpx\" y=\"%.4fpx\" stroke-width=\"%.4fpx\""
		" fill=\"#fff\" stroke=\"black\" stroke-linejoin=\"round\" stroke-dasharray=\"none\""
		" stroke-opacity=\"1.0\" rx=\"%.4fpx\" ry=\"%.4fpx\" />\n"

		"<use xlink:href=\"#controls_rewind\" x=\"%.4fpx\" y=\"%.4fpx\" onclick=\"control_rewind()\"/>\n"
		"<use xlink:href=\"#controls_pause\" x=\"%.4fpx\" y=\"%.4fpx\" onclick=\"control_pause()\"/>\n"
		"<use xlink:href=\"#controls_play\" x=\"%.4fpx\" y=\"%.4fpx\" onclick=\"control_unpause()\"/>\n",

		x_scale * 117.0, y_scale * 60.0,  // width, height
		control_bar_x_offset, control_bar_y_offset,
		x_scale * 1.1719,  // Stroke width
		x_scale * 5.8594, y_scale * 5.8594,  // rx, ry

		control_bar_x_offset, control_bar_y_offset,
		control_bar_x_offset + (x_scale * 56.0), control_bar_y_offset,
		control_bar_x_offset + (x_scale * 56.0), control_bar_y_offset);
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG playback bar to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED58: An error '%s' occured when adding the playback controls to the output file '%s'", error->message, filename);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Write the closing SVG structure to the output file
	g_string_assign(tmp_gstring, "</svg>\n");
	return_value = g_io_channel_write_chars(output_file, tmp_gstring->str, tmp_gstring->len, &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG closing tags to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED11: An error '%s' occured when writing the SVG closing tags to the output file '%s'", error->message, filename);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);
		return;
	}

	// Small update to the status bar, to show progress to the user
	g_string_printf(tmp_gstring, "Wrote SVG file '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Close the output file
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED09: An error '%s' occured when closing the output file '%s'", error->message, filename);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);
		return;
	}

	// Free the memory allocated in this function
	g_string_free(tmp_gstring, TRUE);
	g_free(filename);
}


// Function called when the user selects File -> New from the top menu
void menu_file_new(void)
{
	// Local variables
	GtkDialog			*project_dialog;		// Widget for the dialog
	GtkWidget			*project_table;			// Table used for neat layout of the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to
	gint				dialog_result;			// Catches the return code from the dialog box

	GtkWidget			*name_label;			// Label widget
	GtkWidget			*name_entry;			// Widget for accepting the name of the new project

	GtkWidget			*width_label;			// Label widget
	GtkWidget			*width_button;			//

	GtkWidget			*height_label;			// Label widget
	GtkWidget			*height_button;			//

	GtkWidget			*fps_label;				// Label widget
	GtkWidget			*fps_button;			//

	GtkWidget			*bg_color_label;		// Label widget
	GtkWidget			*bg_color_button;		// Background color selection button


	// * Pop open a dialog box asking the user for the details of the new project *

	// Create the dialog window, and table to hold its children
	project_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Create new Flame Project", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	project_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(project_dialog->vbox), GTK_WIDGET(project_table), FALSE, FALSE, 10);

	// Create the label asking for the new project name
	name_label = gtk_label_new("Project Name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(name_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the new project name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 20);
	gtk_entry_set_text(GTK_ENTRY(name_entry), project_name->str);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(name_entry), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the project width
	width_label = gtk_label_new("Width: ");
	gtk_misc_set_alignment(GTK_MISC(width_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(width_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the project width
	width_button = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_button), 1280);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(width_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the project height
	height_label = gtk_label_new("Height: ");
	gtk_misc_set_alignment(GTK_MISC(height_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(height_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the project height
	height_button = gtk_spin_button_new_with_range(0, 6000, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(height_button), 1024);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(height_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the number of frames per second
	fps_label = gtk_label_new("Frames per second: ");
	gtk_misc_set_alignment(GTK_MISC(fps_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(fps_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the number of frames per second
	fps_button = gtk_spin_button_new_with_range(0, 48, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(fps_button), frames_per_second);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(fps_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label next to the color swatch
	bg_color_label = gtk_label_new("Background color: ");
	gtk_misc_set_alignment(GTK_MISC(bg_color_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(bg_color_label), 0, 1, row_counter, row_counter + 1);

	// Create the background color selection button
    bg_color_button = gtk_color_button_new_with_color(&default_bg_colour);
    gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(bg_color_button), TRUE);
	gtk_table_attach_defaults(GTK_TABLE(project_table), GTK_WIDGET(bg_color_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(project_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(project_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(project_dialog));
		return;
	}

	// If there's a project presently loaded in memory, we unload it
	if (NULL != slides)
	{
		// Free the resources presently allocated to slides
		g_list_foreach(slides, destroy_slide, NULL);

		// Re-initialise pointers
		slides = NULL;
		current_slide = NULL;
	}

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Set the project name
	g_string_printf(project_name, "%s", gtk_entry_get_text(GTK_ENTRY(name_entry)));

	// Set the project width
	project_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_button));

	// Set the project height
	project_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(height_button));

	// Set the number of frames per second
	frames_per_second = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(fps_button));

	// Set the default background color
	gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color_button), &default_bg_colour);

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(project_dialog));

	// Create a blank frame to start things from
	slide_insert();
	current_slide = slides;

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, "New project created.");
	gdk_flush();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();
}


// Function called when the user selects File -> Open from the top menu
void menu_file_open(void)
{
	// Local variables
	GtkFileFilter		*all_filter;
	gchar				*filename;				// Pointer to the chosen file name
	GtkFileFilter		*flame_filter;
	GtkWidget 			*open_dialog;
	gboolean			return_code;

	GString				*tmp_gstring;			// Temporary gstring


	// Create the dialog asking the user to select a Flame Project file
	open_dialog = gtk_file_chooser_dialog_new("Open a Flame Project",
											  GTK_WINDOW(main_window),
											  GTK_FILE_CHOOSER_ACTION_OPEN,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											  NULL);

	// Create the filter so only *.flame files are displayed
	flame_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flame_filter, "*.flame");
	gtk_file_filter_set_name(flame_filter, "Flame Project file (*.flame)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), flame_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(open_dialog), all_filter);

	// Set the path and name of the file to open
	if (NULL != file_name)
	{
		// Select the last opened file if possible
		gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(open_dialog), file_name->str);
	} else
	{
		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(open_dialog), default_project_folder->str);
	}

	// Run the dialog and wait for user input
	if (gtk_dialog_run(GTK_DIALOG(open_dialog)) != GTK_RESPONSE_ACCEPT)
	{
		// * The user didn't choose a file, so destroy the dialog box and return *
		gtk_widget_destroy(open_dialog);
		return;		
	}

	// * We only get to here if a file was chosen *

	// Gray out the toolbar items that can't be used without a project loaded
	disable_layer_toolbar_buttons();
	disable_main_toolbar_buttons();

	// Get the filename from the dialog box
	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(open_dialog));

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(open_dialog);

	// Open and parse the selected file
	return_code = flame_read(filename);

	// Keep the full file name around for future reference
	file_name = g_string_new(NULL);
	file_name = g_string_assign(file_name, filename);

	// Destroy the existing output resolution selector
	g_signal_handler_disconnect(G_OBJECT(resolution_selector), resolution_callback);
	gtk_container_remove(GTK_CONTAINER(message_bar), GTK_WIDGET(resolution_selector));

	// Create a new output resolution selector, including the resolution of the loaded project
	resolution_selector = GTK_COMBO_BOX(create_resolution_selector(res_array, num_res_items, output_width, output_height));
	gtk_table_attach_defaults(message_bar, GTK_WIDGET(resolution_selector), 8, 9, 0, 1);
	resolution_callback = g_signal_connect(G_OBJECT(resolution_selector), "changed", G_CALLBACK(resolution_selector_changed), (gpointer) NULL);
	gtk_widget_show_all(GTK_WIDGET(message_bar));

	// Use the status bar to communicate the successful loading of the project
	tmp_gstring = g_string_new(NULL);
	g_string_printf(tmp_gstring, "Project '%s' successfully loaded.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Make the current slide point to the first slide
	current_slide = slides;

	// Create the tooltips for the slides
	create_tooltips();

	// Update the film strip with the new slides
	refresh_film_strip();

	// Draw the timeline area
	draw_timeline();

	// Draw the workspace area
	draw_workspace();

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);

	// Enable the toolbar buttons
	enable_layer_toolbar_buttons();
	enable_main_toolbar_buttons();

	// Frees the memory allocated in this function
	g_free(filename);
}


// Function called when the user selects File -> Save from the top menu
void menu_file_save(void)
{
	// Local variables
	GtkFileFilter		*all_filter;			// Filter for *.*
	gchar				*dir_name_part;			// Briefly used for holding a directory name
	gchar				*filename;				// Pointer to the chosen file name
	gchar				*file_name_part;		// Briefly used for holding a file name
	GtkFileFilter		*flame_filter;			// Filter for *.flame
	GtkWidget 			*save_dialog;			// Dialog widget
	gboolean			unique_name;			// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;			// Widget for overwrite warning dialog

	xmlDocPtr			document_pointer;		// Points to the XML document structure in memory
	xmlNodePtr			slide_root;				// Points to the root of the slide data
	xmlNodePtr			meta_pointer;			// Points to the meta-data node
	xmlNodePtr			pref_pointer;			// Points to the preferences node
	xmlNodePtr			root_node;				// Points to the root node
	xmlSaveCtxt			*save_context;			// Points to the save context

	GString				*tmp_gstring;			// Temporary GString
	gint				tmp_int;				// Temporary integer
	glong				tmp_long;				// Temporary long integer


	// Initialise some things
	tmp_gstring = g_string_new(NULL);

	// Create the dialog asking the user for the name to save as
	save_dialog = gtk_file_chooser_dialog_new("Save As",
											  GTK_WINDOW(main_window),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);

	// Create the filter so only *.flame files are displayed
	flame_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(flame_filter, "*.flame");
	gtk_file_filter_set_name(flame_filter, "Flame Project file (*.flame)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), flame_filter);

	// Create the filter so all files (*.*) can be displayed
	all_filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(all_filter, "*.*");
	gtk_file_filter_set_name(all_filter, "All files (*.*)");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), all_filter);

	// Set the path and name of the file to save as.  Use project_name as a default
	if (NULL != file_name)
	{
		// Work out the directory and file name components
		dir_name_part = g_path_get_dirname(file_name->str);
		file_name_part = g_path_get_basename(file_name->str);

		// Set the default directory and file name for the dialog		
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), dir_name_part);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), file_name_part);

		// Free the string
		g_free(dir_name_part);
		g_free(file_name_part);
	} else
	{
		// Nothing has been established, so use project_name
		g_string_printf(tmp_gstring, "%s.flame", project_name->str);
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(save_dialog), tmp_gstring->str);

		// Change to the default project directory
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog), default_project_folder->str);
	}

	// Run the dialog and wait for user input
	unique_name = FALSE;
	while (TRUE != unique_name)
	{
		// Get a filename to save as
		if (gtk_dialog_run(GTK_DIALOG(save_dialog)) != GTK_RESPONSE_ACCEPT)
		{
			// The dialog was cancelled, so destroy it and return to the caller
			gtk_widget_destroy(save_dialog);
			return;
		}

		// Retrieve the filename from the dialog box
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(save_dialog));

		// Check if there's an existing file of this name, and give an Overwrite? type prompt if there is
		if (TRUE == g_file_test(filename, G_FILE_TEST_EXISTS))
		{
			// Something with this name already exists
			warn_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
								GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_QUESTION,
								GTK_BUTTONS_YES_NO,
								"Overwrite existing file?");
			if (GTK_RESPONSE_YES == gtk_dialog_run(GTK_DIALOG(warn_dialog)))
			{
				// We've been told to overwrite the existing file
				unique_name = TRUE;
			}
			gtk_widget_destroy(warn_dialog);
		} else
		{
			// The indicated file name is unique, we're fine to save
			unique_name = TRUE;
		}
	}

	// Destroy the dialog box, as it's not needed any more
	gtk_widget_destroy(save_dialog);

	// Create an empty document pointer
	document_pointer = xmlNewDoc("1.0");
	if (NULL == document_pointer)
	{
		display_warning("ED19: Error creating the XML save document\n");
		return;
	}

    // Create the root node
	root_node = xmlNewDocRawNode(document_pointer, NULL, BAD_CAST "flame_project", NULL);
	if (NULL == root_node)
	{
		display_warning("ED21: Error creating the root node\n");
		return;
	}

	// Anchor the root node to the document
	xmlDocSetRootElement(document_pointer, root_node);

    // Create the meta information container
	meta_pointer = xmlNewChild(root_node, NULL, "meta-data", NULL);
	if (NULL == meta_pointer)
	{
		display_warning("ED25: Error creating the meta-data container\n");
		return;
	}

	// Add the save format version number to the XML document
	xmlNewChild(meta_pointer, NULL, "save_format", "2.0");

    // Create the preferences container
	pref_pointer = xmlNewChild(root_node, NULL, "preferences", NULL);
	if (NULL == pref_pointer)
	{
		display_warning("ED20: Error creating the preferences container\n");
		return;
	}

	// Add the preferences to the XML document
	xmlNewChild(pref_pointer, NULL, "project_name", project_name->str);
	xmlNewChild(pref_pointer, NULL, "output_folder", output_folder->str);
	g_string_printf(tmp_gstring, "%u", output_width);
	xmlNewChild(pref_pointer, NULL, "output_width", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", output_height);
	xmlNewChild(pref_pointer, NULL, "output_height", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", output_quality);
	xmlNewChild(pref_pointer, NULL, "output_quality", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_width);
	xmlNewChild(pref_pointer, NULL, "project_width", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", project_height);
	xmlNewChild(pref_pointer, NULL, "project_height", tmp_gstring->str);
	g_string_printf(tmp_gstring, "%u", slide_length);
	xmlNewChild(pref_pointer, NULL, "slide_length", tmp_gstring->str);

    // Create a container for the slides
	slide_root = xmlNewChild(root_node, NULL, "slides", NULL);
	if (NULL == slide_root)
	{
		display_warning("ED22: Error creating the slides container\n");
		return;
	}

	// Add the slide data to the XML structure
	slides = g_list_first(slides);
	g_list_foreach(slides, menu_file_save_slide, slide_root);

	// Create a saving context
	save_context = xmlSaveToFilename(filename, "utf8", 1);  // XML_SAVE_FORMAT == 1

	// Flush the saving context
	tmp_long = xmlSaveDoc(save_context, document_pointer);

	// Close the saving context
	tmp_int = xmlSaveClose(save_context);

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, "Saved project as '%s'.", filename);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// Keep the full file name around for future reference
	file_name = g_string_new(NULL);
	file_name = g_string_assign(file_name, filename);

	// * Function clean up area *

	// Frees the memory holding the file name
	g_free(filename);

	// Free the temporary gstring
	g_string_free(tmp_gstring, TRUE);
}


// Function called when the user selects Help -> About from the top menu
void menu_help_about(void)
{
// fixme4: GtkAboutDialog wasn't present in the default Solaris 10 install. :(
#ifndef __sun

	// Local variables
	GtkAboutDialog		*about_dialog;
	const gchar			*authors[] = {"Justin Clift", NULL};
	gint				tmp_int;

	// Create an About dialog
	about_dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());
	gtk_about_dialog_set_name(about_dialog, APP_NAME);
	gtk_about_dialog_set_version(about_dialog, APP_VERSION);
	gtk_about_dialog_set_copyright(about_dialog, "(C) 2005-2006 Justin Clift <justin@postgresql.org>");
	gtk_about_dialog_set_license(about_dialog, "LGPL");
	gtk_about_dialog_set_license(about_dialog, NULL);
	gtk_about_dialog_set_authors(about_dialog, authors);

	// Display the dialog box
	tmp_int = gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy(GTK_WIDGET(about_dialog));
#endif
}


// Function called when the user selects Help -> Register from the top menu
void menu_help_register(void)
{
#ifndef _WIN32  // Non-windows check
	// Launch a browser window of the Flame Project registration page
	gnome_url_show ("http://www.flameproject.org/go/register", NULL);
#endif
}


// Function called when the user selects Help -> Survey from the top menu
void menu_help_survey(void)
{
#ifndef _WIN32  // Non-windows check
	// Launch a browser window of the Flame Project survey
	gnome_url_show ("http://www.flameproject.org/go/survey", NULL);
#endif
}


// Function called when the user selects Help -> Website from the top menu
void menu_help_website(void)
{
#ifndef _WIN32  // Non-windows check
	// Launch a browser window of the Flame Project website
	gnome_url_show ("http://www.flameproject.org/go/website", NULL);
#endif
}


// Function called when the user selects Screenshots -> Capture from the top menu
void menu_screenshots_capture(void)
{
	// Local variables
	GError				*error = NULL;			// Pointer to error return structure
	gchar				*full_file_name;		// Holds the fully worked out file name to save as
	GIOChannel			*output_file;			// The output file handle
	GIOStatus			return_value;			// Return value used in most GIOChannel functions

	GtkDialog			*capture_dialog;		// Widget for the dialog
	GtkWidget			*capture_table;			// Table used for neat layout of the dialog box

	GtkWidget			*x_offset_label;		// Label widget
	GtkWidget			*x_offset_button;		// Widget for accepting the new X Offset data

	GtkWidget			*y_offset_label;		// Label widget
	GtkWidget			*y_offset_button;		// Widget for accepting the new Y Offset data

	GtkWidget			*x_length_label;		// Label widget
	GtkWidget			*x_length_button;		// Widget for accepting the new X Length data

	GtkWidget			*y_length_label;		// Label widget
	GtkWidget			*y_length_button;		// Widget for accepting the new Y Length data

	gint				dialog_result;			// Catches the return code from the dialog box

	GdkScreen			*which_screen;			// Gets given the screen the monitor is on

	gchar				*tmp_gchar;				// Temporary gchar
	gsize				tmp_gsize;				// Temporary gsize
	gpointer			tmp_ptr;				// Temporary pointer
	GString				*tmp_gstring;			// Temporary string


#ifdef _WIN32
	// If we're running on Windows, pop up a message about capturing not working yet then return
	display_warning("Screen capturing not yet supported for Windows, please place your own screenshots in the screenshots directory");
	return;
#endif

// fixme4: Stuff not present in the shipping version of Solaris 10 :(
#ifndef __sun
	GKeyFile			*lock_file;				// Pointer to the lock file structure
#endif

	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// * Pop open a dialog box asking the user for the offset and size of capture area *

	// Create the dialog window, and table to hold its children
	capture_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Capture screenshots", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	capture_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(capture_dialog->vbox), GTK_WIDGET(capture_table), FALSE, FALSE, 10);

	// Create the label asking for the X Offset
	x_offset_label = gtk_label_new("X Offset: ");
	gtk_misc_set_alignment(GTK_MISC(x_offset_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_label), 0, 1, 0, 1);

	// Create the entry that accepts the new X Offset data
	x_offset_button = gtk_spin_button_new_with_range(0, project_width, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_offset_button), capture_x);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_offset_button), 1, 2, 0, 1);

	// Create the label asking for the Y Offset
	y_offset_label = gtk_label_new("Y Offset: ");
	gtk_misc_set_alignment(GTK_MISC(y_offset_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_label), 0, 1, 1, 2);

	// Create the entry that accepts the new Y Offset data
	y_offset_button = gtk_spin_button_new_with_range(0, project_height, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_offset_button), capture_y);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_offset_button), 1, 2, 1, 2);

	// Which monitor are we displaying on?
	which_screen = gtk_window_get_screen(GTK_WINDOW(main_window));

	// Create the label asking for the X Length
	x_length_label = gtk_label_new("Width: ");
	gtk_misc_set_alignment(GTK_MISC(x_length_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_length_label), 0, 1, 2, 3);

	// Create the entry that accepts the new X Length data
	x_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_width(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_length_button), capture_width);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(x_length_button), 1, 2, 2, 3);

	// Create the label asking for the Y Length
	y_length_label = gtk_label_new("Height: ");
	gtk_misc_set_alignment(GTK_MISC(y_length_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_length_label), 0, 1, 3, 4);

	// Create the entry that accepts the new Y Length data
	y_length_button = gtk_spin_button_new_with_range(0, gdk_screen_get_height(which_screen), 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_length_button), capture_height);
	gtk_table_attach_defaults(GTK_TABLE(capture_table), GTK_WIDGET(y_length_button), 1, 2, 3, 4);

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(capture_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(capture_dialog));

	// Was the anything other than the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The user decided to escape out of the dialog box, so just return
		gtk_widget_destroy(GTK_WIDGET(capture_dialog));
		return;
	}

	// Construct the fullly qualified path name for ~/.flame-lock file (to hold capture settings in)
	tmp_ptr = (gchar *) g_get_home_dir();
	full_file_name = g_build_filename(tmp_ptr, ".flame-lock", NULL);

	// Retrieve the values given by the user
	capture_x = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_offset_button));
	capture_y = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_offset_button));
	capture_width = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(x_length_button));
	capture_height = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(y_length_button));

// fixme4: Key file functions aren't present in the shipping version of Solaris 10 :(
#ifndef __sun
	// Create the contents of the ~/.flame-lock file in memory
	lock_file = g_key_file_new();
	g_key_file_set_string(lock_file, "Project", "Name", project_name->str);  // Name of project
	g_key_file_set_string(lock_file, "Project", "Directory", screenshots_folder->str);  // Directory to save screenshots in
	g_key_file_set_integer(lock_file, "Project", "X_Offset", capture_x);  // Top left X coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "X_Length", capture_width);  // Width of screen area to grab
	g_key_file_set_integer(lock_file, "Project", "Y_Offset", capture_y);  // Top left Y coordinate of screen area
	g_key_file_set_integer(lock_file, "Project", "Y_Length", capture_height);  // Height of screen area to grab
#endif

	// Create IO channel for writing to
	output_file = g_io_channel_new_file(full_file_name, "w", &error);
	if (NULL == output_file)
	{
		// * An error occured when opening the file for writing, so alert the user, and return to the calling routine indicating failure *

		// Display a warning message using our function
		g_string_printf(tmp_gstring, "Error ED16: An error '%s' occured when opening '%s' for writing", error->message, full_file_name);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Write the ~/.flame-lock file to disk
// fixme4: Key file functions aren't present in the shipping version of Solaris 10 :(
#ifndef __sun
	tmp_gchar = g_key_file_to_data(lock_file, NULL, NULL);
#endif
	return_value = g_io_channel_write_chars(output_file, tmp_gchar, strlen(tmp_gchar), &tmp_gsize, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when writing the SVG header to the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED17: An error '%s' occured when writing data to the '%s' file", error->message, full_file_name);
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Close the IO channel
	return_value = g_io_channel_shutdown(output_file, TRUE, &error);
	if (G_IO_STATUS_ERROR == return_value)
	{
		// * An error occured when closing the output file, so alert the user, and return to the calling routine indicating failure *
		g_string_printf(tmp_gstring, "Error ED18: An error '%s' occured when closing the output file '%s'", error->message, full_file_name);

		// Display the warning message using our function
		display_warning(tmp_gstring->str);

		// Free the memory allocated in this function
		g_string_free(tmp_gstring, TRUE);
		g_error_free(error);

		return;
	}

	// Add a message to the status bar so the user gets visual feedback
	g_string_printf(tmp_gstring, "Wrote capture lock file '%s'.", full_file_name);
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_gstring->str);
	gdk_flush();

	// * Function clean up area *

// fixme4: Key file functions aren't present in the shipping version of Solaris 10 :(
#ifndef __sun
	// Close the lock file
	g_key_file_free(lock_file);
#endif

	// Free the temporary GString
	g_string_free(tmp_gstring, TRUE);

	// Destroy the dialog box widget
	gtk_widget_destroy(GTK_WIDGET(capture_dialog));
}


// Function called when the user selects Screenshots -> Import from the top menu
void menu_screenshots_import(void)
{
	// Local variables
	GDir				*dir_ptr;				// Pointer to the directory entry structure
	const gchar			*dir_entry;				// Holds a file name

	GSList				*entries = NULL;		// Holds a list of screen shot file names
	guint				largest_height = 0;		// Holds the height of the largest screenshot thus far
	guint				largest_width = 0;		// Holds the width of the largest screenshot thus far

	gint				num_screenshots = 0;	// Switch to track if other screenshots already exist

	GError				*error = NULL;			// Pointer to error return structure

	guint				recent_message;			// Message identifier, for newest status bar message

	slide				*tmp_slide;				// Temporary slide
	layer				*tmp_layer;				// Temporary layer
	layer_image			*tmp_image_ob;			// Temporary image layer
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	GdkPixbuf			*tmp_gdk_pixbuf;		// Temporary GDK Pixbuf
	GString				*tmp_string;			// Temporary string
	gint				tmp_int = 0;			// Temporary integer
	GdkRectangle		tmp_rect =				// Temporary rectangle covering the area of the status bar
							{0,
							 0,
							 status_bar->allocation.width,
							 status_bar->allocation.height};


	// Initialise various things
	tmp_string = g_string_new(NULL);

	// * We know the path to get the screenshots from (screenshots_folder), and their prefix name (project_name),
	//   so we make a list of them and add them to the slides linked list *

	// * Check if the screenshots folder exists *
	if (!(dir_ptr = g_dir_open(screenshots_folder->str, 0, &error)))
	{
		// * Something went wrong when opening the screenshots folder *
		if (G_FILE_ERROR_NOENT != error->code)
		{
			// * The error was something other than the folder not existing, which we could cope with *

			// Display the warning message using our function
			g_string_printf(tmp_string, "Error ED03: Something went wrong opening the screenshots folder '%s': %s", screenshots_folder->str, error->message);
			display_warning(tmp_string->str);

			// Free the memory allocated in this function
			g_string_free(tmp_string, TRUE);
			g_error_free(error);
			exit(3);
		}

		// The screenshots folder doesn't exist.  Notify the user and direct them to change this in the preferences menu
		display_warning("Error ED04: The screenshots folder doesn't exist.  Please update this in the project preferences and try again");
		g_error_free(error);
		return;
	}

	// * Scan the given directory for screenshot files to import *
	while ((dir_entry = g_dir_read_name(dir_ptr)) != NULL)
	{
		// Look for files starting with the same name as the project
		if (g_str_has_prefix(dir_entry, project_name->str))
		{
			// The directory entry starts with the correct prefix, now let's check the file extension
			if (g_str_has_suffix(dir_entry, ".png"))
			{
				// The directory entry has the correct file extension too, so it's very likely one of our screenshots
				// We add it to the list of screenshot entries we know about
				entries = g_slist_append(entries, g_strdup_printf("%s", dir_entry));
				num_screenshots += 1;
			}
		}
	}

	// If no screenshots were found, alert the user and return to the calling function
	if (0 == num_screenshots)
	{
		// Display the warning message using our function
		g_string_printf(tmp_string, "Error ED05: No screenshots found in screenshot folder.\n\nThey must be named:\n\n\t%s0001.png\n\t%s0002.png\n\t<etc>\n\nIt's case sensitive too.", project_name->str, project_name->str);
		display_warning(tmp_string->str);

		return;
	}

	// Use the status bar to communicate the number of screenshots found
	g_string_printf(tmp_string, "Found %u screenshots.", num_screenshots);
	recent_message = gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_string->str);
	gdk_flush();

	// Redraw the main window
	gtk_widget_draw(status_bar, &tmp_rect);

	// * Load the screenshots *
	// fixme5: Would it be better to just do the first few, and background the rest for later?
	for (tmp_int = 1; tmp_int <= num_screenshots; tmp_int++)
	{
		// Remove the previous status bar message
		gtk_statusbar_remove(GTK_STATUSBAR(status_bar), statusbar_context, recent_message);

		// * The loaded image file becomes a background layer image for a new slide *

		// Allocate a new slide structure for use
		tmp_slide = g_new(slide, 1);
		tmp_slide->layers = NULL;

		// Allocate a new layer structure for use in the slide
		tmp_layer = g_new(layer, 1);

		// Work out the full path to the image file
		g_string_printf(tmp_string, "%s%c", screenshots_folder->str, G_DIR_SEPARATOR);
		tmp_string = g_string_append(tmp_string, g_slist_nth(entries, tmp_int - 1)->data);

		// Load the image file(s) into a thumbnail sized pixel buffer
		tmp_gdk_pixbuf = gdk_pixbuf_new_from_file_at_size(tmp_string->str, preview_width, -1, &error);
		if (NULL == tmp_gdk_pixbuf)
		{
			// * Something went wrong when loading the screenshot *

			// Display a warning message using our function
			g_string_printf(tmp_string, "Error ED06: Something went wrong when loading the screenshot '%s'", tmp_string->str);
			display_warning(tmp_string->str);
			g_string_free(tmp_string, TRUE);

			return;
		}

		// * Create the new background layer *

		// Construct a new image object
		tmp_image_ob = g_new(layer_image, 1);
		tmp_image_ob->x_offset_start = 0;
		tmp_image_ob->y_offset_start = 0;
		tmp_image_ob->x_offset_finish = 0;
		tmp_image_ob->y_offset_finish = 0;
		tmp_image_ob->image_path = g_string_new(tmp_string->str);
		tmp_image_ob->image_data = gdk_pixbuf_new_from_file(tmp_string->str, NULL);  // Load the image again, at full size.  It's the background layer
		tmp_image_ob->width = gdk_pixbuf_get_width(tmp_image_ob->image_data);
		tmp_image_ob->height = gdk_pixbuf_get_height(tmp_image_ob->image_data);
		tmp_image_ob->modified = FALSE;

		// If the new image is larger than the others loaded, we keep the new dimensions
		if (tmp_image_ob->height > largest_height)
		{
			largest_height = tmp_image_ob->height;
		}
		if (tmp_image_ob->width > largest_width)
		{
			largest_width = tmp_image_ob->width;
		}

		// Wrap the background layer info around it
		tmp_layer->object_data = (GObject *) tmp_image_ob;
		tmp_layer->object_type = TYPE_GDK_PIXBUF;
		tmp_layer->start_frame = 0;
		tmp_layer->finish_frame = slide_length;
		tmp_layer->name = g_string_new("Background");
		tmp_layer->external_link = g_string_new(NULL);

		// Add the background layer to the new slide being created
		tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

		// Create the List store the slide layer data is kept in
		tmp_iter = g_new(GtkTreeIter, 1);
		tmp_layer->row_iter = tmp_iter;
		tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
									G_TYPE_STRING,  // TIMELINE_NAME
									G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
									G_TYPE_UINT,  // TIMELINE_START
									G_TYPE_UINT,  // TIMELINE_FINISH
									G_TYPE_UINT,  // TIMELINE_X_OFF_START
									G_TYPE_UINT,  // TIMELINE_Y_OFF_START
									G_TYPE_UINT,  // TIMELINE_X_OFF_FINISH
									G_TYPE_UINT);  // TIMELINE_Y_OFF_FINISH
		gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
		gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
						TIMELINE_NAME, tmp_layer->name->str,
						TIMELINE_VISIBILITY, TRUE,
						TIMELINE_START, 0,
						TIMELINE_FINISH, slide_length,
						TIMELINE_X_OFF_START, 0,
						TIMELINE_Y_OFF_START, 0,
						TIMELINE_X_OFF_FINISH, 0,
						TIMELINE_Y_OFF_FINISH, 0,
						-1);

		// Add the thumbnail to the new slide structure
		tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));

		// Create an event box for adding the thumbnail image to, so it can receive events and display tool tips
		tmp_slide->event_box = gtk_event_box_new();

		// Mark the name for the slide as unset
		tmp_slide->name = NULL;
		tmp_slide->tooltip = NULL;

		// Set the timeline widget for the slide to NULL, so we know to create it later on
		tmp_slide->timeline_widget = NULL;

		// Add the thumbnail to the event box
		gtk_container_add(GTK_CONTAINER(tmp_slide->event_box), GTK_WIDGET(tmp_slide->thumbnail));

		// Add a mouse click handler to the event box
		tmp_slide->click_handler = g_signal_connect(G_OBJECT(tmp_slide->event_box), "button_release_event", G_CALLBACK(film_strip_slide_clicked), tmp_slide);

		// Add the temporary slide to the slides GList
		slides = g_list_append(slides, tmp_slide);

		// Update the status bar with a progress counter
		g_string_printf(tmp_string, "Loaded image %u of %u.", tmp_int, num_screenshots);
		recent_message = gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, tmp_string->str);
		gdk_flush();

		// Redraw the main window
		gtk_widget_draw(status_bar, &tmp_rect);
	}

	// Free the temporary GString
	g_string_free(tmp_string, TRUE);

	// Update the project with the new height and width
	project_height = largest_height;
	project_width = largest_width;

	// Update the film strip with the new slides
	refresh_film_strip();

	// If not presently set, make the first slide the present slide
	if (NULL == current_slide)
	{
		current_slide = g_list_first(slides);
	}

	// Draw the timeline area
	draw_timeline();

	// Draw the workspace area
	draw_workspace();

	// Enable the project based menu items
	menu_enable("/Slide", TRUE);
	menu_enable("/Layer", TRUE);
	menu_enable("/Export", TRUE);
}


// Function called when the user clicks the Crop timeline toolbar button
void project_crop(void)
{
	// Local variables
	GtkDialog			*crop_dialog;			// Widget for the dialog
	GtkWidget			*crop_table;			// Table used for neat layout of the dialog box
	guint				row_counter = 0;		// Used to count which row things are up to
	gint				dialog_result;			// Catches the return code from the dialog box

	gint				bottom_value;
	layer				*last_layer;			// Temporary layer
	GList				*layer_pointer;			// Points to the layers in the selected slide
	gint				left_value;
	gint				new_height;				// Hold the height of the cropped area
	GdkPixbuf			*new_pixbuf;			// Holds the cropped image data
	gint				new_width;				// Hold the width of the cropped area
	gint				num_slides;				// Total number of layers
	gint				right_value;
	gint				slide_counter;
	slide				*slide_data;
	gint				top_value;

	GtkWidget			*left_label;			// Label widget
	GtkWidget			*left_button;			//

	GtkWidget			*right_label;			// Label widget
	GtkWidget			*right_button;			//

	GtkWidget			*top_label;				// Label widget
	GtkWidget			*top_button;			//

	GtkWidget			*bottom_label;			// Label widget
	GtkWidget			*bottom_button;			//

	gint				tmp_int;				// Temporary int
	GdkPixbuf			*tmp_pixbuf;			// Temporary pixbuf


	// If no project is loaded then don't run this function
	if (NULL == current_slide)
	{
		// Make a beep, then return
		// fixme4: This code should never be reached any more, as exporting is disabled while no project loaded
		sound_beep();
		return;
	}

	// * Pop open a dialog box asking the user how much to crop off each side of the image *

	// Create the dialog window, and table to hold its children
	crop_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Crop image layer", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	crop_table = gtk_table_new(3, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(crop_dialog->vbox), GTK_WIDGET(crop_table), FALSE, FALSE, 10);

	// Create the label asking for the left side crop amount
	left_label = gtk_label_new("Left crop: ");
	gtk_misc_set_alignment(GTK_MISC(left_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	left_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(left_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(left_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	right_label = gtk_label_new("Right crop: ");
	gtk_misc_set_alignment(GTK_MISC(right_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	right_button = gtk_spin_button_new_with_range(0, project_width, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(right_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(right_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the top crop amount
	top_label = gtk_label_new("Top crop: ");
	gtk_misc_set_alignment(GTK_MISC(top_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the left side crop amount
	top_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(top_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(top_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Create the label asking for the right side crop amount
	bottom_label = gtk_label_new("Bottom crop: ");
	gtk_misc_set_alignment(GTK_MISC(bottom_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_label), 0, 1, row_counter, row_counter + 1);

	// Create the entry that accepts the right side crop amount
	bottom_button = gtk_spin_button_new_with_range(0, project_height, 10);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(bottom_button), 0);
	gtk_table_attach_defaults(GTK_TABLE(crop_table), GTK_WIDGET(bottom_button), 1, 2, row_counter, row_counter + 1);
	row_counter = row_counter + 1;

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(crop_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(crop_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// The dialog was cancelled, so destroy the dialog box and return
		gtk_widget_destroy(GTK_WIDGET(crop_dialog));
		return;
	}

	// Get the values from the dialog
	left_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(left_button));
	right_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(right_button));
	top_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(top_button));
	bottom_value = (gint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(bottom_button));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(crop_dialog));

	// Loop through the slide structure, cropping the backgrounds
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	for (slide_counter = 0; slide_counter < num_slides; slide_counter++)
	{
		slide_data = g_list_nth_data(slides, slide_counter);
		layer_pointer = slide_data->layers;
		layer_pointer = g_list_last(layer_pointer);

		// * Check if this slide has a background image *
		last_layer = layer_pointer->data;

		// Is this layer an image?
		if (TYPE_GDK_PIXBUF != last_layer->object_type)
		{
			// No it's not, so skip this slide
			continue;
		}

		// Is this layer the background?
		tmp_int = g_ascii_strncasecmp(last_layer->name->str, "Background", 10);
		if (0 != tmp_int)
		{
			// No it's not, so skip this slide
			continue;
		}

		// Create a new pixbuf, for storing the cropped image in
		new_height = gdk_pixbuf_get_height(((layer_image *) last_layer->object_data)->image_data)
			- top_value
			- bottom_value;
		new_width = gdk_pixbuf_get_width(((layer_image *) last_layer->object_data)->image_data)
			- left_value
			- right_value;
		new_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, new_width, new_height);

		// Create a new pixbuf, having just the cropped image data in it
		gdk_pixbuf_copy_area(((layer_image *) last_layer->object_data)->image_data, // Source pixbuf
			left_value,  // Left crop
			top_value,  // Top crop
			new_width,  // Width
			new_height,  // Height
			new_pixbuf,  // Destination
			0, 0);

		// Update the layer with the new cropped data
		tmp_pixbuf = ((layer_image *) last_layer->object_data)->image_data;
		((layer_image *) last_layer->object_data)->image_data = new_pixbuf;
		((layer_image *) last_layer->object_data)->width = new_width;
		((layer_image *) last_layer->object_data)->height = new_height;
		g_string_assign(((layer_image *) last_layer->object_data)->image_path, "");

		// Free the memory used by the old pixbuf
		g_object_unref(tmp_pixbuf);

		// Redraw the film strip thumbnail
		draw_thumbnail(g_list_nth(slides, slide_counter));
	}

	// Update project width and height global variables
	project_height = project_height - top_value - bottom_value;
	project_width = project_width - left_value - right_value;

	// Redraw the film strip area
	refresh_film_strip();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();
}


// Clears out the film strip, then re-creates it according to the full slide set
void refresh_film_strip(void)
{
	// Local variables
	gint				num_slides;
	guint				slide_counter;

	GList				*tmp_glist;
	slide				*tmp_slide;


	// * Clear the film strip area of existing content *
	tmp_glist = gtk_container_get_children(GTK_CONTAINER(film_strip));
	if (NULL != tmp_glist)
	{
		// For each slide shown in the film strip, remove its event box
		tmp_glist = g_list_first(tmp_glist);
		num_slides = g_list_length(tmp_glist) - 1;
		for (slide_counter = 0; slide_counter <= num_slides; slide_counter++)
		{
			// Remove the slide from the film strip area
			tmp_slide = g_list_nth_data(tmp_glist, slide_counter);
			gtk_container_remove(GTK_CONTAINER(film_strip), GTK_WIDGET(tmp_slide));
		}
		g_list_free(tmp_glist);
	}

	// * For each slide in the slides array, add its event box (with contained thumbnail) to the film strip area *
	slides = g_list_first(slides);
	num_slides = g_list_length(slides) - 1;
	for (slide_counter = 0; slide_counter <= num_slides; slide_counter++)
	{
		// Add the event box to the film strip area
		tmp_slide = g_list_nth_data(slides, slide_counter);
		gtk_box_pack_start(GTK_BOX(film_strip), GTK_WIDGET(tmp_slide->event_box), FALSE, FALSE, 0);

		// Add a separator between the thumbnails, to make things that little bit clearer
		gtk_box_pack_start(GTK_BOX(film_strip), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	// Display all of the images
	gtk_widget_show_all(GTK_WIDGET(film_strip));
}


// Function called when the user selects Slide -> Delete from the top menu
void slide_delete(void)
{
	// Local variables
	gint				num_slides;				// Number of slides in the whole slide list
	gint				slide_position;			// Which slide in the slide list we are deleting

	GList				*tmp_glist;				// Temporary GList


	// Are we trying to delete the only slide in the project (not good)?
	slides = g_list_first(slides);
	num_slides = g_list_length(slides);
	if (1 == num_slides)
	{
		// Yes we are, so give a warning message and don't delete the slide
		display_warning("You must have at least one slide in a project.\n");
		return;
	}

	// Remove the current slide from the slide list
	slide_position = g_list_position(slides, current_slide);
	tmp_glist = current_slide;
	slides = g_list_remove_link(slides, current_slide);

	// * Update the currently selected slide to point to the next slide *
	if (num_slides == (slide_position + 1))
	{
		// If we're deleting the last slide, we'll need to point to the previous one instead
		slide_position--;
	}
	current_slide = g_list_nth(slides, slide_position);

	// Recreate the slide tooltips
	create_tooltips();

	// Redraw the timeline
	draw_timeline();

	// Redraw the workspace
	draw_workspace();

	// Refresh the film strip area
	refresh_film_strip();

	// Free the resources allocated to the deleted slide
	destroy_slide(tmp_glist->data, NULL);
	g_list_free(tmp_glist);
}


// Function called when the user selects Slide -> Insert from the top menu
void slide_insert(void)
{
	// Local variables
	gint				slide_position;			// Which slide in the slide list we have selected

	GdkPixbuf			*tmp_gdk_pixbuf;		// Temporary GDK Pixbuf
	GtkTreeIter			*tmp_iter;				// Temporary GtkTreeIter
	layer				*tmp_layer;				// Temporary layer
	slide				*tmp_slide;				// Temporary slide


	// Create a new, empty slide
	tmp_slide = g_new(slide, 1);
	tmp_slide->layers = NULL;
	tmp_slide->name = NULL;

	// Allocate a new layer structure for use in the slide
	tmp_layer = g_new(layer, 1);
	tmp_layer->object_type = TYPE_EMPTY;
	tmp_layer->start_frame = 0;
	tmp_layer->finish_frame = slide_length;
	tmp_layer->name = g_string_new("Empty");
	tmp_layer->external_link = g_string_new(NULL);
	tmp_layer->object_data = (GObject *) g_new(layer_empty, 1);
	((layer_empty *) tmp_layer->object_data)->bg_color.red = default_bg_colour.red;
	((layer_empty *) tmp_layer->object_data)->bg_color.green = default_bg_colour.green;
	((layer_empty *) tmp_layer->object_data)->bg_color.blue = default_bg_colour.blue;

	// Create a blank thumbnail using the default background color
	tmp_gdk_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, preview_width, 233);
	gdk_pixbuf_fill(tmp_gdk_pixbuf, ((default_bg_colour.red / 255) << 24)
		+ ((default_bg_colour.green / 255) << 16)
		+ ((default_bg_colour.blue / 255) << 8) + 0xff);

	// Add the empty layer to the new slide being created
	tmp_slide->layers = g_list_append(tmp_slide->layers, tmp_layer);

	// Create the List store the slide layer data is kept in
	tmp_iter = g_new(GtkTreeIter, 1);
	tmp_layer->row_iter = tmp_iter;
	tmp_slide->layer_store = gtk_list_store_new(TIMELINE_N_COLUMNS,  // TIMELINE_N_COLUMNS
									G_TYPE_STRING,  // TIMELINE_NAME
									G_TYPE_BOOLEAN,  // TIMELINE_VISIBILITY
									G_TYPE_UINT,  // TIMELINE_START
									G_TYPE_UINT,  // TIMELINE_FINISH
									G_TYPE_UINT,  // TIMELINE_X_OFF_START
									G_TYPE_UINT,  // TIMELINE_Y_OFF_START
									G_TYPE_UINT,  // TIMELINE_X_OFF_FINISH
									G_TYPE_UINT);  // TIMELINE_Y_OFF_FINISH
	gtk_list_store_append(tmp_slide->layer_store, tmp_iter);
	gtk_list_store_set(tmp_slide->layer_store, tmp_iter,
					TIMELINE_NAME, tmp_layer->name->str,
					TIMELINE_VISIBILITY, TRUE,
					TIMELINE_START, 0,
					TIMELINE_FINISH, slide_length,
					TIMELINE_X_OFF_START, 0,
					TIMELINE_Y_OFF_START, 0,
					TIMELINE_X_OFF_FINISH, 0,
					TIMELINE_Y_OFF_FINISH, 0,
					-1);

	// Add the thumbnail to the new slide structure
	tmp_slide->thumbnail = GTK_IMAGE(gtk_image_new_from_pixbuf(tmp_gdk_pixbuf));

	// Create an event box for adding the thumbnail image to, so it can receive events and display tool tips
	tmp_slide->event_box = gtk_event_box_new();

	// Mark the tooltip for the slide as not-yet-created
	tmp_slide->tooltip = NULL;

	// Set the timeline widget for the slide to NULL, so we know to create it later on
	tmp_slide->timeline_widget = NULL;

	// Add the thumbnail to the event box
	gtk_container_add(GTK_CONTAINER(tmp_slide->event_box), GTK_WIDGET(tmp_slide->thumbnail));

	// Add a mouse click handler to the event box
	tmp_slide->click_handler = g_signal_connect(G_OBJECT(tmp_slide->event_box), "button_release_event", G_CALLBACK(film_strip_slide_clicked), tmp_slide);

	// If the current slide hasn't been initialised (this is the first slide), then we initialise it
	if (NULL == current_slide)
	{
		slides = g_list_append(slides, tmp_slide);
		current_slide = slides;
	} else
	{
		// Add the temporary slide after the presently selected slide
		slide_position = g_list_position(slides, current_slide);
		slides = g_list_insert(slides, tmp_slide, slide_position + 1);
	}

	// Recreate the slide tooltips
	create_tooltips();

	// Update the film strip area
	refresh_film_strip();

	// Update the status bar
	gtk_statusbar_push(GTK_STATUSBAR(status_bar), statusbar_context, "Added new slide.");
	gdk_flush();
}


// Function called when the user selects Slide -> Move down from the top menu
void slide_move_down(void)
{
	// Local variables
	GList				*next_slide;			// Pointer to the slide below
	gint				num_slides;				// The total number of slides
	gint				slide_position;			// Which slide in the slide list we are moving
	gpointer			this_slide_data;		// Pointer to the data for this slide


	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	num_slides = g_list_length(slides);
	if (num_slides == (slide_position + 1))
	{
		// We can't move the bottom most slide any further down, so just return
		return;
	}

	// Swap the slides around
	this_slide_data = current_slide->data;
	next_slide = g_list_nth(slides, slide_position + 1);
	current_slide->data = next_slide->data;
	next_slide->data = this_slide_data;
	current_slide = next_slide;

	// Recreate the slide tooltips
	create_tooltips();

	// Refresh the film strip area
	refresh_film_strip();
}


// Function called when the user selects Slide -> Move up from the top menu
void slide_move_up(void)
{
	// Local variables
	GList				*previous_slide;		// Pointer to the slide above
	gint				slide_position;			// Which slide in the slide list we are moving
	gpointer			this_slide_data;		// Pointer to the data for this slide


	// Safety check
	slides = g_list_first(slides);
	slide_position = g_list_position(slides, current_slide);
	if (0 == slide_position)
	{
		// We can't move the upper most slide any further up, so just return
		return;
	}

	// Swap the slides around
	this_slide_data = current_slide->data;
	previous_slide = g_list_nth(slides, slide_position - 1);
	current_slide->data = previous_slide->data;
	previous_slide->data = this_slide_data;
	current_slide = previous_slide;

	// Recreate the slide tooltips
	create_tooltips();

	// Refresh the film strip area
	refresh_film_strip();
}


// Function called when the user selects Slide -> Reset slide name from the top menu
void slide_name_reset(void)
{
	// Frees the string used for storing the present slide name, and sets it to NULL to mark as 'unset'
	g_string_free(((slide *) current_slide->data)->name, TRUE);
	((slide *) current_slide->data)->name = NULL;

	// Recreate the film strip tooltips
	create_tooltips();
}


// Function called when the user selects Slide -> Edit slide name from the top menu
void slide_name_set(void)
{
	// Local variables
	GtkDialog			*slide_dialog;			// Widget for the dialog
	GtkWidget			*dialog_table;			// Table used for neat layout of the dialog box
	gint				dialog_result;			// Catches the return code from the dialog box

	GtkWidget			*name_label;			// Label widget
	GtkWidget			*name_entry;			// Widget for accepting the name of the new layer

	GString				*tmp_gstring;			// Temporary GString


	// Initialise various things
	tmp_gstring = g_string_new(NULL);

	// * Display a dialog box asking for the new name of the slide *

	// Create the dialog window, and table to hold its children
	slide_dialog = GTK_DIALOG(gtk_dialog_new_with_buttons("Edit slide name", GTK_WINDOW(main_window), GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL));
	dialog_table = gtk_table_new(8, 2, FALSE);
	gtk_box_pack_start(GTK_BOX(slide_dialog->vbox), GTK_WIDGET(dialog_table), FALSE, FALSE, 0);

	// Create the label asking for the new layer name
	name_label = gtk_label_new("Slide name: ");
	gtk_misc_set_alignment(GTK_MISC(name_label), 0, 0.5);
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(name_label), 0, 1, 0, 1);

	// Create the entry that accepts the new layer name
	name_entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(name_entry), 50);
	if (NULL != ((slide *) current_slide->data)->name)
	{
		// Display the present name in the text field
		gtk_entry_set_text(GTK_ENTRY(name_entry), ((slide *) current_slide->data)->name->str);
	} else
	{
		// No present name, so just use the slide's position
		g_string_printf(tmp_gstring, "Slide %u", g_list_position(slides, current_slide) + 1);
		gtk_entry_set_text(GTK_ENTRY(name_entry), tmp_gstring->str);
	}
	gtk_table_attach_defaults(GTK_TABLE(dialog_table), GTK_WIDGET(name_entry), 1, 2, 0, 1);

	// Run the dialog
	gtk_widget_show_all(GTK_WIDGET(slide_dialog));
	dialog_result = gtk_dialog_run(GTK_DIALOG(slide_dialog));

	// Was the OK button pressed?
	if (GTK_RESPONSE_ACCEPT != dialog_result)
	{
		// * The user cancelled the dialog *

		// Destroy the dialog box then return
		gtk_widget_destroy(GTK_WIDGET(slide_dialog));
	}

	// fixme4: We should validate the user input (slide name) here

	// Set the new slide name
	if (NULL == ((slide *) current_slide->data)->name)
	{
		((slide *) current_slide->data)->name = g_string_new(NULL);
	}
	g_string_printf(((slide *) current_slide->data)->name, "%s", gtk_entry_get_text(GTK_ENTRY(name_entry)));

	// Destroy the dialog box
	gtk_widget_destroy(GTK_WIDGET(slide_dialog));

	// Recreate the film strip tooltips
	create_tooltips();

	// Free the resources allocated in this function
	g_string_free(tmp_gstring, TRUE);
}


/* 
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.60  2006/09/12 12:12:09  vapour
 * Started adding code to work out values for the swf header and write them to the output file.
 *
 * Revision 1.59  2006/09/05 13:11:00  vapour
 * Added initial code to write the FWS header string for flash files.
 *
 * Revision 1.58  2006/08/09 06:09:47  vapour
 * Expanded dialog boxes to have a field for an external link.
 *
 * Revision 1.57  2006/08/01 14:59:45  vapour
 * Updated the project file format number to 2.0.
 *
 * Revision 1.56  2006/07/30 04:58:50  vapour
 * Removed the ming includes, commented out the code needing ming, and added a notice to the user if they try and export flash that it's not  going to work as the code is being rewritten.
 *
 * Revision 1.55  2006/07/30 02:21:13  vapour
 * The control bar in exported svg now positions, scales, displays correctly, and controls things like it should.  i.e. it's usable now. :)
 *
 * Revision 1.54  2006/07/29 16:03:39  vapour
 * Control bar in exported svg now positions and scales correctly.
 *
 * Revision 1.53  2006/07/29 14:21:42  vapour
 * The pause button in the new exported svg control bar now scales.
 *
 * Revision 1.52  2006/07/24 12:00:45  vapour
 * The play button in the new exported svg control bar should now scale too.
 *
 * Revision 1.51  2006/07/23 12:37:15  vapour
 * + Added code to pause exported svg movies at the beginning.
 * + Started scaling the new exported svg playback controls.
 * + Added carriage returns to the exported svg Bitstream Vera font.
 *
 * Revision 1.50  2006/07/13 13:52:08  vapour
 * + More code to display the mouse pointer in the working area.
 * + Added functions to pauseAnimations and unpauseAnimations.
 * + Visually cleaned up the SVG exported for the play button in the control bar.
 * + Started adjusting the control bar to use the new pause functions.
 *
 * Revision 1.49  2006/07/09 08:06:46  vapour
 * Added the code to display the mouse pointer graphic in the working area.
 *
 * Revision 1.48  2006/07/06 14:55:06  vapour
 * Updated initial width and height attributes of mouse cursor image.
 *
 * Revision 1.47  2006/07/05 12:19:34  vapour
 * Fleshed out initial mouse pointer creation dialog box.
 *
 * Revision 1.46  2006/07/04 12:43:31  vapour
 * + Added a stub function for creating a new mouse pointer layer.
 *
 * Revision 1.45  2006/07/02 11:06:02  vapour
 * Added functions for disabling and enabling the layer toolbar buttons.
 *
 * Revision 1.44  2006/06/28 13:52:46  vapour
 * Marked some code that should no longer be needed.
 *
 * Revision 1.43  2006/06/28 13:42:54  vapour
 * Added further code for working with grayed out toolbar buttons.
 *
 * Revision 1.42  2006/06/27 13:43:37  vapour
 * Created initial stub functions for enabling and disabling the main toolbar icons.
 *
 * Revision 1.41  2006/06/26 22:35:24  vapour
 * Improved the error message when no screenshots are found, so the user will be able to check if their files are just named wrongly.
 *
 * Revision 1.40  2006/06/25 12:55:40  vapour
 * The standard roman character set for Bitstream Vera is now embedded in exported svg files.
 *
 * Revision 1.39  2006/06/22 14:24:56  vapour
 * + Typo fix for the svg stylesheet definition.
 * + Embedded some characters of an italic Bitstream Vera font, to verify the font is indeed being used (successful).
 *
 * Revision 1.38  2006/06/21 14:42:03  vapour
 * Started adding code to embed the Bitstream Vera fonts in the output svg.
 *
 * Revision 1.37  2006/06/18 13:42:29  vapour
 * + Added a popup warning about screenshots not working on Windows.
 * + Play button now scales and positions properly, and it also looks reasonable.
 *
 * Revision 1.36  2006/06/17 13:00:30  vapour
 * Continuing with replacing the ugly playback button with a nicer one.
 *
 * Revision 1.35  2006/06/15 09:56:34  vapour
 * Started replacing the ugly play button with a much nicer one from openclipart.org.
 *
 * Revision 1.34  2006/06/12 14:13:39  vapour
 * Playback control bar and buttons are now scaled and positioned reasonably well.
 *
 * Revision 1.33  2006/06/12 11:04:22  vapour
 * Started adding a playback control bar to the SVG output.
 *
 * Revision 1.32  2006/06/12 03:52:28  vapour
 * Updated many of the warning messages to go through the display_warning function.
 *
 * Revision 1.31  2006/06/10 15:38:41  vapour
 * Committed the code for setting and unsetting slide names, which I'd somehow managed to miss committing before.
 *
 * Revision 1.30  2006/06/10 15:35:26  vapour
 * The File -> New option no longer unloads the presently loaded project if the user cancels out.
 *
 * Revision 1.29  2006/06/10 15:28:14  vapour
 * Added slide name support to the slide_insert function.
 *
 * Revision 1.28  2006/06/10 15:07:24  vapour
 * + More visual realignment of variable names for my Linux system.
 * + Added functions to set and reset slide names.
 *
 * Revision 1.27  2006/06/08 12:13:32  vapour
 * Created a new function that generates tooltips.
 *
 * Revision 1.26  2006/06/07 15:24:06  vapour
 * + Updated the slide deletion code to use the new destroy_slide function.
 * + Updated so the resources presently allocated to a project are freed up when beginning a new project.
 *
 * Revision 1.25  2006/06/06 12:51:35  vapour
 * Fixed the bug causing deletion of slides to crash.
 *
 * Revision 1.24  2006/06/06 12:25:45  vapour
 * Improved the alphabetical order of functions.
 *
 * Revision 1.23  2006/06/04 06:12:33  vapour
 * Updated the svg output so element properties are initially defined in a <defs> section.
 *
 * Revision 1.22  2006/06/01 10:08:20  vapour
 * Updated to calculate the project height and width from imported screenshots.
 *
 * Revision 1.21  2006/05/31 14:02:03  vapour
 * Added code so the capture offsets are kept through a session.
 *
 * Revision 1.20  2006/05/28 09:52:59  vapour
 * Fixed a small bug, where the output resolution function wasn't detecting an existing match properly.
 *
 * Revision 1.19  2006/05/28 09:39:16  vapour
 * + Fixed a misleading comment.
 * + Re-tab aligned some variables for my Linux Eclipse.
 *
 * Revision 1.18  2006/05/24 14:57:32  vapour
 * Updated the zoom_selector creation function to dynamically include a new resolution if required.
 *
 * Revision 1.17  2006/05/23 13:12:55  vapour
 * Added an initial function to create the selection of available output resolutions dynamically from a list.
 *
 * Revision 1.16  2006/05/20 13:33:24  vapour
 * Fixed several places where I was using the wrong call to free memory, leading to potential crashing bugs.
 *
 * Revision 1.15  2006/05/17 11:23:51  vapour
 * Added the export_time_counter global variable.
 *
 * Revision 1.14  2006/05/15 13:40:50  vapour
 * + Now keeps track of the last file name used in a session, and makes that the default name for opening and saving of files.
 * + Changed ifdefs so gnome functions aren't used on Windows.
 *
 * Revision 1.13  2006/05/14 12:31:26  vapour
 * + Now keeps track of the last file name used in a session, and makes that the default name for opening and saving of files.
 * + Initial SVG tag is now broken up into lines and tabbed in, as per the exampl in Jonathon Watts SVG Authoring Guidelines (http://jwatt.org/svg/authoring).
 * + Added a warning dialog when overwriting an existing exported Flash file.
 * + Added a warning dialog when overwriting an existing exported SVG file.
 *
 * Revision 1.12  2006/04/27 16:40:40  vapour
 * Added a tag to the SVG output to showing it was created with the Flame Project.
 *
 * Revision 1.11  2006/04/26 18:34:48  vapour
 * Changes and tweaks to support the new project opening code.
 *
 * Revision 1.10  2006/04/25 10:56:26  vapour
 * Updated to appropriately set the modified flag in image layers.
 *
 * Revision 1.9  2006/04/25 09:55:25  vapour
 * Bug fix.  Importing screenshots into an existing project now works.
 *
 * Revision 1.8  2006/04/25 09:18:28  vapour
 * Added a overwrite warning dialog to the save option.
 *
 * Revision 1.7  2006/04/22 08:36:54  vapour
 * + Replaced the text string display in the timeline (layer) widget area, with the x and y finish positions.
 * + Updated the entire project to use the word "finish" consistently, instead of "final".
 *
 * Revision 1.6  2006/04/21 17:48:58  vapour
 * + Updated header with clearer copyright and license details.
 * + Moved the History section to the end of the file.
 *
 * Revision 1.5  2006/04/21 05:34:47  vapour
 * Added a dialog box for the Export As SVG option, asking the user where they want to save.
 *
 * Revision 1.4  2006/04/20 12:03:37  vapour
 * + Added a dialog box for the Export As Flash option, asking the user where they want to save.
 * + Quick fix to the flash exporting inner function, so it doesn't generate a warning message on empty layers.
 *
 * Revision 1.3  2006/04/18 18:00:52  vapour
 * Tweaks to allow compilation to succeed on both Windows and Solaris as well.
 * On Windows, the app will fire up as it only really required changes to not use GConf.
 * On Solaris however, a lot of stuff needed to be disabled, so it core dumps right away, prior to even displaying a window.
 * However, this *is* progress of a sort. :)
 *
 * Revision 1.2  2006/04/16 06:00:26  vapour
 * + Removed header info copied from my local repository.
 * + Included the LGPL license info.
 * + Fixed some potential g_list bugs that turned up when examining warning messages on MinGW.
 *
 * Revision 1.1  2006/04/13 15:59:54  vapour
 * Initial version, copied from my local CVS repository.
 */

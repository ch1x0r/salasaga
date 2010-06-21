#include "flex_mxml_operations.h"
#include <stdio.h>
#include <string.h>
#include <glib/gprintf.h>
#include <libxml/encoding.h>

// check, if libxml2 library was compiled with needed functionality, else stop compilation with error message
#ifndef LIBXML_WRITER_ENABLED
	#error libxml2 was compiled without LIBXML_WRITER_ENABLED. This flags needed to generate mxml files
#endif

#ifndef LIBXML_OUTPUT_ENABLED
	#error libxml2 was compiled without LIBXML_OUTPUT_ENABLED. This flags needed to generate mxml files
#endif

#define XML_FILE_ENCODING "UTF-8" // use UTF-8 codepage in xml files as default codepage

//---------------------------------------------------------------------------------------------
// flash compilation functions
//---------------------------------------------------------------------------------------------
flex_mxml_compilation_flash_options_t* flex_mxml_compilation_flash_options_create() {
	flex_mxml_compilation_flash_options_t* flash_options = g_malloc(sizeof(flex_mxml_compilation_flash_options_t));

	memset(flash_options, 0, sizeof(flex_mxml_compilation_flash_options_t));

	flash_options->creator = g_string_new("Salasaga");

	return flash_options;
}

void flex_mxml_compilation_flash_options_delete(flex_mxml_compilation_flash_options_t* flash_options) {
	if (flash_options->title) {
		g_string_free(flash_options->title,1);
	}
	if (flash_options->description) {
		g_string_free(flash_options->description,1);
	}
	if (flash_options->creator) {
		g_string_free(flash_options->creator,1);
	}
	if (flash_options->publisher) {
		g_string_free(flash_options->publisher,1);
	}
	if (flash_options->language) {
		g_string_free(flash_options->language,1);
	}

	g_free(flash_options);
}

gint flex_mxml_compile_to_swf(gchar* source_mxml_filename, gchar* destination_swf_filename,flex_mxml_compilation_flash_options_t* swf_options) {
	// prepare mxml compiller flags
	GString* mxml_compiller_parameters = g_string_sized_new(50);

	g_string_append_printf(mxml_compiller_parameters, "mxmlc %s -output %s ",source_mxml_filename, destination_swf_filename);

	// set additional options to mxmlc compiller
	if (swf_options != 0) {
		if (swf_options->framerate > 0 && swf_options->framerate < 64) {
			g_string_append_printf(mxml_compiller_parameters, "-default-frame-rate %i ", swf_options->framerate);
		}
		if (swf_options->width > 0 && swf_options->height > 0) {
			g_string_append_printf(mxml_compiller_parameters, "-default-size %i %i ", swf_options->width, swf_options->height);
		}
		if (swf_options->title != NULL) {
			g_string_append_printf(mxml_compiller_parameters, "-title %s ", swf_options->title->str);
		}
		if (swf_options->description != NULL) {
			g_string_append_printf(mxml_compiller_parameters, "-description %s ", swf_options->description->str);
		}
		if (swf_options->publisher != NULL) {
			g_string_append_printf(mxml_compiller_parameters, "-publisher %s ", swf_options->publisher->str);
		}
		if (swf_options->creator != NULL) {
			g_string_append_printf(mxml_compiller_parameters, "-creator %s ", swf_options->creator->str);
		}
		if (swf_options->language != NULL) {
			g_string_append_printf(mxml_compiller_parameters, "-language+=%s ", swf_options->language->str);
		}
		if (swf_options->background_color > 0) {
			g_string_append_printf(mxml_compiller_parameters, "-default-background-color 0x%x ", swf_options->background_color);
		}
	}

	if (get_debug_level()) {
		g_printf("run mxmlc : %s\n", mxml_compiller_parameters->str);
	}

	GError* error = NULL;
	gchar* stdout;
	gchar* stderr;
	gint exit_status;
	int return_value = 0;

	// call mxmlc using PATH environment variable (external)
	if (!g_spawn_command_line_sync(mxml_compiller_parameters->str, &stdout, &stderr,&exit_status,&error)) {

		// try to use internal mxml
		GString* flex_path = g_string_new(g_get_current_dir());
		g_string_append(flex_path, "/../../flex/sdk/bin/");
		g_string_append(flex_path, mxml_compiller_parameters->str);

		if (!g_spawn_command_line_sync(flex_path->str, &stdout, &stderr,&exit_status,&error)) {
			if (get_debug_level() && error) {
				g_printf("%s",error->message);
				g_printf("mxmlc compiller output: %s\n%s\n", stdout,stderr);
			}

			g_string_free(mxml_compiller_parameters,1);
			g_string_free(flex_path, 1);

			return -1;
		}
		g_string_free(flex_path, 1);
	}

	if (get_debug_level()) {
		g_printf("mxmlc compiller output: %s\n%s\n", stdout,stderr);
	}

	if (exit_status != 0) {
		return return_value = -1;
	}

	g_free(stdout);
	g_free(stderr);

	g_string_free(mxml_compiller_parameters,1);

	GString* command = g_string_sized_new(50);
	g_string_append_printf(command, "firefox file://%s", destination_swf_filename);

	if (get_debug_level()) {
		g_printf("run firefox : %s\n", command->str);
	}

	//TODO: add into salasaga checkbox and path to browser
	// show in browser generated flash
	g_spawn_command_line_async (command->str, NULL);

	g_string_free(command, 1);

	return return_value;
}

//---------------------------------------------------------------------------------------------
// manipulate with dom
//---------------------------------------------------------------------------------------------

flex_mxml_dom_t flex_mxml_create_document() {
    LIBXML_TEST_VERSION;

    flex_mxml_dom_t dom;

    dom.styleText = g_string_new("@namespace mx \"http://www.adobe.com/2006/mxml\";");
    dom.actionScriptText = g_string_sized_new(250);

    dom.doc = xmlNewDoc(BAD_CAST "1.0");

    dom.root = xmlNewNode(NULL, BAD_CAST "mx:Application");
	xmlNewProp(dom.root, BAD_CAST "xmlns:mx", BAD_CAST "http://www.adobe.com/2006/mxml");
	xmlNewProp(dom.root, BAD_CAST "layout", BAD_CAST "absolute");

	dom.style = xmlNewChild(dom.root, NULL, BAD_CAST "mx:Style", NULL);

    return dom;
}

void flex_mxml_close_document(flex_mxml_dom_t dom) {
	g_string_free(dom.styleText, 1);
	g_string_free(dom.actionScriptText, 1);

	xmlFreeDoc(dom.doc);
	xmlCleanupParser();
}

void flex_mxml_file_save(flex_mxml_dom_t dom,gchar* filepathname) {

	xmlNodePtr styleNodeValue = xmlNewText(BAD_CAST dom.styleText->str);
	xmlAddChild(dom.style, styleNodeValue);

	xmlDocSetRootElement(dom.doc, dom.root);
	// save DOM to file
	xmlSaveFile(filepathname, dom.doc);
}

//---------------------------------------------------------------------------------------------
// draw api
//---------------------------------------------------------------------------------------------
xmlNodePtr flex_mxml_shape_add_button(flex_mxml_dom_t dom, int x, int y, int width, int height, gchar* name) {
	/* Create a new XmlWriter for uri, with no compression. */
	GString* x_str = g_string_sized_new(50);
	GString* y_str = g_string_sized_new(50);
	GString* width_str = g_string_sized_new(50);
	GString* height_str = g_string_sized_new(50);

	g_string_printf(x_str, "%i",x);
	g_string_printf(y_str, "%i",y);
	g_string_printf(width_str, "%i",width);
	g_string_printf(height_str, "%i",height);

	xmlNodePtr node = xmlNewChild(dom.root, NULL, BAD_CAST "mx:Button",NULL);
	xmlNewProp(node, BAD_CAST "id", BAD_CAST name);
	xmlNewProp(node, BAD_CAST "label", BAD_CAST name);
	xmlNewProp(node, BAD_CAST "x", BAD_CAST x_str->str);
	xmlNewProp(node, BAD_CAST "y", BAD_CAST y_str->str);
	xmlNewProp(node, BAD_CAST "width", BAD_CAST width_str->str);
	xmlNewProp(node, BAD_CAST "height", BAD_CAST height_str->str);

	g_string_free(x_str,1);
	g_string_free(y_str,1);

	return node;
}

xmlNodePtr flex_mxml_shape_add_text(flex_mxml_dom_t* dom, gint x, gint y, gchar* text, gchar* font_style, gint font_size, flex_mxml_rgb_t font_color) {
	xmlNodePtr node = xmlNewChild(dom->root, NULL, BAD_CAST "mx:Text", NULL);

	GString* color_value = g_string_sized_new(10);
	GString* font_size_value = g_string_sized_new(7);
	GString* x_value = g_string_new(0);
	GString* y_value = g_string_new(0);

	// create color atribute for DOM node text
	g_string_printf(color_value, "0x%x%x%x", font_color.red, font_color.green, font_color.blue);

	// create font size atribute
	g_string_printf(font_size_value, "%i", font_size);

	g_string_printf(x_value, "%i", x);
	g_string_printf(y_value, "%i", y);

	// fill atributes of text node
	xmlNewProp(node, BAD_CAST "text", BAD_CAST text);
	xmlNewProp(node, BAD_CAST "color", BAD_CAST color_value->str);
	xmlNewProp(node, BAD_CAST "fontFamily", BAD_CAST font_style);
	xmlNewProp(node, BAD_CAST "fontSize", BAD_CAST font_size_value->str);
	xmlNewProp(node, BAD_CAST "x", BAD_CAST x_value->str);
	xmlNewProp(node, BAD_CAST "y", BAD_CAST y_value->str);

	g_string_free(color_value, 1);
	g_string_free(font_size_value, 1);
	g_string_free(x_value, 1);
	g_string_free(y_value, 1);

	return node;
}

xmlNodePtr flex_mxml_shape_add_line(flex_mxml_dom_t dom, int x, int y, int width, int height, int r, int g, int b) {
	/* Create a new XmlWriter for uri, with no compression. */
	xmlNodePtr node = xmlNewChild(dom.root, NULL, BAD_CAST "mx:Line",NULL);
	//xmlNewProp(dom.root, BAD_CAST "id", BAD_CAST "button1");

	gchar* lineActionScript = "var lineShape:Shape = new Shape(); \
							lineShape.graphics.lineStyle(2, 0x990000, .75);\
							lineShape.graphics.moveTo(0,0); \
							lineShape.graphics.lineTo(200,200); \
							this.rawChildren.addChild(lineShape);";
	//xmlNewProp(node, BAD_CAST "id", BAD_CAST "button1");
	return node;
}

//---------------------------------------------------------------------------------------------
// style api
//---------------------------------------------------------------------------------------------
void flex_mxml_set_application_bgcolor(flex_mxml_dom_t* dom, flex_mxml_rgb_t* bg_color) {
	GString* bg_color_str = g_string_sized_new(7);

	g_string_printf(bg_color_str, "0x%x%x%x", bg_color->red, bg_color->green, bg_color->blue);

	printf ("%s",bg_color_str->str);

	xmlNewProp(dom->root, BAD_CAST "backgroundColor", BAD_CAST bg_color_str->str);

	g_string_free(bg_color_str, 1);

}
/*void flex_mxml_set_style_application (flex_mxml_dom_t dom, flex_mxml_rgb_t background_color) {
	g_string_append_printf(dom.styleText, "mx|Application { background-color: #%x%x%x;}", background_color.red, background_color.green, background_color.blue);
}*/

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

/**
 * Create mxml DOM in memory, add standard xml header, create root element mx:Application
 * @return xmlDocPtr allocated pointer. If was some error, this value will be 0. You must remove this pointer, using flex_mxml_close_document function
 * @see flex_mxml_close_document
 */
flex_mxml_dom_t flex_mxml_create_document() {
    LIBXML_TEST_VERSION;

    flex_mxml_dom_t dom;

	//xmlDocPtr doc;	/* document pointers */
	//xmlNodePtr root_node = NULL, node = NULL, node1 = NULL;/* node pointers */

    dom.doc = xmlNewDoc(BAD_CAST "1.0");

    dom.root = xmlNewNode(NULL, BAD_CAST "mx:Application");
	xmlNewProp(dom.root, BAD_CAST "xmlns:mx", BAD_CAST "http://www.adobe.com/2006/mxml");

	dom.style = xmlNewChild(dom.root, NULL, BAD_CAST "mx:Style", NULL);
	//xmlNodeSetContent(dom.style, BAD_CAST "Application { background-color: #FFFAA0; }");
    xmlNodePtr node1 = xmlNewText(BAD_CAST "Application { background-color: #000AA0; }");
    xmlAddChild(dom.style, node1);


	xmlDocSetRootElement(dom.doc, dom.root);

    return dom;
}

void flex_mxml_file_save(flex_mxml_dom_t dom,gchar* filepathname) {
	// save DOM to file
	xmlSaveFile(filepathname, dom.doc);
}

void flex_mxml_close_document(flex_mxml_dom_t dom) {
	xmlFreeDoc(dom.doc);
	xmlCleanupParser();
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

xmlNodePtr flex_mxml_shape_add_button(flex_mxml_dom_t dom, int x, int y, gchar* name) {
	/* Create a new XmlWriter for uri, with no compression. */
	GString* x_str = g_string_sized_new(50);
	GString* y_str = g_string_sized_new(50);

	g_string_printf(x_str, "%i",x);
	g_string_printf(y_str, "%i",y);

	xmlNodePtr node = xmlNewChild(dom.root, NULL, BAD_CAST "mx:Button",NULL);
	xmlNewProp(node, BAD_CAST "id", BAD_CAST name);
	xmlNewProp(node, BAD_CAST "label", BAD_CAST name);
	xmlNewProp(node, BAD_CAST "x", BAD_CAST x_str->str);
	xmlNewProp(node, BAD_CAST "y", BAD_CAST y_str->str);
	xmlNewProp(node, BAD_CAST "width", BAD_CAST "100");
	xmlNewProp(node, BAD_CAST "height", BAD_CAST "20");


	g_string_free(x_str,1);
	g_string_free(y_str,1);

	return node;
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
	}
	g_debug("run mxmlc : %s\n", mxml_compiller_parameters->str);

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
			g_debug("%s",error->message);

			g_string_free(mxml_compiller_parameters,1);
			g_string_free(flex_path, 1);

			return -1;
		}
		g_string_free(flex_path, 1);
	}

	#ifdef FLEX_DIR
	}
	#endif

	g_printf("%s\n%s\n", stdout,stderr);

	if (exit_status != 0) {
		return return_value = -1;
	}

	g_free(stdout);
	g_free(stderr);

	g_string_free(mxml_compiller_parameters,1);

	GString* command = g_string_sized_new(50);
	g_string_append_printf(command, "firefox file://%s", destination_swf_filename);

	//TODO: add into salasaga checkbox and path to browser
	// show in browser generated flash
	g_spawn_command_line_async (command->str, NULL);

	g_string_free(command, 1);

	return return_value;
}

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

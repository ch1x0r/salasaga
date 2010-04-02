#ifndef FLEX_MXML_OPERATIONS_H_
#define FLEX_MXML_OPERATIONS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <glib.h>
#include <libxml/xmlwriter.h>

/**
 * Create mxml DOM in memory, add standard xml header, create root element mx:Application
 * @return xmlDocPtr allocated pointer. If was some error, this value will be 0. You must remove this pointer, using flex_mxml_close_document function
 * @see flex_mxml_close_document
 */
xmlDocPtr flex_mxml_create_document();

/**
 * Save mxml DOM into file
 * @param xmlDocPtr allocated in memory mxml file
 * @param gchar* file name, where to save mxml file
 */
void flex_mxml_file_save(xmlDocPtr doc,gchar* filepathname);

/**
 * Close opened xml document and free used memmory
 * @param xmlDocPtr allocated in memory xml file
 */
void flex_mxml_close_document(xmlDocPtr doc);

/**
 * Flash options
 */
typedef struct swf_options {
	gint framerate;			// home many frames per second will have output flash
	gint height;
	gint width;

	GString title;			// flash title
	GString description;	// flash description
	GString publisher;		// who publish flash
	GString creator;		// what generated flash. Containing text "Salasaga"
	GString language;		// flash language
} swf_options_t;

/**
 * Compile mxml file to swf file using flex mxmlc compiller
 */
gint flex_mxml_compile_to_swf(gchar* source_mxml_filename, gchar* destination_swf_filename, swf_options_t* swf_otpions);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FLEX_MXML_OPERATIONS_H_


#ifndef FLEX_MXML_OPERATIONS_H_
#define FLEX_MXML_OPERATIONS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <glib.h>
#include <libxml/xmlwriter.h>


/**
 * Flash compilation options
 */
typedef struct {
	gint framerate;			// home many frames per second will have output flash
	gint height;
	gint width;
	gint background_color;	// background flash color

	GString* title;			// flash title
	GString* description;	// flash description
	GString* publisher;		// who publish flash
	GString* creator;		// what generated flash. Containing text "Salasaga"
	GString* language;		// flash language
} flex_mxml_compilation_flash_options_t;

typedef struct {
	xmlDocPtr doc;
	xmlNodePtr root;
	xmlNodePtr style;
} flex_mxml_dom_t;

/**
 * Create flex_mxml_compilation_flash_options_t in a memory, and fill all fields with default values
 * @return flex_mxml_compilation_flash_options_t allocated in memory, field with default values fields. Do not forget to use
 * function flex_mxml_compilation_flash_options_delete to free used memory
 * @see flex_mxml_compilation_flash_options_delete
 */
flex_mxml_compilation_flash_options_t* flex_mxml_compilation_flash_options_create();

/**
 * Free used memory, that allocated using function flex_mxml_compilation_flash_options_create
 * @param flex_mxml_compilation_flash_options_t allocated in memory data struct
 */
void flex_mxml_compilation_flash_options_delete(flex_mxml_compilation_flash_options_t*);

/**
 * Create mxml DOM in memory, add standard xml header, create root element mx:Application
 * @return xmlDocPtr allocated pointer. If was some error, this value will be 0. You must remove this pointer, using flex_mxml_close_document function
 * @see flex_mxml_close_document
 */
flex_mxml_dom_t flex_mxml_create_document();

/**
 * Save mxml DOM into file
 * @param xmlDocPtr allocated in memory mxml file
 * @param gchar* file name, where to save mxml file
 */
void flex_mxml_file_save(flex_mxml_dom_t doc,gchar* filepathname);

/**
 * Close opened xml document and free used memmory
 * @param xmlDocPtr allocated in memory xml file
 */
void flex_mxml_close_document(flex_mxml_dom_t dom);

xmlNodePtr flex_mxml_shape_add_button(flex_mxml_dom_t dom);

xmlNodePtr flex_mxml_shape_add_line(flex_mxml_dom_t dom);

/**
 * Compile mxml file to swf file using flex mxmlc compiller
 * @param gchar* source_mxml_filename input mxml file
 * @param gchar* destination_swf_filename output swf file
 * @param swf_options_t* swf_otpions flash options. Can be NULL (just generate flash from mxml file)
 * @return 0 if generated fine or -1 if something goes wrong
 */
gint flex_mxml_compile_to_swf(gchar* source_mxml_filename, gchar* destination_swf_filename, flex_mxml_compilation_flash_options_t* swf_otpions);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FLEX_MXML_OPERATIONS_H_


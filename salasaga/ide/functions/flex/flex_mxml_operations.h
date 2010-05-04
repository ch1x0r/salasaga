#ifndef FLEX_MXML_OPERATIONS_H_
#define FLEX_MXML_OPERATIONS_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <glib.h>
#include <libxml/xmlwriter.h>

//---------------------------------------------------------------------------------------------
// flash compilation functions
//---------------------------------------------------------------------------------------------

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

	GString* styleText;
	GString* actionScriptText;
} flex_mxml_dom_t;

typedef struct {
	guint red;
	guint green;
	guint blue;
} flex_mxml_rgb_t;

enum font_styles { normal, italic };

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
 * Compile mxml file to swf file using flex mxmlc compiller
 * @param gchar* source_mxml_filename input mxml file
 * @param gchar* destination_swf_filename output swf file
 * @param swf_options_t* swf_otpions flash options. Can be NULL (just generate flash from mxml file)
 * @return 0 if generated fine or -1 if something goes wrong
 */
gint flex_mxml_compile_to_swf(gchar* source_mxml_filename, gchar* destination_swf_filename, flex_mxml_compilation_flash_options_t* swf_otpions);

//---------------------------------------------------------------------------------------------
// manipulate with dom
//---------------------------------------------------------------------------------------------

/**
 * Create empty DOM for mxml file
 * @return flex_mxml_dom_t
 */
flex_mxml_dom_t flex_mxml_create_document();

/**
 * Close opened xml document and free used memmory
 * @param xmlDocPtr allocated in memory xml file
 */
void flex_mxml_close_document(flex_mxml_dom_t dom);
/**
 * Save mxml DOM into file
 * @param xmlDocPtr allocated in memory mxml file
 * @param gchar* file name, where to save mxml file
 */
void flex_mxml_file_save(flex_mxml_dom_t doc,gchar* filepathname);

//---------------------------------------------------------------------------------------------
// draw api
//---------------------------------------------------------------------------------------------

/**
 * add button into mxml file
 */
xmlNodePtr flex_mxml_shape_add_button(flex_mxml_dom_t dom, gint x, gint y, gint widht, gint height, gchar* value);

/**
 * add text into mxml file
 */
xmlNodePtr flex_mxml_shape_add_text(flex_mxml_dom_t* dom, gint x, gint y, gchar* text, gchar* font_style, gint font_size, flex_mxml_rgb_t font_color);

/**
 * add line to mxml
 */
xmlNodePtr flex_mxml_shape_add_line(flex_mxml_dom_t dom, gint x, gint y, gint width, gint height, gint r, gint g, gint b);

//---------------------------------------------------------------------------------------------
// style api
//---------------------------------------------------------------------------------------------
/**
 * Set flash background color
 * @param @dom DOM object, where already build application tag
 * @param bg_color color of background
 */
void flex_mxml_set_application_bgcolor(flex_mxml_dom_t* dom, flex_mxml_rgb_t* bg_color);
//void flex_mxml_set_style_application (flex_mxml_dom_t dom, flex_mxml_rgb_t background_color);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FLEX_MXML_OPERATIONS_H_


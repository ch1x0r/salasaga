#ifndef FLEX_MXML_CREATE_SHAPE_H_
#define FLEX_MXML_CREATE_SHAPE_H_

// To keep C++ programs that include this header happy
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Math include
#include <math.h>

// Locale includes
#include <locale.h>
#include <langinfo.h>

// GTK include
#include <gtk/gtk.h>

#include "flex_mxml_operations.h"
#include "../../salasaga_types.h"
#include "../global_functions.h"

gboolean flex_create_shape( flex_mxml_dom_t* dom,layer* this_layer_data);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // FLEX_MXML_OPERATIONS_H_

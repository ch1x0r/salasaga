#include <check.h>		// include check frontend for unit tests
#include <stdlib.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include "../flex_mxml_operations.h"
/**
 * Print content of text file to stdout
 */
void print_text_file_to_stdout(gchar* file_name) {
	gchar* buffer;
	gsize buffer_length;
	GError* error;

	if (g_file_get_contents (file_name, &buffer, &buffer_length, &error) == 0) {
		g_free(buffer);
		g_printf("g_file_get_contents: %s\n", error->message);
	}

	g_printf("%s", buffer);
	g_free(buffer);
}

/**
 * Test 1
 * Test for property mxml file create, gust generate an empty mxml file but with right xml structure
 */
START_TEST (mxml_init_test) {
	gchar mxml_file_name[L_tmpnam];
	gchar swf_file_name[L_tmpnam];

	// get some temporary filename for mxml file
	fail_unless(tmpnam(mxml_file_name) == mxml_file_name, "Can't get temporary name for mxml file");

	// get some temporary filename for swf file
	fail_unless(tmpnam(swf_file_name) == swf_file_name, "Can't get temporary name for swf file");

	g_printf("mxml file at\t%s\nswf file at\t%s\n", mxml_file_name, swf_file_name);

	// create mxml DOM
	xmlDocPtr doc = flex_mxml_create_document();

	// check, if DOM created successfull
	fail_unless(doc != 0, "Can't create xml file");

	// save DOM to temporary file
	flex_mxml_file_save(doc, mxml_file_name);

	// print file content to stdout
	print_text_file_to_stdout(mxml_file_name);

	// compile mxml file into flash
	fail_unless(flex_mxml_compile_to_swf(mxml_file_name, swf_file_name,0) ==0, "Failed to compile mxml file");

	//remove temporary file from filesystem
	//fail_unless(g_remove(mxml_file_name) == 0, "can't remove mxml file");

	// remove DOM from memory
	flex_mxml_close_document(doc);
}
END_TEST


Suite* flex_mxml_operations_suit (void) {
	Suite *s = suite_create ("mxml");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");
	/* add test functions */
	tcase_add_test (tc_core, mxml_init_test);

	suite_add_tcase (s, tc_core);

	return s;
}

int main (void) {
	int number_failed;

	/* create test suite */
	Suite *s = flex_mxml_operations_suit ();

	/* create object, that will run test suites */
	SRunner *sr = srunner_create (s);

	/* run test suites in mode CK_NORMAL */
	srunner_run_all (sr, CK_NORMAL);

	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}


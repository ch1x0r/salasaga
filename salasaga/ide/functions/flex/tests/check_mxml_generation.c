#include <check.h>		// include check frontend for unit tests
#include <stdlib.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include "../flex_mxml_operations.h"
#include <string.h>

/**
 * Print content of text file to stdout
 */
void print_text_file_to_stdout(gchar* file_name) {
	gchar* buffer;
	gsize buffer_length;
	GError* error;

	if (g_file_get_contents (file_name, &buffer, &buffer_length, &error) == 0) {
		g_free(buffer);
		g_debug("g_file_get_contents: %s\n", error->message);
	}

	g_debug("%s", buffer);
	g_free(buffer);
}

/**
 * Test 1
 * Test for property mxml file create, gust generate an empty mxml file but with right xml structure
 */
START_TEST (mxml_init_test) {
	gchar mxml_file_name[L_tmpnam + sizeof(".mxml") + 1];
	gchar swf_file_name[L_tmpnam + sizeof(".swf") + 1];

	// get some temporary filename for mxml file
	fail_unless(tmpnam(mxml_file_name) != NULL, "Can't get temporary name for mxml file");
	strncat (mxml_file_name, ".mxml", sizeof(".mxml"));

	// get some temporary filename for swf file
	fail_unless(tmpnam(swf_file_name) != NULL, "Can't get temporary name for swf file");
	strncat (swf_file_name, ".swf", sizeof(".swf"));

	g_debug("mxml file at\t%s\nswf file at\t%s\n", mxml_file_name, swf_file_name);

	// create mxml DOM
	flex_mxml_dom_t dom = flex_mxml_create_document();

	// check, if DOM created successfull
	fail_unless(dom.doc != 0, "Can't create xml file");

	flex_mxml_shape_add_button(dom);

	// save DOM to temporary file
	flex_mxml_file_save(dom, mxml_file_name);

	// print file content to stdout
	print_text_file_to_stdout(mxml_file_name);

	// compile mxml file into flash
	fail_unless(flex_mxml_compile_to_swf(mxml_file_name, swf_file_name,0) ==0, "Failed to compile mxml file, see mxmlc compiller message");

	// check, if swf file was created by mxmlc compiller
	fail_unless(g_file_test(swf_file_name, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) == TRUE, "mxmlc compiler didn't create swf file from mxml file\n"
			"See mxmlc return error for more information");

	//remove temporary files from filesystem
	fail_unless(g_remove(mxml_file_name) == 0, "can't remove mxml file");
	fail_unless(g_remove(swf_file_name) == 0, "can't remove swf file");

	// remove DOM from memory
	flex_mxml_close_document(dom);
}
END_TEST


Suite* flex_mxml_operations_suit (void) {
	Suite *s = suite_create ("mxml");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");

	/* In this unit test we call external java to convert mxml file to swf. This call very slow */
	tcase_set_timeout(tc_core, 60);	// i think, 60 seconds will be enough to generate dummy swf file

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


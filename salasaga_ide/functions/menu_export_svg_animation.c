/*
 * $Id$
 *
 * Flame Project: Function called when the user selects Export -> SVG Animation from the top menu 
 * 
 * Copyright (C) 2007 Justin Clift <justin@postgresql.org>
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
#include <glib/gstdio.h>
#include <gtk/gtk.h>

// Gnome includes
#include <libgnome/gnome-url.h>

// XML includes
#include <libxml/xmlsave.h>

#ifdef _WIN32
	// Windows only code
	#include <windows.h>
	#include "flame-keycapture.h"
#endif

// Flame Edit includes
#include "../flame-types.h"
#include "../externs.h"
#include "display_warning.h"
#include "menu_export_svg_animation_slide.h"
#include "sound_beep.h"


void menu_export_svg_animation(void)
{
	// Local variables
	GtkFileFilter			*all_filter;				// Filter for *.*
	GError				*error = NULL;				// Pointer to error return structure
	GtkWidget 			*export_dialog;				// Dialog widget
	gchar				*filename;				// Pointer to the chosen file name
	gfloat				control_bar_x_offset;		   	// X offset for the onscreen play button
	gfloat				control_bar_y_offset;		   	// Y offset for the onscreen play button
	GIOStatus			return_value;				// Return value used in most GIOChannel functions
	GList				*slide_pointer;				// Points to the presently processing slide
	GtkFileFilter			*svg_filter;				// Filter for *.svg
	gboolean			unique_name;				// Switch used to mark when we have a valid filename
	GtkWidget			*warn_dialog;				// Widget for overwrite warning dialog
	gfloat				x_scale;				// Width scale factor for the scene
	gfloat				y_scale;				// Height scale factor for the scene

	gsize				tmp_gsize;				// Temporary gsize
	GString				*tmp_gstring;				// Temporary GString


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


/*
 * History
 * +++++++
 * 
 * $Log$
 * Revision 1.2  2007/10/06 11:38:28  vapour
 * Continued adjusting function include definitions.
 *
 * Revision 1.1  2007/09/29 04:22:12  vapour
 * Broke gui-functions.c and gui-functions.h into its component functions.
 *
 */

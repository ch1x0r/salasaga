check_PROGRAMS= check_mxml_generation

check_mxml_generation_SOURCES=check_mxml_generation.c
check_mxml_generation_LDFLAGS=-lcheck @GLIB_LIBS@ @LIBXML_LIBS@ ../../flex_mxml_operations.o

TESTS = check_mxml_generation

AM_CPPFLAGS=@GLIB_CFLAGS@ @LIBXML_CFLAGS@ -DFLEX_DIR=@abs_top_srcdir@/flex/sdk/bin/


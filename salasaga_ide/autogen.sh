#!/bin/bash

aclocal -I config -I m4
autoheader
touch stamp-h
autoconf
automake --add-missing -c
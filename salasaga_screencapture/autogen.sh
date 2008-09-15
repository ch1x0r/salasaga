#!/bin/bash

aclocal -I config
autoheader
touch stamp-h
autoconf
automake --add-missing -c

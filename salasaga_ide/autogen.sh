#!/bin/bash

aclocal
autoheader
touch stamp-h
autoconf
automake --add-missing -c

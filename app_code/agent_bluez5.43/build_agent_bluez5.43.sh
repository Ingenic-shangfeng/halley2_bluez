#!/bin/bash

export CC=mips-linux-gnu-gcc
export COMPILE_CROSS=mips-linux-gnu
export HOST=mips-linux-gnu
export PREFIX=/

set -e

DIR_CURRENT=`pwd`
COMPILE_FOLDER=$DIR_CURRENT/compileCode
OUTPUT_FOLDER=$DIR_CURRENT/output
SOURCE_CODE_FOLDER=$DIR_CURRENT/app_packages
APP_CODE=$DIR_CURRENT/app_code
APP_FOLDER=$DIR_CURRENT/file_system

	 PACMD_CODE="
		  $COMPILE_FOLDER/agent_bluez5.43/*.c
	 "
	 CFLAGS="
		  -I$COMPILE_FOLDER/pulseaudio-5.0/src
		  -I$COMPILE_FOLDER/pulseaudio-5.0
		  -I$OUTPUT_FOLDER/usr/include/dbus-1.0 \
		  -I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include \
		  -DHAVE_CONFIG_H
		  -Wall
	 "
	 LDFLAGS="
		  -L$OUTPUT_FOLDER/usr/lib
		  -L$OUTPUT_FOLDER/usr/local/lib
		  -L$OUTPUT_FOLDER/usr/lib/pulseaudio
		  -lsndfile
		  -lpulsecommon-5.0
		  -ljson-c
		  -ldbus-1
		  -lsndfile
		  -lasound
		  -lsbc
		  -lltdl
		  -pthread
		  -Wl,-rpath=/usr/lib/pulseaudio
	 "
	 $CC $PACMD_CODE $CFLAGS $LDFLAGS -o agent_bluez5.43

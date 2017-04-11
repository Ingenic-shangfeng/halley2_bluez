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

BLUEZ_4_101_NAME="bluez-4.101.tar.xz"
BLUEZ_HCIDUMP_NAME="bluez-hcidump-2.5.tar.xz"
EXPAT_2_1_0_NAME="expat-2.1.0.tar.gz"
DBUS_1_8_8_NAME="dbus-1.8.8.tar.gz"
ZLIB_1_2_8_NAME="zlib-1.2.8.tar.gz"
LIBFFI_3_1_NAME="libffi-3.1.tar.gz"
GLIB_2_40_0_NAME="glib-2.40.0.tar.xz"

./wget_app_packages.sh
rm -rf $COMPILE_FOLDER $OUTPUT_FOLDER
mkdir -p $COMPILE_FOLDER $OUTPUT_FOLDER

#Extract files
	 echo "Extracting bluez-4.101.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$BLUEZ_4_101_NAME -C $COMPILE_FOLDER/
	 echo "Extracting bluez-hcidump-2.5.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$BLUEZ_HCIDUMP_NAME -C $COMPILE_FOLDER/
	 echo "Extracting expat-2.1.0.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$EXPAT_2_1_0_NAME -C $COMPILE_FOLDER/
	 echo "Extracting dbus-1.8.8.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$DBUS_1_8_8_NAME -C $COMPILE_FOLDER/
	 echo "Extracting zlib-1.2.8.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$ZLIB_1_2_8_NAME -C $COMPILE_FOLDER/
	 echo "Extracting libffi-3.1.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$LIBFFI_3_1_NAME -C $COMPILE_FOLDER/
	 echo "Extracting glib-2.40.0.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$GLIB_2_40_0_NAME -C $COMPILE_FOLDER/

#compile
	 cd $COMPILE_FOLDER/zlib-1.2.8/
	 echo "building `pwd`"
		  ./configure
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/libffi-3.1/
	 echo "building `pwd`"
		  ./configure \
			  --host=$HOST \
			  --prefix=/usr \
			  --sysconfdir=/etc \
			  --disable-debug \
			  --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/glib-2.40.0/
	 echo "building `pwd`"
		  glib_cv_stack_grows=no \
		  glib_cv_uscore=no \
		  ac_cv_func_posix_getpwuid_r=yes \
		  ac_cv_func_posix_getgrgid_r=yes \
		  ac_cv_path_GLIB_COMPILE_RESOURCES=yes \
		  ./configure \
			   --prefix=/usr \
			   --host=$HOST \
			   --sysconfdir=/etc \
			   ZLIB_CFLAGS="-I$OUTPUT_FOLDER/usr/local/include" \
			   ZLIB_LIBS="-L$OUTPUT_FOLDER/usr/local/lib -lz" \
			   LIBFFI_CFLAGS="-I$OUTPUT_FOLDER/usr/include -I$OUTPUT_FOLDER/usr/lib/libffi-3.1/include/" \
			   LIBFFI_LIBS="-L$OUTPUT_FOLDER/usr/lib -lffi" \
			   PKG_CONFIG_LIBDIR="$OUTPUT_FOLDER/usr/lib" \
			   --disable-dependency-tracking \
			   --disable-debug \
			   --disable-mem-pools \
			   --disable-installed-tests \
			   --disable-always-build-tests \
			   --disable-largefile \
			   --enable-static \
			   --enable-shared \
			   --enable-included-printf \
			   --disable-selinux \
			   --disable-fam \
			   --disable-xattr \
			   --disable-libelf \
			   --disable-gtk-doc \
			   --disable-gtk-doc-html \
			   --disable-gtk-doc-pdf \
			   --disable-man \
			   --disable-dtrace \
			   --disable-systemtap \
			   --disable-coverage \
			   --disable-Bsymbolic \
			   --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/expat-2.1.0/
	 echo "building `pwd`"
		  ./configure \
			  --prefix=/usr \
			  --host=$HOST \
			  --enable-shared \
			  --disable-static \
			  --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/dbus-1.8.8/
	 echo "building `pwd`"
		  ./configure \
			  --host=$HOST \
			  --prefix=/usr \
			  --sysconfdir=/etc \
			  --disable-developer \
			  --disable-ansi \
			  --disable-verbose-mode \
			  --disable-asserts \
			  --disable-checks \
			  --disable-xml-docs \
			  --disable-doxygen-docs \
			  --enable-abstract-sockets \
			  --disable-selinux \
			  --disable-libaudit \
			  --disable-inotify \
			  --disable-kqueue \
			  --disable-console-owner-file \
			  --disable-launchd \
			  --disable-systemd \
			  --disable-embedded-tests \
			  --disable-modular-tests \
			  --disable-tests \
			  --disable-installed-tests \
			  --enable-epoll \
			  --disable-x11-autolaunch \
			  --disable-Werror \
			  --enable-stats \
			  --localstatedir=/var \
			  --without-x \
			  --with-dbus-user=dbus \
			  --with-system-socket=/var/run/dbus/system_bus_socket \
			  --with-system-pid-file=/var/run/messagebus.pid \
			  --with-sysroot="$OUTPUT_FOLDER" \
			  CFLAGS="-I$OUTPUT_FOLDER/usr/include" \
			  LDFLAGS="-L$OUTPUT_FOLDER/usr/lib"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER/

	 cd $COMPILE_FOLDER/bluez-4.101/
	 echo "building `pwd`"
		  ./configure \
			  --prefix=/usr \
			  --sysconfdir=/etc \
			  --host=$HOST \
			  --enable-static \
			  --enable-shared \
			  --disable-pie \
			  --disable-network \
			  --disable-sap \
			  --disable-serial \
			  --disable-input \
			  --disable-audio \
			  --enable-service \
			  --enable-health \
			  --enable-pnat \
			  --enable-gstreamer \
			  --disable-alsa \
			  --disable-usb \
			  --enable-tools \
			  --disable-bccmd \
			  --disable-pcmcia \
			  --disable-hid2hci \
			  --disable-dfutool \
			  --disable-hidd \
			  --disable-pand \
			  --disable-dund \
			  --disable-cups \
			  --enable-test \
			  --disable-datafiles \
			  --disable-debug \
			  --disable-maemo6 \
			  --disable-dbusoob \
			  --disable-wiimote \
			  --disable-hal \
			  --enable-gatt \
			  CFLAGS="-I$OUTPUT_FOLDER/usr/include -fdata-sections -ffunction-sections" \
			  LDFLAGS="-L$OUTPUT_FOLDER/usr/lib -Wl,--gc-sections" \
			  DBUS_CFLAGS="-I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include -I$OUTPUT_FOLDER/usr/include/dbus-1.0" \
			  DBUS_LIBS="-L$OUTPUT_FOLDER/usr/lib -ldbus-1 -lpthread -lrt" \
			  GLIB_CFLAGS="-I$OUTPUT_FOLDER/usr/lib/glib-2.0/include -I$OUTPUT_FOLDER/usr/include/glib-2.0" \
			  GLIB_LIBS="-L$OUTPUT_FOLDER/usr/lib -lglib-2.0 -pthread -lrt"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/bluez-hcidump-2.5/
	 echo "building `pwd`"
		  ./configure \
			  --prefix=/usr \
			  --host=$HOST
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 find $OUTPUT_FOLDER -exec mips-linux-gnu-strip {} \; > /dev/null 2>&1

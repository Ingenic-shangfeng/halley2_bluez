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

BLUEZ_5_43_NAME="bluez-5.43.tar.xz"
EXPAT_2_1_0_NAME="expat-2.1.0.tar.gz"
DBUS_1_8_8_NAME="dbus-1.8.8.tar.gz"
ZLIB_1_2_8_NAME="zlib-1.2.8.tar.gz"
LIBFFI_3_1_NAME="libffi-3.1.tar.gz"
GLIB_2_40_0_NAME="glib-2.40.0.tar.xz"
LIBTOOL_2_4="libtool-2.4.tar.xz"
PULSEAUDIO_5_0_NAME="pulseaudio-5.0.tar.xz"
LIBJSON_NAME="json-c-json-c-0.11-20130402.tar.gz"
LIBSNDFILE_NAME="libsndfile-1.0.25.tar.gz"
ALSA_LIB_NAME="alsa-lib-1.0.28.tar.bz2"
SBC_1_3_NAME="sbc-1.3.tar.gz"

./wget_app_packages.sh
rm -rf  $APP_FOLDER
rm -rf $COMPILE_FOLDER $OUTPUT_FOLDER
mkdir -p $COMPILE_FOLDER $OUTPUT_FOLDER
tar xf file_system.tar
cp -r $APP_CODE/* $COMPILE_FOLDER

#Extract files
	 echo "Extracting bluez-5.43.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$BLUEZ_5_43_NAME -C $COMPILE_FOLDER/
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
	 echo "Extracting libtool-2.4.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$LIBTOOL_2_4 -C $COMPILE_FOLDER/
	 echo "Extracting json-c-json-c-0.11-20130402.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$LIBJSON_NAME -C $COMPILE_FOLDER/
	 echo "Extracting libsndfile-1.0.25.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$LIBSNDFILE_NAME -C $COMPILE_FOLDER/
	 echo "Extracting alsa-lib-1.0.28.tar.bz2"
	 tar xf $SOURCE_CODE_FOLDER/$ALSA_LIB_NAME -C $COMPILE_FOLDER/
	 echo "Extracting sbc-1.3.tar.gz"
	 tar xzf $SOURCE_CODE_FOLDER/$SBC_1_3_NAME -C $COMPILE_FOLDER/
	 echo "Extracting pulseaudio-5.0.tar.xz"
	 tar xJf $SOURCE_CODE_FOLDER/$PULSEAUDIO_5_0_NAME -C $COMPILE_FOLDER/

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

	 cd $COMPILE_FOLDER/bluez-5.43/
	 echo "building `pwd`"
		  ./configure \
			  --prefix=/usr \
			  --sysconfdir=/etc \
			  --host=$HOST \
			  CFLAGS="-I$OUTPUT_FOLDER/usr/include -fdata-sections -ffunction-sections" \
			  LDFLAGS="-L$OUTPUT_FOLDER/usr/lib -Wl,--gc-sections" \
			  DBUS_CFLAGS="-I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include -I$OUTPUT_FOLDER/usr/include/dbus-1.0" \
			  DBUS_LIBS="-L$OUTPUT_FOLDER/usr/lib -ldbus-1 -lpthread -lrt" \
			  GLIB_CFLAGS="-I$OUTPUT_FOLDER/usr/lib/glib-2.0/include -I$OUTPUT_FOLDER/usr/include/glib-2.0" \
			  GLIB_LIBS="-L$OUTPUT_FOLDER/usr/lib -lglib-2.0 -pthread -lrt" \
			  --disable-dependency-tracking \
			  --disable-threads \
			  --disable-backtrace \
			  --disable-debug \
			  --disable-pie \
			  --enable-library \
			  --disable-test \
			  --enable-tools \
			  --disable-monitor \
			  --disable-udev \
			  --disable-cups \
			  --disable-obex \
			  --disable-client \
			  --disable-systemd \
			  --disable-datafiles \
			  --disable-manpages \
			  --enable-experimental \
			  --enable-deprecated \
			  --disable-sixaxis \
			  --disable-android \
			  --with-sysroot="$OUTPUT_FOLDER" \
			  --without-dbusconfdir \
			  --without-dbussystembusdir \
			  --without-dbussessionbusdir \
			  --without-udevdir \
			  --without-systemdsystemunitdir \
			  --with-systemduserunitdir
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/libtool-2.4/
	 echo "building `pwd`"
		  ./configure --host=$HOST --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/json-c-json-c-0.11-20130402/
	 echo "building `pwd`"
		  ./configure --host=$HOST --disable-oldname-compat
		  sed -i '/rpl_/d' config.h
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/libsndfile-1.0.25/
	 echo "building `pwd`"
		  ./configure \
			  --host=$HOST \
			  --disable-experimental \
			  --disable-bow-docs \
			  --disable-sqlite \
			  --disable-alsa \
			  --disable-external-libs \
			  --disable-octave \
			  --disable-test-coverage \
			  --disable-largefile \
			  --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/alsa-lib-1.0.28/
	 echo "building `pwd`"
		  ./configure \
			  --host=$HOST \
			  --disable-python \
			  --with-sysroot="$OUTPUT_FOLDER"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/sbc-1.3/
	 echo "building `pwd`"
		  ./configure --host=$HOST \
			  --with-sysroot="$OUTPUT_FOLDER" \
			  SNDFILE_CFLAGS="-I$OUTPUT_FOLDER/usr/local/include" \
			  SNDFILE_LIBS="-L$OUTPUT_FOLDER/usr/local/lib -lsndfile"
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/pulseaudio-5.0/
	 sed -i 's/DEFAULT_LATENCY_MSEC 200/DEFAULT_LATENCY_MSEC 500/g' \
		 ./src/modules/module-loopback.c
	 echo "building `pwd`"
		  ./configure \
			  --host=$HOST \
			  --prefix=/usr \
			  --enable-alsa \
			  --enable-dbus \
			  --enable-static \
			  --enable-shared \
			  --enable-dbus \
			  --enable-hal-compat \
			  --enable-bluez5 \
			  --disable-bluez4 \
			  --disable-legacy-database-entry-format \
			  --disable-openssl \
			  --disable-nls \
			  --disable-rpath          \
			  --disable-neon-opt       \
			  --disable-largefile      \
			  --disable-x11            \
			  --disable-tests          \
			  --disable-samplerate     \
			  --disable-oss-output     \
			  --disable-oss-wrapper    \
			  --disable-coreaudio-output \
			  --disable-esound         \
			  --disable-solaris        \
			  --disable-waveout        \
			  --disable-glib2          \
			  --disable-gtk3           \
			  --disable-gconf          \
			  --disable-avahi          \
			  --disable-jack           \
			  --disable-asyncns        \
			  --disable-tcpwrap        \
			  --disable-lirc           \
			  --disable-udev           \
			  --disable-ipv6           \
			  --disable-openssl        \
			  --disable-xen            \
			  --disable-orc            \
			  --disable-systemd        \
			  --disable-systemd-journal \
			  --disable-manpages       \
			  --disable-per-user-esound-socket  \
			  --disable-mac-universal  \
			  --disable-default-build-tests \
			  --disable-legacy-database-entry-format \
			  --disable-webrtc-aec \
			  --with-database=simple \
			  --without-caps     \
			  --without-fftw    \
			  --without-speex  \
			  --with-sysroot="$OUTPUT_FOLDER" \
			  CFLAGS="-I$OUTPUT_FOLDER/usr/local/include/" \
			  LDFLAGS="-L$OUTPUT_FOLDER/usr/local/lib -lltdl -L$OUTPUT_FOLDER/usr/lib -ldbus-1 -ljson-c -lsndfile -lsbc -lasound" \
			  DBUS_CFLAGS="-I$OUTPUT_FOLDER/usr/include/dbus-1.0 -I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include/" \
			  DBUS_LIBS="-L$OUTPUT_FOLDER/usr/lib -ldbus-1" \
			  LIBJSON_CFLAGS="-I$OUTPUT_FOLDER/usr/local/include/json-c" \
			  LIBJSON_LIBS="-L$OUTPUT_FOLDER/usr/local/lib -ljson-c" \
			  LIBSNDFILE_CFLAGS="-I$OUTPUT_FOLDER/usr/local/include" \
			  LIBSNDFILE_LIBS="-L$OUTPUT_FOLDER/usr/local/lib -lsndfile" \
			  SBC_CFLAGS="-I$OUTPUT_FOLDER/usr/local/include" \
			  SBC_LIBS="-L$OUTPUT_FOLDER/usr/local/lib -lsbc" \
			  ASOUNDLIB_CFLAGS="-I$OUTPUT_FOLDER/usr/include/ -I$OUTPUT_FOLDER/usr/include/alsa/" \
			  ASOUNDLIB_LIBS="-L$OUTPUT_FOLDER/usr/lib -lasound" \

		  sed -i '/HAVE_GETUID/d' config.h
		  make -j
		  make install DESTDIR=$OUTPUT_FOLDER
	 cd $COMPILE_FOLDER/agent_bluez5.43/
	 echo "building `pwd`"
		  $CC   \
			  -I$COMPILE_FOLDER/pulseaudio-5.0/src \
			  -I$COMPILE_FOLDER/pulseaudio-5.0 \
			  -I$OUTPUT_FOLDER/usr/include/dbus-1.0 \
			  -I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include \
			  -DHAVE_CONFIG_H \
			  -Wall \
			  -L$OUTPUT_FOLDER/usr/lib \
			  -L$OUTPUT_FOLDER/usr/local/lib \
			  -L$OUTPUT_FOLDER/usr/lib/pulseaudio \
			  -lsndfile \
			  -lpulsecommon-5.0 \
			  -ljson-c \
			  -ldbus-1 \
			  -lsndfile \
			  -lasound \
			  -lsbc \
			  -lltdl \
			  -pthread \
			  -Wl,-rpath=/usr/lib/pulseaudio \
			  *.c \
			  -o agent_bluez5.43
	 cd $COMPILE_FOLDER/avrcp_test_bluez5.43
	 echo "building `pwd`"
		  $CC   \
			   -I$OUTPUT_FOLDER/usr/include/dbus-1.0 \
			   -I$OUTPUT_FOLDER/usr/lib/dbus-1.0/include \
			   -L$OUTPUT_FOLDER/usr/lib \
			   -ldbus-1 \
			   -lpthread \
			   avrcp.c \
			   -o avrcp_test
	 cd $COMPILE_FOLDER/bluez_ble_server
	 echo "building `pwd`"
		  make

	 cd $DIR_CURRENT

	 cp $COMPILE_FOLDER/bluez-5.43/src/main.conf \
		 $APP_FOLDER/etc/bluetooth/
	 cp $COMPILE_FOLDER/bluez-5.43/src/bluetooth.conf \
		 $APP_FOLDER/etc/dbus-1/system.d/
	 cp $OUTPUT_FOLDER/usr/bin/pacmd \
		 $APP_FOLDER/usr/bin/
	 cp $OUTPUT_FOLDER/usr/bin/pulseaudio \
		 $APP_FOLDER/usr/bin/
	 cp $OUTPUT_FOLDER/usr/lib/libasound.so.2 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/local/lib/libjson-c.so.2 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/local/lib/libltdl.so.7 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/lib/libpulse.so.0 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/lib/libpulsecore-5.0.so \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/local/lib/libsbc.so.1 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/local/lib/libsndfile.so.1 \
		 $APP_FOLDER/usr/lib/
	 cp $OUTPUT_FOLDER/usr/libexec/bluetooth/bluetoothd \
		 $APP_FOLDER/usr/sbin/
	 cp $OUTPUT_FOLDER/usr/share/alsa/alsa.conf \
		 $APP_FOLDER/usr/share/alsa/
	 cp $OUTPUT_FOLDER/usr/share/alsa/cards/aliases.conf \
		 $APP_FOLDER/usr/share/alsa/cards/
	 cp $OUTPUT_FOLDER/usr/share/alsa/pcm/default.conf \
		 $APP_FOLDER/usr/share/alsa/pcm/
	 cp $OUTPUT_FOLDER/usr/lib/pulseaudio/libpulsecommon-5.0.so \
	 	   $APP_FOLDER/usr/lib/pulseaudio/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-alsa-sink.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-bluetooth-policy.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-bluetooth-discover.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-bluez5-discover.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-bluez5-device.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/libbluez5-util.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/libalsa-util.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/libcli.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-cli-protocol-unix.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/libprotocol-cli.so \
	 	   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $OUTPUT_FOLDER/usr/lib/pulse-5.0/modules/module-loopback.so \
		   $APP_FOLDER/usr/lib/pulse-5.0/modules/
	 cp $COMPILE_FOLDER/agent_bluez5.43/agent_bluez5.43 \
	 	   $APP_FOLDER/usr/sbin/agent
	 cp $COMPILE_FOLDER/avrcp_test_bluez5.43/avrcp_test \
	 	   $APP_FOLDER/usr/bin/
	 cp $COMPILE_FOLDER/bluez_ble_server/ble_server \
	 	   $APP_FOLDER/usr/bin/
	 
	 find file_system -exec mips-linux-gnu-strip {} \; > /dev/null 2>&1

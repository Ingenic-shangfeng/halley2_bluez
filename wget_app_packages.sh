# The code packages are use for compile BlueZ 5.43,
# Pulseaudio-5, ofono-1.18

SOURCE_CODE_FOLDER=./app_packages
mkdir -p $SOURCE_CODE_FOLDER

BLUEZ_5_43_NAME="bluez-5.43.tar.xz"
BLUEZ_5_43_URL="http://www.kernel.org/pub/linux/bluetooth/bluez-5.43.tar.xz"

EXPAT_2_1_0_NAME="expat-2.1.0.tar.gz"
EXPAT_2_1_0_URL="http://downloads.sourceforge.net/expat/expat-2.1.0.tar.gz"

DBUS_1_8_8_NAME="dbus-1.8.8.tar.gz"
DBUS_1_8_8_URL="https://dbus.freedesktop.org/releases/dbus/dbus-1.8.8.tar.gz"

ZLIB_1_2_8_NAME="zlib-1.2.8.tar.gz"
ZLIB_1_2_8_URL="http://www.zlib.net/fossils/zlib-1.2.8.tar.gz"

LIBFFI_3_1_NAME="libffi-3.1.tar.gz"
LIBFFI_3_1_URL="ftp://sourceware.org/pub/libffi/libffi-3.1.tar.gz"

GLIB_2_40_0_NAME="glib-2.40.0.tar.xz"
GLIB_2_40_0_URL="https://ftp.gnome.org/pub/gnome/sources/glib/2.40/glib-2.40.0.tar.xz"

LIBTOOL_2_4_NAME="libtool-2.4.tar.xz"
LIBTOOL_2_4_URL="http://mirrors.ustc.edu.cn/gnu/libtool/libtool-2.4.tar.xz"

PULSEAUDIO_5_0_NAME="pulseaudio-5.0.tar.xz"
PULSEAUDIO_5_0_URL="http://freedesktop.org/software/pulseaudio/releases/pulseaudio-5.0.tar.xz"

LIBJSON_NAME="json-c-json-c-0.11-20130402.tar.gz"
LIBJSON_URL="http://pkgs.fedoraproject.org/repo/extras/json-c/json-c-0.11-20130402.tar.gz/7013b2471a507942eb8ed72a5d872d16/json-c-0.11-20130402.tar.gz"

LIBSNDFILE_NAME="libsndfile-1.0.25.tar.gz"
LIBSNDFILE_URL="http://pkgs.fedoraproject.org/repo/pkgs/libsndfile/libsndfile-1.0.25.tar.gz/e2b7bb637e01022c7d20f95f9c3990a2/libsndfile-1.0.25.tar.gz"

ALSA_LIB_NAME="alsa-lib-1.0.28.tar.bz2"
ALSA_LIB_URL="ftp://ftp.alsa-project.org/pub/lib/alsa-lib-1.0.28.tar.bz2"

SBC_1_3_NAME="sbc-1.3.tar.gz"
SBC_1_3_URL="http://www.kernel.org/pub/linux/bluetooth/sbc-1.3.tar.gz"

OFONO_1_18_NAME="ofono-1.18.tar.xz"
OFONO_1_18_URL="https://www.kernel.org/pub/linux/network/ofono/ofono-1.18.tar.xz"

CheckPackage () {
	if [ -f "$SOURCE_CODE_FOLDER/$1" ]
	then
		echo -e "\033[33;32m$1 package found\e[0m"
	else
		echo -e "\033[33;33m$1 package not found, download now\e[0m"
		wget -O $SOURCE_CODE_FOLDER/$1 $2
	fi
}

CheckPackage $BLUEZ_5_43_NAME         $BLUEZ_5_43_URL
CheckPackage $EXPAT_2_1_0_NAME        $EXPAT_2_1_0_URL
CheckPackage $DBUS_1_8_8_NAME         $DBUS_1_8_8_URL
CheckPackage $ZLIB_1_2_8_NAME         $ZLIB_1_2_8_URL
CheckPackage $LIBFFI_3_1_NAME         $LIBFFI_3_1_URL
CheckPackage $GLIB_2_40_0_NAME        $GLIB_2_40_0_URL
CheckPackage $LIBTOOL_2_4_NAME        $LIBTOOL_2_4_URL
CheckPackage $PULSEAUDIO_5_0_NAME     $PULSEAUDIO_5_0_URL
CheckPackage $LIBJSON_NAME            $LIBJSON_URL
CheckPackage $LIBSNDFILE_NAME         $LIBSNDFILE_URL
CheckPackage $ALSA_LIB_NAME           $ALSA_LIB_URL
CheckPackage $SBC_1_3_NAME            $SBC_1_3_URL
CheckPackage $OFONO_1_18_NAME         $OFONO_1_18_URL

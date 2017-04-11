# The code packages are use for compile BlueZ 5.43,
# Pulseaudio-5, ofono-1.18

SOURCE_CODE_FOLDER=./app_packages
mkdir -p $SOURCE_CODE_FOLDER

BLUEZ_4_101_NAME="bluez-4.101.tar.xz"
BLUEZ_4_101_URL="http://www.kernel.org/pub/linux/bluetooth/bluez-4.101.tar.xz"

BLUEZ_HCIDUMP_NAME="bluez-hcidump-2.5.tar.xz"
BLUEZ_HCIDUMP_URL="http://www.kernel.org/pub/linux/bluetooth/bluez-hcidump-2.5.tar.xz"

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

CheckPackage () {
	if [ -f "$SOURCE_CODE_FOLDER/$1" ]
	then
		echo -e "\033[33;32m$1 package found\e[0m"
	else
		echo -e "\033[33;33m$1 package not found, download now\e[0m"
		wget -O $SOURCE_CODE_FOLDER/$1 $2
	fi
}

CheckPackage $BLUEZ_4_101_NAME        $BLUEZ_4_101_URL
CheckPackage $BLUEZ_HCIDUMP_NAME      $BLUEZ_HCIDUMP_URL
CheckPackage $EXPAT_2_1_0_NAME        $EXPAT_2_1_0_URL
CheckPackage $DBUS_1_8_8_NAME         $DBUS_1_8_8_URL
CheckPackage $ZLIB_1_2_8_NAME         $ZLIB_1_2_8_URL
CheckPackage $LIBFFI_3_1_NAME         $LIBFFI_3_1_URL
CheckPackage $GLIB_2_40_0_NAME        $GLIB_2_40_0_URL

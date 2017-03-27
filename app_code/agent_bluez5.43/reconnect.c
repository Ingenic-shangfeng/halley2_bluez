/*
 * reconnect.c
 *
 * Copyright (C) 2017 Ingenic Semiconductor Co., Ltd
 * Author: Feng Shang <feng.shang@ingenic.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dbus/dbus.h>
#include "reconnect.h"

#define INTERFACE_FREEDESKTOP_DBUS     "org.freedesktop.DBus.ObjectManager"

static int check_dev_addr(char *addr)
{
	int i;
	for(i=0;i<17;i++){
		if(!isxdigit(addr[i])){
			if(addr[i] != ':'){
				return -1;
			}
		}
	}
	return 0;
}
char *dev2path(char *dev)
{
	static char path[] = "/org/bluez/hci0/dev_00_00_00_00_00_00";
#define INDEX_OF_MAC 20
	int i;
	for(i=0;i<strlen(dev);i++){
		if(dev[i] != ':'){
			path[INDEX_OF_MAC + i] = dev[i];
		}
	}
	return path;
}
static void connect_dev(DBusConnection* conn, char *dev)
{
	DBusMessage* msg;
	char interface[] = "org.bluez.Device1";

	msg = dbus_message_new_method_call(
			"org.bluez", dev2path(dev), interface, "Connect");
	dbus_connection_send(conn, msg, NULL);
	dbus_message_unref(msg);
}
int reconnect(DBusConnection* conn)
{
	FILE *fp;
	char dev_addr[18] = {0};
	fp = fopen(RECONNECT_RECORD, "r");
	if(fp == NULL){
		return -1;
	}
	if(fgets(dev_addr, sizeof(dev_addr), fp) == NULL){
		return -1;
	}
	fclose(fp);
	if(check_dev_addr(dev_addr) < 0){
		return -1;
	}
	printf("Reconnect to device %s\n", dev_addr);
	connect_dev(conn, dev_addr);
	return 0;
}

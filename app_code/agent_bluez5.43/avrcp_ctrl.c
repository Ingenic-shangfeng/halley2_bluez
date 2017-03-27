/*
 * avrcp_ctrl.c
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


/* AVRCP control communicate with user and bluz
 * To user: use interface "/ingenic/avrcp/status"
 *     "Connected"
 *     "Disconnected"
 *     "playing"
 *     "stopped"
 * from user: use interface "/ingenic/avrcp/ctrl"
 *     "Play"
 *     "Stop"
 *     "Next"
 *     "Previous"
 *     "Pause"
 */

#include <dbus/dbus.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "avrcp_ctrl.h"

#define DEST_BLUEZ    "org.bluez"
static char player_path[80];
static char device_path[80];

dbus_bool_t init_avrcp_srv(DBusConnection* conn)
{
	DBusError err;
	// initialise the errors
	dbus_error_init(&err);
	// add a rule for which messages we want to see
	dbus_bus_add_match(conn,
			"type='signal',"
			"interface='org.freedesktop.DBus.ObjectManager'",
			&err);
	dbus_bus_add_match(conn,
			"type='signal',"
			"interface='org.freedesktop.DBus.Properties'",
			&err);
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Match Error (%s)\n", err.message);
		return FALSE;
	}
	return TRUE;
}

static int notify_status(DBusConnection* conn, char *status)
{
	DBusMessage* msg;
	msg = dbus_message_new_signal(PATH_DBUS_INGENIC_AVRCP_STATUS,
			INTERFACE_FREEDESKTOP_DBUS, status);
	if(msg){
		dbus_connection_send(conn, msg, NULL);
		dbus_connection_flush(conn);
		dbus_message_unref(msg);
		return 0;
	}
	return -1;
}

dbus_bool_t process_avrcp_status(DBusConnection *conn, DBusMessage *msg)
{
	const char *member;
	DBusMessageIter args;
	char *path;
	member = dbus_message_get_member(msg);
	if(member && !strcmp(member, "InterfacesAdded")){
		dbus_message_iter_init(msg, &args);
		dbus_message_iter_get_basic(&args, &path);
		if(strstr(path, "/player")){
			strcpy(player_path, path);
			printf("player_path : %s\n", path);
			notify_status(conn, "Connected");
		}else if(strstr(path, "/fd")){
			strcpy(device_path, path);
			*strstr(device_path, "/fd") = '\0';
			printf("device_path : %s\n", path);
		}
		return TRUE;
	}else if(member && !strcmp(member, "InterfacesRemoved")){
		dbus_message_iter_init(msg, &args);
		dbus_message_iter_get_basic(&args, &path);
		if(strstr(path, "/player")){
			printf("player_path removed\n");
			player_path[0] = '\0';
			notify_status(conn, "Disconnected");
		}else if(strstr(path, "/fd")){
			device_path[0] = '\0';
			printf("device_path removed\n");
		}
		return TRUE;
	}else if(member && !strcmp(member, "PropertiesChanged")){
		const char *dev_path;
		char *str;
		dev_path = dbus_message_get_path(msg);
		if(dev_path != NULL && !strcmp(dev_path, player_path)){
			dbus_message_iter_init(msg, &args);
			dbus_message_iter_get_basic(&args, &str);//'s'
			if(!strcmp(str, "org.bluez.MediaPlayer1")){
				dbus_message_iter_next(&args);
				dbus_message_iter_recurse(&args, &args);//'a'
				dbus_message_iter_recurse(&args, &args);//'e'
				dbus_message_iter_get_basic(&args, &str);//'s'
				if(!strcmp(str, "Status")){
					DBusMessageIter subIter;
					dbus_message_iter_next(&args);
					dbus_message_iter_recurse(&args, &subIter);//'v'
					dbus_message_iter_get_basic(&subIter, &str);//'s'
					notify_status(conn, str);
				}
			}
		}
	}
	return FALSE;
}

static void media_ctrl(DBusConnection* conn, char *path, char *method)
{
#define INTF_MEDIA_PLAYER    "org.bluez.MediaPlayer1"
	DBusMessage* msg;
	if(path[0] == '\0'){
		return;
	}
	msg = dbus_message_new_method_call(
			DEST_BLUEZ, path, INTF_MEDIA_PLAYER, method);
	dbus_connection_send(conn, msg, NULL);
	dbus_message_unref(msg);
}

dbus_bool_t process_avrcp_ctrl(DBusConnection *conn, DBusMessage *msg)
{
	const char *member, *path;
	path = dbus_message_get_path(msg);
	if(path && !strcmp(path, PATH_DBUS_INGENIC_AVRCP_CTRL)){
		member = dbus_message_get_member(msg);
		if(member != NULL){
			if(!strcmp(member, "Play")){
				media_ctrl(conn, player_path, "Play");
				return TRUE;
			}else if(!strcmp(member, "Stop")){
				media_ctrl(conn, player_path, "Stop");
				return TRUE;
			}else if(!strcmp(member, "Pause")){
				media_ctrl(conn, player_path, "Pause");
				return TRUE;
			}else if(!strcmp(member, "Next")){
				media_ctrl(conn, player_path, "Next");
				return TRUE;
			}else if(!strcmp(member, "Previous")){
				media_ctrl(conn, player_path, "Previous");
				return TRUE;
			}else if(!strcmp(member, "VolumeUp")){
				return TRUE;
			}else if(!strcmp(member, "VolumeDown")){
				return TRUE;
			}
		}
	}
	return FALSE;
}

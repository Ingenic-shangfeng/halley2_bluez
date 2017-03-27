/*
 * avrcp.c
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

#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#define PATH_DBUS_INGENIC_AVRCP_CTRL   "/ingenic/avrcp/ctrl"
#define PATH_DBUS_INGENIC_AVRCP_STATUS "/ingenic/avrcp/status"
#define PATH_DBUS_INGENIC_PULSE_PACMD  "/ingenic/pulse/pacmd"
#define INTERFACE_FREEDESKTOP_DBUS     "org.freedesktop.DBus.ObjectManager"

static DBusConnection* init_dbus(void);
static int media_ctrl(DBusConnection* conn, char *method);
static int media_status_get(DBusConnection* conn);
static int pacmd(DBusConnection* conn, char *command);

int main(int argc, char** argv)
{
	DBusConnection* conn = init_dbus();
	media_status_get(conn);
	while(1){
		char str[80];
		scanf("%s", str);
		if(!strcmp("volume", str)){
			int vol;
			static char buf[64];
			scanf("%d", &vol);
			sprintf(buf, "set-sink-volume 0 %d\n", vol);
			pacmd(conn, buf);
		}else{
			media_ctrl(conn, str);
		}
	}
	return 0;
}
static int pacmd(DBusConnection* conn, char *command)
{
	DBusMessage* msg;
	DBusMessageIter args;
	msg = dbus_message_new_signal(PATH_DBUS_INGENIC_PULSE_PACMD,
			INTERFACE_FREEDESKTOP_DBUS, "command");
	if(msg){
		dbus_message_iter_init_append(msg, &args);
		dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &command);
		dbus_connection_send(conn, msg, NULL);
		//dbus_connection_flush(conn);
		dbus_message_unref(msg);
		return 0;
	}
	return -1;
}
static DBusConnection* init_dbus(void)
{
	DBusConnection* conn;
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
	if (NULL == conn) {
		printf("dbus init failed\n");
		exit(1);
	}
	return conn;
}
static int media_ctrl(DBusConnection* conn, char *method)
{
	DBusMessage* msg;
	msg = dbus_message_new_signal(PATH_DBUS_INGENIC_AVRCP_CTRL,
			INTERFACE_FREEDESKTOP_DBUS, method);
	if(msg){
		dbus_connection_send(conn, msg, NULL);
		//dbus_connection_flush(conn);
		dbus_message_unref(msg);
		return 0;
	}
	return -1;
}
static void *get_status(void *p)
{
	DBusConnection* conn = (DBusConnection*)p;
	DBusMessage* msg;
	while(TRUE){
		dbus_connection_read_write(conn, 100);
		msg = dbus_connection_pop_message(conn);
		if(msg != NULL){
			const char *member, *path;
			path = dbus_message_get_path(msg);
			if(path && !strcmp(path, PATH_DBUS_INGENIC_AVRCP_STATUS)){
				member = dbus_message_get_member(msg);
				if(member != NULL){
					printf("A2DP Status : %s\n", member);
				}
			}
			dbus_message_unref(msg);
		}
	}
	return NULL;
}

static int media_status_get(DBusConnection* conn)
{
	pthread_t th_recv;
	DBusError err;
	dbus_error_init(&err);
	dbus_bus_add_match(conn,                                                             
			"type='signal',"
			"interface='org.freedesktop.DBus.ObjectManager'",
			&err);
	dbus_connection_flush(conn);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "Match Error (%s)\n", err.message);
		return -1;
	}
	pthread_create(&th_recv, NULL, get_status, conn);
	return 0;
}

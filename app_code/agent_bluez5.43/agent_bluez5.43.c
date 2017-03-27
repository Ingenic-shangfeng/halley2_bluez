/*
 * agent.c  - Auto process pairing request
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "pair_ctrl.h"
#include "avrcp_ctrl.h"
#include "reconnect.h"
#include "pulse_pacmd.h"

#define DEST_BLUEZ    "org.bluez"
#define PATH_BLUEZ    "/org/bluez"
#define PATH_AGENT    "/org/bluez/agent"
#define INTF_AGENT_MG "org.bluez.AgentManager1"
#define INTF_AGENT    "org.bluez.Agent1"
#define CAPABILITY    "NoInputNoOutput"


typedef dbus_bool_t (*msg_process_t)(DBusConnection *conn, DBusMessage *msg);

static msg_process_t msg_process[] = {
	process_pair_req,
	process_service_req,
	process_avrcp_status,
	process_avrcp_ctrl,
	process_pulse_pacmd,
};
int main(int argc, char** argv)
{
   DBusMessage* msg;
   DBusConnection* conn;

   // connect to the system bus and check for errors
   conn = dbus_bus_get(DBUS_BUS_SYSTEM, NULL);
   if (NULL == conn) {
      exit(1);
   }

   init_pairable(conn);
   init_avrcp_srv(conn);
   pulse_pacmd_init(conn);
   reconnect(conn);

   //Wait for processing pair request
	while (true) {
		//blocking read of the next available message
		dbus_connection_read_write(conn, 100);
		msg = dbus_connection_pop_message(conn);
		if(msg != NULL){
			int i;
			for(i=0;i<sizeof(msg_process)/sizeof(msg_process[0]);i++){
				dbus_bool_t res = msg_process[i](conn, msg);
				if(res){
					break;
				}
			}
			dbus_message_unref(msg);
		}
	}
}

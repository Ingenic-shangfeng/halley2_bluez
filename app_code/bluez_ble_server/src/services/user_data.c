/*
 * user_data.c
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "gatt.h"
#include "mgmt.h"
#include "log.h"
#include "mem_manage.h"
#define HCI_DEV_ID 0

#define UUID_SERV_BLE_SERV "5F00"
#define UUID_CHAC_BLE_DATA "5F5F"
const static struct gatts_if *gatts;
static bt_uuid_t *uuid_service, *uuid_server_data;

char data[256];

static void onConnectionStateChange(bdaddr_t *addr, int newState, void *pdata)
{
	if(GATT_STATUS_DISCONNECTED == newState){
		le_set_advertise_enable(HCI_DEV_ID);
	}
}
static void onCharacterRead(bdaddr_t *addr, bt_uuid_t *chac_uuid, void *pdata,
		UINT8 *read_rsp_buf, UINT16 *read_rsp_buf_len)
{
	if(!bt_uuid_cmp(chac_uuid, uuid_server_data)){
		*read_rsp_buf_len = strlen(data);
		strcpy((char*)read_rsp_buf, data);
	}
}
static void onCharacterWrite(bdaddr_t *addr, bt_uuid_t *chac_uuid,
	  UINT8 *buf, UINT16 len, void *pdata)
{
	if(!bt_uuid_cmp(chac_uuid, uuid_server_data)){
		memcpy(data, buf, len>sizeof(data)?sizeof(data):len);
	}
}
static void onDescriptorRead(bdaddr_t *addr, bt_uuid_t *desc_uuid,
		void *pdata, UINT16 *ret_desc)
{
}
static void onDescriptorWrite(bdaddr_t *addr, bt_uuid_t *desc_uuid,
	  UINT16 desc, void *pdata)
{
}
static void onMtuChanged(bdaddr_t *addr, int mtu, void *pdata)
{
}

static struct gatts_cb io_cb = {
	onConnectionStateChange,
	onCharacterRead,
	onCharacterWrite,
	onDescriptorRead,
	onDescriptorWrite,
	onMtuChanged,
};

void init_ble_server(void)
{
	struct gatt_service *service;
	struct gatt_character *server_data;

	uuid_service = bt_create_uuid_from_string(UUID_SERV_BLE_SERV);
	uuid_server_data = bt_create_uuid_from_string(UUID_CHAC_BLE_DATA);

	service = create_service(uuid_service);
	server_data = create_character(uuid_server_data,
			BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_WRITE);

	service_add_character(service, server_data);
	gatts = register_gatt_service(service, &io_cb, NULL);
	if(!gatts){
		Log.e("ble server init failed");
	}
}


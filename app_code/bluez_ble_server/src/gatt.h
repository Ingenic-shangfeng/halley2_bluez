/*
 * gatt.h
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

#ifndef __GATT_H__
#define __GATT_H__

#include "queue.h"
#include "bt_util.h"
#include "att.h"
#include "uuid.h"

#define DESCREPTOR_NOTIFICATION 0x0001
#define DESCREPTOR_INDICATION   0x0002

struct gatt_service;
struct gatt_character;

typedef enum {
	GATT_SUCCESS         = 0,
	GATT_FAILED_BUSY     = -1,
	GATT_FAILED_NOEXIST  = -2,
	GATT_FAILED_TIMEOUT  = -3,
	GATT_FAILED_NOMEM    = -4,
} gatt_status_t;

enum{
	GATT_STATUS_CONNECTED,
	GATT_STATUS_DISCONNECTED,
};

struct gatts_if {
	int (*sendNotification)(bdaddr_t *addr, bt_uuid_t *chac_uuid, UINT8 *buf, UINT16 len);
	//int (*sendIndication)(bdaddr_t *addr, bt_uuid_t *chac_uuid, char *buf, UINT16 len);
	///*, callback*/);
};

struct gatts_cb {
	void (*onConnectionStateChange)(bdaddr_t *addr, int newState, void *pdata);
	void (*onCharacterRead)(bdaddr_t *addr, bt_uuid_t *chac_uuid, void *pdata,
			UINT8 *read_rsp_buf, UINT16 *read_rsp_buf_len);
	void (*onCharacterWrite)(bdaddr_t *addr, bt_uuid_t *chac_uuid,
			UINT8 *buf, UINT16 len, void *pdata);
	void (*onDescriptorRead)(bdaddr_t *addr, bt_uuid_t *desc_uuid, void *pdata,
			UINT16 *ret_desc);
	void (*onDescriptorWrite)(bdaddr_t *addr, bt_uuid_t *desc_uuid,
			UINT16 desc, void *pdata);
	void (*onMtuChanged)(bdaddr_t *addr, int mtu, void *pdata);
	//void onIndicationSent(bdaddr_t *addr, int status, void *pdata);
};
struct gattc_cb{
	void (*onCharacteristicChanged)(bt_uuid_t *characteristic, void *pdata);
	void (*onCharacteristicRead)(bt_uuid_t *characteristic, int status, void *pdata);
	void (*onCharacteristicWrite)(bt_uuid_t *characteristic, int status, void *pdata);
	void (*onDescriptorRead)(bt_uuid_t *descriptor, int status, void *pdata);
	void (*onDescriptorWrite)(bt_uuid_t *descriptor, int status, void *pdata);
	void (*onMtuChanged)(bdaddr_t *addr, int mtu, int status, void *pdata);
	void (*onReadRemoteRssi)(bdaddr_t *addr, int rssi, int status, void *pdata);
	void (*onServicesDiscovered)(bdaddr_t *addr, int status, void *pdata);
	void (*onConnectionStateChange)(bdaddr_t *addr, int newState, void *pdata);

	/* unfinished */
};

int init_gatt(int hdev);
//server api
struct gatt_service* create_service(bt_uuid_t *uuid);
struct gatt_character* create_character(bt_uuid_t *uuid, UINT8 prop);
int service_add_character(struct gatt_service *service, struct gatt_character *character);
const struct gatts_if* register_gatt_service(
		struct gatt_service* gatt_service, struct gatts_cb *io_cb, void *pdata);
//client api
const struct gatt_client* register_gatt_client(struct gattc_cb *io_cb, void *pdata);

#endif /* __GATT_H__ */

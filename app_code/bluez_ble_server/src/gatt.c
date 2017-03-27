/*
 * gatt.c
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
#include "att.h"
#include "gatt.h"
#include "log.h"
#include "bt_mutex.h"

struct gatt_service {
	//int (*sendIndication)(bdaddr_t *addr, char *buf, UINT16 len);
	//int (*sendNotification)(bdaddr_t *addr, char *buf, UINT16 len/*, callback*/);
	UINT16 handle_start;
	UINT16 handle_end;
	bt_uuid_t *uuid;
	struct queue *character_list; /* type of struct gatt_character */
	struct gatts_cb *io_cb;
	struct gatts_if io_if;
	void *pdata;
};

struct gatt_character {
	bt_uuid_t *uuid;
	UINT16 type_handle;
	UINT16 value_handle;
	UINT16 desc_handle;
	UINT8 prop;
};

struct common_rsp_func {
	void (*mtu_rsp)(bdaddr_t *addr, UINT16 mtu, void *pdata);
	void (*find_info_rsp)(bdaddr_t *addr,
			UINT16 start_handle, UINT16 end_handle, void *pdata);
	void (*read_by_type_rsp)(bdaddr_t *addr,
			UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata);
	void (*read_by_grp_type_rsp)(bdaddr_t *addr,
			UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata);
};
struct gatt_services {
	bt_mutex_t gatt_mutex;
	struct queue *service_list; /* type of struct gatt_service */
	struct common_rsp_func *common_rsp_func;
	struct att *att_op;
};

static struct gatt_services* create_services(void);
static void mtu_rsp(bdaddr_t *addr, UINT16 mtu, void *pdata);
static void find_info_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, void *pdata);
static void read_by_type_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata);
static void read_by_grp_type_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata);
static int gatts_send_notification(bdaddr_t *addr, bt_uuid_t *chac_uuid,
		UINT8 *buf, UINT16 len);

static struct common_rsp_func common_rsp_func = {
	mtu_rsp,
	find_info_rsp,
	read_by_type_rsp,
	read_by_grp_type_rsp,
};
static void conn_change_cb(bdaddr_t addr, int status, void *pdata);
static int onReceive(bdaddr_t *addr, UINT8 opcode,
		const void *pdu, UINT16 length, void *pdata);
struct att_cb att_cb = {
	conn_change_cb,
	onReceive,
};

/*
	int (*connect)(bdaddr_t *addr);
	int (*disconnect)(bdaddr_t *addr);
	int (*send)(bdaddr_t *addr, struct att_send_op *att_send_op);
*/
static const struct att *att;
static struct gatt_services *gatt_services;
int init_gatt(int hdev)
{
	gatt_services = create_services();
	if(!gatt_services){
		Log.e("create services failed");
		goto create_services_failed;
	}
	if(bt_mutex_init(&gatt_services->gatt_mutex)<0){
		Log.e("mutex init failed");
		goto mutex_init_failed;
	}
	att = register_att(hdev, &att_cb, gatt_services);
	if(!att){
		Log.e("register att failed");
		goto register_att_failed;
	}
	gatt_services->att_op = (struct att*)att;
	gatt_services->common_rsp_func = &common_rsp_func;
	return GATT_SUCCESS;
register_att_failed :
mutex_init_failed :
create_services_failed :
	return GATT_FAILED_NOMEM;
}
static void gatt_send(bdaddr_t *addr, struct gatt_services* gatt_services,
		UINT16 opcode, UINT8 *rsp, UINT16 rsp_len)
{
	struct att_send_op send_op;
	memset(&send_op, 0, sizeof(struct att_send_op));
	send_op.opcode = opcode;
	send_op.pdu = rsp;
	send_op.len= rsp_len;
	gatt_services->att_op->send(addr, &send_op);
}
static void send_att_error(bdaddr_t *addr, struct gatt_services* gatt_services,
		UINT16 opcode, UINT16 handle, UINT8 err_code)
{
	UINT8 rsp[4], *p = rsp;
	UINT16 rsp_len;
	UINT8_TO_STREAM(p, opcode);
	UINT16_TO_STREAM(p, handle);
	UINT8_TO_STREAM(p, err_code);
	rsp_len = p - rsp;
	gatt_send(addr, gatt_services, BT_ATT_OP_ERROR_RSP, rsp, rsp_len);
}
struct device_status {
	bdaddr_t *addr;
	int status;
};
static void gatt_conn_change_cb(void *data, void *user_data)
{
	struct gatt_service *gatt_service = data;
	struct device_status *dev_status = user_data;
	gatt_service->io_cb->onConnectionStateChange(
			dev_status->addr, dev_status->status, gatt_service->pdata);
}
static void conn_change_cb(bdaddr_t addr, int status, void *pdata)
{
	struct gatt_services *gatt_services = pdata;
	struct device_status dev_status;
	dev_status.addr = &addr; 
	dev_status.status = status; 
	queue_foreach(gatt_services->service_list, gatt_conn_change_cb, &dev_status);
}

struct handle_info {
	struct gatt_service *gatt_service;
	struct gatt_character *gatt_character;
	handle_type_t type;
};
static bool find_gatt_service_by_handle_cb(const void *data, const void *match_data)
{
	struct gatt_service *gatt_service = (struct gatt_service*)data;
	UINT16 *handle = (UINT16*) match_data;
	if(gatt_service->handle_start <= *handle &&
			gatt_service->handle_end >= *handle){
		return true;
	}else{
		return false;
	}
}
static bool find_gatt_character_by_handle_cb(const void *data, const void *match_data)
{
	struct gatt_character *gatt_character;
	UINT16 *handle;
	gatt_character = (struct gatt_character*)data;
	handle = (UINT16*)match_data;
	if((*handle == gatt_character->value_handle-1) ||
			(*handle == gatt_character->value_handle) ||
			(*handle == gatt_character->desc_handle)){
		return true;
	}
	return false;
}
static int get_handle_info(struct handle_info *handle_info, UINT16 handle)
{
	struct gatt_service *gatt_service;
	struct gatt_character *gatt_character;
	bt_mutex_t *mutex;
	if(handle == 0 || handle >= 0xFFFF){
		goto services_no_exsit_failed;
	}
	memset(handle_info, 0, sizeof(struct handle_info));
	if(!gatt_services){
		Log.e("gatt server not inited");
		goto services_no_exsit_failed;
	}
	mutex = &gatt_services->gatt_mutex;
	bt_mutex_lock(mutex);
	gatt_service = queue_find(gatt_services->service_list,
			find_gatt_service_by_handle_cb, &handle);
	if(!gatt_service){
		goto get_handle_info_failed;
	}
	handle_info->gatt_service = gatt_service;
	gatt_character = queue_find(gatt_service->character_list,
			find_gatt_character_by_handle_cb, &handle);
	if(!gatt_character){
		goto get_handle_info_failed;
	}
	handle_info->gatt_character = gatt_character;
	if(handle == gatt_character->value_handle){
		handle_info->type = HANDLE_CHARACTER;
	}else if(handle == gatt_character->desc_handle){
		handle_info->type = HANDLE_DESC_CONF;
	}else{
		goto get_handle_info_failed;
	}
	bt_mutex_unlock(mutex);
	return GATT_SUCCESS;
get_handle_info_failed :
	bt_mutex_unlock(mutex);
services_no_exsit_failed :
	return GATT_FAILED_NOEXIST;
}

static int onReceive(bdaddr_t *addr, UINT8 opcode,
		const void *pdu, UINT16 length, void *pdata)
{
	struct gatt_services *gatt_services = pdata;
	static UINT8 rsp_pdu[1024];
	static UINT16 rsp_len;
	//UINT8 req_opcode = get_req_opcode(opcode);
	//rsq_len = 0;
	UINT8 *p = (UINT8*)pdu;
	switch(opcode){
		case BT_ATT_OP_ERROR_RSP :
			break;
		case BT_ATT_OP_MTU_REQ :{
			UINT16 mtu;
			STREAM_TO_UINT16(mtu, p);
			gatt_services->common_rsp_func->mtu_rsp(addr, mtu, gatt_services);
			}break;
		case BT_ATT_OP_MTU_RSP :
			break;
		case BT_ATT_OP_FIND_INFO_REQ :{
			UINT16 start_handle, end_handle;
			STREAM_TO_UINT16(start_handle, p);
			STREAM_TO_UINT16(end_handle, p);
			gatt_services->common_rsp_func->find_info_rsp(addr, start_handle, end_handle, gatt_services);
			}break;
		case BT_ATT_OP_FIND_INFO_RSP :
			break;
		case BT_ATT_OP_READ_BY_TYPE_REQ :{
			UINT16 start_handle, end_handle, remain_len;
			bt_uuid_t uuid;
			STREAM_TO_UINT16(start_handle, p);
			STREAM_TO_UINT16(end_handle, p);
			remain_len = length - (p-(UINT8*)pdu);
			if(remain_len  == 2){
				uint16_t tmp;
				STREAM_TO_UINT16(tmp, p);
				bt_uuid16_create(&uuid, tmp);
			}else if(remain_len  == 16){
				uint128_t tmp;
				STREAM_TO_ARRAY(&tmp, p, 16);
				bt_uuid128_create(&uuid, tmp);
			}else{
				Log.e("format error : %s@%d", __func__, __LINE__);
			}
			gatt_services->common_rsp_func->read_by_type_rsp(
					addr, start_handle, end_handle, &uuid, gatt_services);
			}break;
		case BT_ATT_OP_READ_BY_TYPE_RSP :
			break;
		case BT_ATT_OP_READ_BY_GRP_TYPE_REQ :{
			UINT16 start_handle, end_handle, remain_len;
			bt_uuid_t uuid;
			STREAM_TO_UINT16(start_handle, p);
			STREAM_TO_UINT16(end_handle, p);
			remain_len = length - (p-(UINT8*)pdu);
			if(remain_len  == 2){
				uint16_t tmp;
				STREAM_TO_UINT16(tmp, p);
				bt_uuid16_create(&uuid, tmp);
			}else if(remain_len  == 16){
				uint128_t tmp;
				STREAM_TO_ARRAY(&tmp, p, 16);
				bt_uuid128_create(&uuid, tmp);
			}else{
				Log.e("format error : %s@%d", __func__, __LINE__);
			}
			gatt_services->common_rsp_func->read_by_grp_type_rsp(
					addr, start_handle, end_handle, &uuid, gatt_services);
			}break;
		case BT_ATT_OP_READ_BY_GRP_TYPE_RSP :
			break;
		case BT_ATT_OP_READ_REQ :
		case BT_ATT_OP_WRITE_REQ :{
			UINT16 handle;
			struct handle_info handle_info;
			STREAM_TO_UINT16(handle, p);
			if(get_handle_info(&handle_info, handle) == GATT_SUCCESS){
				switch(handle_info.type){
					//the end of read & write need process
					case HANDLE_CHARACTER :
						if(opcode == BT_ATT_OP_WRITE_REQ){
							gatt_send(addr, gatt_services,
									BT_ATT_OP_WRITE_RSP, NULL, 0);
							handle_info.gatt_service->io_cb->onCharacterWrite(
									addr,
									handle_info.gatt_character->uuid,
									p,
									length - ((UINT8*)p - (UINT8*)pdu),
									handle_info.gatt_service->pdata);
						}else if(opcode == BT_ATT_OP_READ_REQ){
							rsp_len = 0;
							handle_info.gatt_service->io_cb->onCharacterRead(
									addr, handle_info.gatt_character->uuid,
									handle_info.gatt_service->pdata,
									rsp_pdu, &rsp_len);
							if(rsp_len < 0){
								rsp_len = 0;
							}
							gatt_send(addr, gatt_services,
									BT_ATT_OP_READ_RSP, rsp_pdu, rsp_len);
						}
						break;
					case HANDLE_DESC_CONF :
						if(opcode == BT_ATT_OP_WRITE_REQ){
							UINT16 desc_value;
							gatt_send(addr, gatt_services,
									BT_ATT_OP_WRITE_RSP, NULL, 0);
							STREAM_TO_UINT16(desc_value, p);
							handle_info.gatt_service->io_cb->onDescriptorWrite(
									addr,
									handle_info.gatt_character->uuid,
									desc_value,
									handle_info.gatt_service->pdata);
						}else if(opcode == BT_ATT_OP_READ_REQ){
							UINT16 desc_value;
							rsp_len = 0;
							handle_info.gatt_service->io_cb->onDescriptorRead(
									addr, handle_info.gatt_character->uuid,
									handle_info.gatt_service->pdata,
									&desc_value);
							if(rsp_len < 0){
								rsp_len = 0;
							}
							gatt_send(addr, gatt_services,
									BT_ATT_OP_READ_RSP, (UINT8*)&desc_value, 2);
						}
						break;
					default :
						break;
				}
			}else{

				Log.e("handle not found");
			}
			}break;
		case BT_ATT_OP_HANDLE_VAL_NOT :
		case BT_ATT_OP_HANDLE_VAL_IND :
		case BT_ATT_OP_READ_RSP :
		case BT_ATT_OP_WRITE_RSP :
			//use for client
			break;
		case BT_ATT_OP_FIND_BY_TYPE_VAL_REQ :
		case BT_ATT_OP_FIND_BY_TYPE_VAL_RSP :
		case BT_ATT_OP_READ_BLOB_REQ :
		case BT_ATT_OP_READ_BLOB_RSP :
		case BT_ATT_OP_READ_MULT_REQ :
		case BT_ATT_OP_READ_MULT_RSP :
		case BT_ATT_OP_WRITE_CMD :
		case BT_ATT_OP_SIGNED_WRITE_CMD :
		case BT_ATT_OP_PREP_WRITE_REQ :
		case BT_ATT_OP_PREP_WRITE_RSP :
		case BT_ATT_OP_EXEC_WRITE_REQ :
		case BT_ATT_OP_EXEC_WRITE_RSP :
		case BT_ATT_OP_HANDLE_VAL_CONF :
		default:{//not support
			send_att_error(addr, gatt_services, opcode, 0x0001,
					BT_ATT_ERROR_REQUEST_NOT_SUPPORTED);
			Log.e("ATT opcode not support : %d", opcode);
			}break;
	}
	return 0;
}

static struct gatt_services* create_services(void)
{
	struct gatt_services *gatt_services;
	gatt_services = mem_malloc(sizeof(struct gatt_services));
	if(!gatt_services){
		goto malloc_gatt_failed;
	}
	gatt_services->service_list = queue_new();
	if(!gatt_services->service_list){
		goto malloc_service_list_failed;
	}
	return gatt_services;
malloc_service_list_failed :
	mem_free(gatt_services);
malloc_gatt_failed :
	return NULL;
}
struct gatt_service* create_service(bt_uuid_t *uuid)//init uuid & character_list
{
	struct gatt_service *gatt_service;
	gatt_service = mem_malloc(sizeof(struct gatt_service));
	if(!gatt_service){
		return NULL;
	}
	gatt_service->character_list = queue_new();
	if(!gatt_service->character_list){
		goto malloc_character_list_failed;
	}
	gatt_service->uuid = uuid;
	gatt_service->io_if.sendNotification = gatts_send_notification;
	return gatt_service;
malloc_character_list_failed :
	mem_free(gatt_service);
	return NULL;
}
struct gatt_character* create_character(bt_uuid_t *uuid, UINT8 prop)
{
	struct gatt_character *gatt_character;
	gatt_character = mem_malloc(sizeof(struct gatt_character));
	if(!gatt_character){
		return NULL;
	}
	gatt_character->uuid = uuid;
	gatt_character->prop = prop;
	return gatt_character;
}
int service_add_character(struct gatt_service *service, struct gatt_character *character)
{
	if((!service) || (!service->character_list) || (!character)){
		return GATT_FAILED_NOEXIST;
	}
	queue_push_tail(service->character_list, character);
	return GATT_SUCCESS;
}

void init_service_handle_value_cb(void *data, void *user_data)
{
	struct gatt_character *gatt_character = data;
	struct gatt_service *gatt_service = user_data;
	if(gatt_character->prop & BT_GATT_CHRC_PROP_NOTIFY ||
		gatt_character->prop & BT_GATT_CHRC_PROP_INDICATE ){
		gatt_character->type_handle = gatt_service->handle_end + 1;
		gatt_character->value_handle = gatt_service->handle_end + 2;
		gatt_character->desc_handle = gatt_service->handle_end + 3;
		gatt_service->handle_end += 3;
	}else{
		gatt_character->type_handle = gatt_service->handle_end + 1;
		gatt_character->value_handle = gatt_service->handle_end + 2;
		gatt_character->desc_handle = 0;
		gatt_service->handle_end += 2;
	}
}
static int init_service_handle_value(struct gatt_services* gatt_services,
		struct gatt_service *gatt_service)
{
	UINT16 last_handle_value;
	struct gatt_service *last_gatt_service;
	last_gatt_service = queue_peek_tail(gatt_services->service_list);
	if(!last_gatt_service){
		last_handle_value = 0;
	}else{
		last_handle_value = last_gatt_service->handle_end;
	}
	if(last_handle_value >= 0xFFFF){
		return GATT_FAILED_NOMEM;
	}
	gatt_service->handle_start = last_handle_value + 1;//one handle for service.
	gatt_service->handle_end = gatt_service->handle_start;
	queue_foreach(gatt_service->character_list, init_service_handle_value_cb, gatt_service);
	return GATT_SUCCESS;
}
const struct gatts_if* register_gatt_service(
		struct gatt_service* gatt_service, struct gatts_cb *io_cb, void *pdata)
{
	bt_mutex_t *mutex;
	if(!gatt_services){
		Log.e("gatt server not inited");
		return NULL;
	}
	if((!gatt_service) || (!io_cb)){
		Log.e("regist gatt service failed");
		return NULL;
	}
	mutex = &gatt_services->gatt_mutex;
	bt_mutex_lock(mutex);
	init_service_handle_value(gatt_services, gatt_service);
	gatt_service->io_cb = io_cb;
	gatt_service->pdata = pdata;
	queue_push_tail(gatt_services->service_list, gatt_service);
	bt_mutex_unlock(mutex);
	return &gatt_service->io_if;
}
const struct gatt_client* register_gatt_client(struct gattc_cb *io_cb, void *pdata)
{
	return NULL;
}

struct addr_mtu {
	bdaddr_t *addr;
	UINT16 mtu;
};
static void mtu_change_cb(void *data, void *user_data)
{
	struct gatt_service *gatt_service = data;
	struct addr_mtu *addr_mtu = user_data;
	gatt_service->io_cb->onMtuChanged(
			addr_mtu->addr,
			addr_mtu->mtu,
			gatt_service->pdata);
}
static void mtu_rsp(bdaddr_t *addr, UINT16 mtu, void *pdata)
{
	struct gatt_services *gatt_services = pdata;
	struct addr_mtu addr_mtu;
	UINT8 rsp[8], *p = rsp;
	UINT16 rsp_len;
	UINT16_TO_STREAM(p, mtu);
	rsp_len = p - rsp;
	gatt_send(addr, gatt_services, BT_ATT_OP_MTU_RSP, rsp, rsp_len);

	addr_mtu.addr = addr;
	addr_mtu.mtu = mtu;
	queue_foreach(gatt_services->service_list, mtu_change_cb, &addr_mtu);
}

struct handle_range {
	UINT16 start_handle;
	UINT16 end_handle;
};
static bool find_gatt_descreptor_by_handle_range_cb(
		const void *data, const void *match_data)
{
	const struct gatt_character *gatt_character = data;
	const struct handle_range *handle_range = match_data;
	if(gatt_character->desc_handle >= handle_range->start_handle &&
			gatt_character->desc_handle <= handle_range->end_handle){
		return true;
	}else{
		return false;
	}
}
static bool find_desc_in_service_by_handle_range_cb(
		const void *data, const void *match_data)
{
	const struct gatt_service *gatt_service = data;
	const struct handle_range *handle_range = match_data;
	if(gatt_service->handle_start <= handle_range->end_handle &&
			gatt_service->handle_end >= handle_range->start_handle){
		struct gatt_character *gatt_character;
		gatt_character = queue_find(gatt_service->character_list,
				find_gatt_descreptor_by_handle_range_cb, handle_range);
		if(gatt_character){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}
static void find_info_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, void *pdata)
{
	struct gatt_service *gatt_service;
	struct gatt_character *gatt_character;
	UINT8 rsp[32], *p = rsp;
	UINT16 rsp_len;

		struct handle_range handle_range;
		handle_range.start_handle = start_handle;
		handle_range.end_handle = end_handle;
		gatt_service = queue_find(gatt_services->service_list,
				find_desc_in_service_by_handle_range_cb, &handle_range);
		if(!gatt_service){
			goto info_not_found_failed;
		}
		gatt_character = queue_find(gatt_service->character_list,
				find_gatt_descreptor_by_handle_range_cb, &handle_range);
		if(!gatt_character){
			goto info_not_found_failed;
		}
		UINT8_TO_STREAM(p, FIND_INFO_UUID_TYPE_16_BIT);//all descreptor type UUID is 16 bit
		UINT16_TO_STREAM(p, gatt_character->desc_handle);
		//we only support character configure descreptor
		UINT16_TO_STREAM(p, GATT_CLIENT_CHARAC_CFG_UUID);
		rsp_len = p - rsp;
		gatt_send(addr, gatt_services, BT_ATT_OP_FIND_INFO_RSP, rsp, rsp_len);
		return;
info_not_found_failed :
	send_att_error(addr, gatt_services, BT_ATT_OP_FIND_INFO_REQ,
			start_handle, BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND);
	return;
}

static bool find_gatt_character_by_handle_range_cb(
		const void *data, const void *match_data)
{
	const struct gatt_character *gatt_character = data;
	const struct handle_range *handle_range = match_data;
	if(gatt_character->type_handle >= handle_range->start_handle &&
			gatt_character->type_handle <= handle_range->end_handle){
		return true;
	}else{
		return false;
	}
}
static bool find_gatt_service_by_handle_range_cb(
		const void *data, const void *match_data)
{
	const struct gatt_service *gatt_service = data;
	const struct handle_range *handle_range = match_data;
	if(gatt_service->handle_start >= handle_range->start_handle &&
			gatt_service->handle_end <= handle_range->end_handle){
		return true;
	}else{
		return false;
	}
}
static bool find_chac_in_service_by_handle_range_cb(
		const void *data, const void *match_data)
{
	const struct gatt_service *gatt_service = data;
	const struct handle_range *handle_range = match_data;
	if(gatt_service->handle_start <= handle_range->end_handle &&
			gatt_service->handle_end >= handle_range->start_handle){
		struct gatt_character *gatt_character;
		gatt_character = queue_find(gatt_service->character_list,
				find_gatt_character_by_handle_range_cb, handle_range);
		if(gatt_character){
			return true;
		}else{
			return false;
		}
	}else{
		return false;
	}
}
static void read_by_type_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata)
{
	struct gatt_service *gatt_service;
	struct gatt_character *gatt_character;
	UINT8 rsp[32], *p = rsp;
	UINT16 rsp_len;
	if(!bt_uuid_cmp_str(uuid, GATT_CHARAC_UUID_STR)){
		struct handle_range handle_range;
		handle_range.start_handle = start_handle;
		handle_range.end_handle = end_handle;
		gatt_service = queue_find(gatt_services->service_list,
				find_chac_in_service_by_handle_range_cb, &handle_range);
		if(!gatt_service){
			goto type_not_found_failed;
		}
		gatt_character = queue_find(gatt_service->character_list,
				find_gatt_character_by_handle_range_cb, &handle_range);
		if(!gatt_character){
			goto type_not_found_failed;
		}

		UINT8_TO_STREAM(p, 2+1+2+gatt_character->uuid->type/8);
		UINT16_TO_STREAM(p, gatt_character->type_handle);
		UINT8_TO_STREAM(p, gatt_character->prop);
		UINT16_TO_STREAM(p, gatt_character->value_handle);
		if(gatt_character->uuid->type == BT_UUID16){
			UINT16_TO_STREAM(p, gatt_character->uuid->value.u16);
		}else if(gatt_service->uuid->type == BT_UUID128){
			ARRAY_TO_STREAM(p, gatt_character->uuid->value.u128.data, 16);
		}
		rsp_len = p - rsp;
		gatt_send(addr, gatt_services, BT_ATT_OP_READ_BY_TYPE_RSP, rsp, rsp_len);
		return;
	}else{//not support other uuid
		goto type_not_found_failed;
	}
type_not_found_failed :
	send_att_error(addr, gatt_services, BT_ATT_OP_READ_BY_TYPE_REQ,
			start_handle, BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND);
	return;
}

static void read_by_grp_type_rsp(bdaddr_t *addr,
		UINT16 start_handle, UINT16 end_handle, bt_uuid_t *uuid, void *pdata)
{
	struct gatt_services *gatt_services = pdata;
	struct gatt_service *gatt_service;
	char str[80];
	UINT8 rsp[1024], *p = rsp;
	UINT16 rsp_len;
	bt_uuid_to_string(uuid, str, 80);
	Log.v("%s@%d, start=%d end=%d uuid=%s", __func__, __LINE__,
			start_handle, end_handle, str);
	if(!bt_uuid_cmp_str(uuid, GATT_PRIM_SVC_UUID_STR)){
		struct handle_range handle_range;
		handle_range.start_handle = start_handle;
		handle_range.end_handle = end_handle;
		gatt_service = queue_find(gatt_services->service_list,
				find_gatt_service_by_handle_range_cb, &handle_range);
		if(gatt_service){
			UINT8_TO_STREAM(p, gatt_service->uuid->type/8 + 4);
			UINT16_TO_STREAM(p, gatt_service->handle_start);
			UINT16_TO_STREAM(p, gatt_service->handle_end);
			if(gatt_service->uuid->type == BT_UUID16){
				UINT16_TO_STREAM(p, gatt_service->uuid->value.u16);
			}else if(gatt_service->uuid->type == BT_UUID128){
				ARRAY_TO_STREAM(p, gatt_service->uuid->value.u128.data, 16);
			}
			rsp_len = p - rsp;
			gatt_send(addr, gatt_services,
					BT_ATT_OP_READ_BY_GRP_TYPE_RSP, rsp, rsp_len);
			return;
		}else{
			goto read_by_grp_type_rsp_not_found;
		}
	}else{
		goto read_by_grp_type_rsp_not_found;
	}
read_by_grp_type_rsp_not_found:
	send_att_error(addr, gatt_services, BT_ATT_OP_READ_BY_GRP_TYPE_REQ,
			start_handle, BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND);
}

static bool find_chac_by_uuid_cb(const void *data, const void *match_data)
{
	const struct gatt_character *gatt_character = data;
	const bt_uuid_t *chac_uuid = match_data;
	if(!bt_uuid_cmp(gatt_character->uuid, chac_uuid)){
		return true;
	}else{
		return false;
	}
}
static bool find_chac_is_in_service_by_uuid_cb(
		const void *data, const void *match_data)
{
	const struct gatt_service *gatt_service = data;
	const bt_uuid_t *chac_uuid = match_data;
	struct gatt_character *gatt_character;
	gatt_character = queue_find(gatt_service->character_list,
			find_chac_by_uuid_cb, chac_uuid);
	if(gatt_character){
		return true;
	}else{
		return false;
	}
}
static struct gatt_character* find_chac_by_uuid(
		struct gatt_services *gatt_services, bt_uuid_t *chac_uuid)
{
	struct gatt_service *gatt_service;
	gatt_service = queue_find(gatt_services->service_list,
			find_chac_is_in_service_by_uuid_cb, chac_uuid);
	if(!gatt_service){
		return NULL;
	}
	return queue_find(gatt_service->character_list,
			find_chac_by_uuid_cb, chac_uuid);
}
static int gatts_send_notification(bdaddr_t *addr, bt_uuid_t *chac_uuid,
		UINT8 *buf, UINT16 len)
{
	UINT8 rsp[1024], *p = rsp;
	UINT16 rsp_len;
	struct gatt_character *gatt_character;
	gatt_character = find_chac_by_uuid(gatt_services, chac_uuid);
	if(!gatt_character){
		return GATT_FAILED_NOEXIST;
	}
	UINT16_TO_STREAM(p, gatt_character->value_handle);
	ARRAY_TO_STREAM(p, buf, len);
	rsp_len = p - rsp;
	gatt_send(addr, gatt_services, BT_ATT_OP_HANDLE_VAL_NOT, rsp, rsp_len);
	return GATT_SUCCESS;
}

/*
 * att.c
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

#if 0
oooooooooooooooooo UNFINISHED FUNCTION oooooooooooooooooooo

static att_status_t add_att_pending(struct att_pending *att_pending,
		struct att_send_op *att_send_op)
static void remove_att_pending(struct att_pending *att_pending, UINT8 opcode)
#endif

#include <stdio.h>
#include <pthread.h>
#include "att.h"
#include "queue.h"
#include "bt_util.h"
#include "bt_semaphore.h"
#include "log.h"

struct att_pending {
	struct queue *req_queue; /* for saving struct att_send_op */
	bt_sem_t sem_att;
};

static int att_connect(bdaddr_t *addr);
static int att_disconnect(bdaddr_t *addr);
int att_send(bdaddr_t *addr, struct att_send_op *att_send_op);

static void *user_data;
static struct att_cb att_cb;
static struct att att = {
	att_connect,
	att_disconnect,
	att_send,
};

static const struct att_io *att_io;
struct att_pending* att_pending;
void att_io_conn_change_cb(bdaddr_t addr, int status, void *pdata);
void att_io_receive(bdaddr_t addr, UINT8 opcode, UINT8 *pdu, UINT32 len, void *pdata);
static struct att_io_cb att_io_cb = {
	att_io_conn_change_cb,
	att_io_receive,
};

static struct att_pending* new_att_pending(void);
static att_status_t add_att_pending(struct att_pending *att_pending,
		struct att_send_op *att_send_op);
static void remove_att_pending(struct att_pending *att_pending, UINT8 opcode);

enum att_op_type {
	ATT_OP_TYPE_REQ,
	ATT_OP_TYPE_RSP,
	ATT_OP_TYPE_CMD,
	ATT_OP_TYPE_IND,
	ATT_OP_TYPE_NOT,
	ATT_OP_TYPE_CONF,
	ATT_OP_TYPE_UNKNOWN,
};

static const struct {
	UINT8 opcode;
	enum att_op_type type;
} att_opcode_type_table[] = {
	{ BT_ATT_OP_ERROR_RSP,             ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_MTU_REQ,               ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_MTU_RSP,               ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_FIND_INFO_REQ,         ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_FIND_INFO_RSP,         ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_FIND_BY_TYPE_VAL_REQ,  ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_FIND_BY_TYPE_VAL_RSP,  ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BY_TYPE_REQ,      ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BY_TYPE_RSP,      ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_REQ,              ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_RSP,              ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BLOB_REQ,         ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BLOB_RSP,         ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_MULT_REQ,         ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_MULT_RSP,         ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_REQ,  ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_RSP,  ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_REQ,             ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_WRITE_RSP,             ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_CMD,             ATT_OP_TYPE_CMD },
	{ BT_ATT_OP_SIGNED_WRITE_CMD,      ATT_OP_TYPE_CMD },
	{ BT_ATT_OP_PREP_WRITE_REQ,        ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_PREP_WRITE_RSP,        ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_EXEC_WRITE_REQ,        ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_EXEC_WRITE_RSP,        ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_HANDLE_VAL_NOT,        ATT_OP_TYPE_NOT },
	{ BT_ATT_OP_HANDLE_VAL_IND,        ATT_OP_TYPE_IND },
	{ BT_ATT_OP_HANDLE_VAL_CONF,       ATT_OP_TYPE_CONF },
	{ }
};

static enum att_op_type get_op_type(UINT8 opcode)
{
	int i;

	for (i = 0; att_opcode_type_table[i].opcode; i++) {
		if (att_opcode_type_table[i].opcode == opcode)
			return att_opcode_type_table[i].type;
	}

	return ATT_OP_TYPE_UNKNOWN;
}

static const struct {
	UINT8 req_opcode;
	UINT8 rsp_opcode;
} att_req_rsp_mapping_table[] = {
	{ BT_ATT_OP_MTU_REQ,              BT_ATT_OP_MTU_RSP },
	{ BT_ATT_OP_FIND_INFO_REQ,        BT_ATT_OP_FIND_INFO_RSP },
	{ BT_ATT_OP_FIND_BY_TYPE_VAL_REQ, BT_ATT_OP_FIND_BY_TYPE_VAL_RSP },
	{ BT_ATT_OP_READ_BY_TYPE_REQ,     BT_ATT_OP_READ_BY_TYPE_RSP },
	{ BT_ATT_OP_READ_REQ,             BT_ATT_OP_READ_RSP },
	{ BT_ATT_OP_READ_BLOB_REQ,        BT_ATT_OP_READ_BLOB_RSP },
	{ BT_ATT_OP_READ_MULT_REQ,        BT_ATT_OP_READ_MULT_RSP },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_REQ, BT_ATT_OP_READ_BY_GRP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_REQ,            BT_ATT_OP_WRITE_RSP },
	{ BT_ATT_OP_PREP_WRITE_REQ,       BT_ATT_OP_PREP_WRITE_RSP },
	{ BT_ATT_OP_EXEC_WRITE_REQ,       BT_ATT_OP_EXEC_WRITE_RSP },
	{ }
};

UINT8 get_req_opcode(UINT8 rsp_opcode)
{
	int i;

	for (i = 0; att_req_rsp_mapping_table[i].rsp_opcode; i++) {
		if (att_req_rsp_mapping_table[i].rsp_opcode == rsp_opcode)
			return att_req_rsp_mapping_table[i].req_opcode;
	}

	return 0;
}
void att_io_conn_change_cb(bdaddr_t addr, int status, void *pdata)
{
	int att_status;
	att_status = status;
	att_cb.conn_change_cb(addr, att_status, pdata);
}

void att_io_receive(bdaddr_t addr, UINT8 opcode, UINT8 *pdu, UINT32 len, void *pdata)
{
	enum att_op_type type;
	type = get_op_type(opcode);
	switch(type){
		case ATT_OP_TYPE_REQ  :
		case ATT_OP_TYPE_NOT  :
		case ATT_OP_TYPE_IND  :
		case ATT_OP_TYPE_CONF :
			att_cb.onReceive(&addr, opcode, pdu, len, pdata);
			break;
		case ATT_OP_TYPE_RSP  :
			remove_att_pending(att_pending, opcode);
			att_cb.onReceive(&addr, opcode, pdu, len, pdata);
			break;
		default :
			break;
	}
}

const struct att* register_att(int hdev, struct att_cb *io_cb, void *pdata)
{
	if(io_cb == NULL ||
			io_cb->conn_change_cb == NULL ||
			io_cb->onReceive== NULL){
		Log.e("att regist callbacks failed");
		return NULL;
	}
	if(att_cb.conn_change_cb != NULL && att_cb.onReceive!= NULL){
		Log.e("att already registed");
		return NULL;
	}
	att_cb = *io_cb;
	user_data = pdata;
	att_pending = new_att_pending();
	if(!att_pending){
		Log.e("malloc att_pending failed");
		return NULL;
	}
	att_io = register_att_io(hdev, &att_io_cb, pdata);
	return &att;
}

int att_connect(bdaddr_t *addr)
{
	int res;
	res = att_io->connect(addr);
	return res;
}
int att_disconnect(bdaddr_t *addr)
{
	int res;
	res = att_io->disconnect(addr);
	return res;
}
int att_send(bdaddr_t *addr, struct att_send_op *att_send_op)
{
	int res;
	enum att_op_type type;
	type = get_op_type(att_send_op->opcode);
	switch(type){
		case ATT_OP_TYPE_REQ  :
		case ATT_OP_TYPE_IND  :
		case ATT_OP_TYPE_CONF :
			add_att_pending(att_pending, att_send_op);
			res = att_io->send(addr, att_send_op->opcode, att_send_op->pdu, att_send_op->len);
			if(res != ATT_IO_SUCCESS){
				remove_att_pending(att_pending, att_send_op->opcode);
			}
			break;
		case ATT_OP_TYPE_RSP  :
		case ATT_OP_TYPE_NOT  :
			res = att_io->send(addr, att_send_op->opcode, att_send_op->pdu, att_send_op->len);
			break;
		default :
			res = -1;
			break;
	}
	return res;
}

struct att_pending* new_att_pending(void)
{
	struct att_pending *att_pending;
	att_pending = mem_malloc(sizeof(struct att_pending));
	if(!att_pending){
		goto malloc_att_pending_failed;
	}
	att_pending->req_queue = queue_new();
	if(!att_pending->req_queue){
		goto malloc_req_queue_failed;
	}
	if(bt_sem_init(&att_pending->sem_att, 1) < 0){
		goto init_sem_att_failed;
	}
	return att_pending;
init_sem_att_failed :
	mem_free(att_pending->req_queue);
malloc_req_queue_failed :
	mem_free(att_pending);
malloc_att_pending_failed :
	return NULL;
}

#if 0 /* not use */
static void destroy_att_pending(struct att_pending *att_pending,
		queue_destroy_func_t destroy_req_queue_cb)
{
	if(!att_pending){
		return;
	}
	queue_destroy(att_pending->req_queue, destroy_req_queue_cb);
	bt_sem_destroy(&att_pending->sem_att);
	return;
}
#endif
static att_status_t add_att_pending(struct att_pending *att_pending,
		struct att_send_op *att_send_op)
{
	//need add timeout event
	return ATT_SUCCESS;
}
static void remove_att_pending(struct att_pending *att_pending, UINT8 opcode)
{
	return;
}

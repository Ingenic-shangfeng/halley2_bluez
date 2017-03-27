/*
 * att.h
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

#ifndef __ATT_H__
#define __ATT_H__

#include "att_io.h"

#define ATT_REQUEST_TOUT_S              3
/* ATT protocol opcodes */
#define BT_ATT_OP_ERROR_RSP             0x01
#define BT_ATT_OP_MTU_REQ               0x02
#define BT_ATT_OP_MTU_RSP               0x03
#define BT_ATT_OP_FIND_INFO_REQ         0x04
#define BT_ATT_OP_FIND_INFO_RSP         0x05
#define BT_ATT_OP_FIND_BY_TYPE_VAL_REQ  0x06
#define BT_ATT_OP_FIND_BY_TYPE_VAL_RSP  0x07
#define BT_ATT_OP_READ_BY_TYPE_REQ      0x08
#define BT_ATT_OP_READ_BY_TYPE_RSP      0x09
#define BT_ATT_OP_READ_REQ              0x0a
#define BT_ATT_OP_READ_RSP              0x0b
#define BT_ATT_OP_READ_BLOB_REQ         0x0c
#define BT_ATT_OP_READ_BLOB_RSP         0x0d
#define BT_ATT_OP_READ_MULT_REQ         0x0e
#define BT_ATT_OP_READ_MULT_RSP         0x0f
#define BT_ATT_OP_READ_BY_GRP_TYPE_REQ  0x10
#define BT_ATT_OP_READ_BY_GRP_TYPE_RSP  0x11
#define BT_ATT_OP_WRITE_REQ             0x12
#define BT_ATT_OP_WRITE_RSP             0x13
#define BT_ATT_OP_WRITE_CMD             0x52
#define BT_ATT_OP_SIGNED_WRITE_CMD      0xD2
#define BT_ATT_OP_PREP_WRITE_REQ        0x16
#define BT_ATT_OP_PREP_WRITE_RSP        0x17
#define BT_ATT_OP_EXEC_WRITE_REQ        0x18
#define BT_ATT_OP_EXEC_WRITE_RSP        0x19
#define BT_ATT_OP_HANDLE_VAL_NOT        0x1B
#define BT_ATT_OP_HANDLE_VAL_IND        0x1D
#define BT_ATT_OP_HANDLE_VAL_CONF       0x1E

/* Error codes for Error response PDU */
#define BT_ATT_ERROR_INVALID_HANDLE                     0x01
#define BT_ATT_ERROR_READ_NOT_PERMITTED	                0x02
#define BT_ATT_ERROR_WRITE_NOT_PERMITTED                0x03
#define BT_ATT_ERROR_INVALID_PDU                        0x04
#define BT_ATT_ERROR_AUTHENTICATION                     0x05
#define BT_ATT_ERROR_REQUEST_NOT_SUPPORTED              0x06
#define BT_ATT_ERROR_INVALID_OFFSET                     0x07
#define BT_ATT_ERROR_AUTHORIZATION                      0x08
#define BT_ATT_ERROR_PREPARE_QUEUE_FULL                 0x09
#define BT_ATT_ERROR_ATTRIBUTE_NOT_FOUND                0x0A
#define BT_ATT_ERROR_ATTRIBUTE_NOT_LONG                 0x0B
#define BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE   0x0C
#define BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN        0x0D
#define BT_ATT_ERROR_UNLIKELY                           0x0E
#define BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION            0x0F
#define BT_ATT_ERROR_UNSUPPORTED_GROUP_TYPE             0x10
#define BT_ATT_ERROR_INSUFFICIENT_RESOURCES             0x11

/* GATT Characteristic Properties Bitfield values */
#define BT_GATT_CHRC_PROP_BROADCAST          0x01
#define BT_GATT_CHRC_PROP_READ               0x02
#define BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP 0x04
#define BT_GATT_CHRC_PROP_WRITE              0x08
#define BT_GATT_CHRC_PROP_NOTIFY             0x10
#define BT_GATT_CHRC_PROP_INDICATE           0x20
#define BT_GATT_CHRC_PROP_AUTH               0x40
#define BT_GATT_CHRC_PROP_EXT_PROP           0x80

#define FIND_INFO_UUID_TYPE_16_BIT  0x01
#define FIND_INFO_UUID_TYPE_128_BIT 0x02

typedef enum {
	ATT_SUCCESS         = 0,
	ATT_FAILED_BUSY     = -1,
	ATT_FAILED_NOEXIST  = -2,
	ATT_FAILED_TIMEOUT  = -3,
} att_status_t;

typedef enum {
	HANDLE_ATTR_DATA,
	HANDLE_CHARACTER,
	HANDLE_DESC_CONF,
	HANDLE_TYPE_UNKNOWN,
} handle_type_t;

typedef void (*att_response_func_t)(bdaddr_t addr, UINT8 opcode, const void *pdu,
		UINT16 length, void *pdata, att_status_t status);

struct att_send_op {
	uint16_t opcode;
	void *pdu;
	uint16_t len;
	att_response_func_t resp_cb;
	void *user_data;
};

enum{
	ATT_STATUS_CONNECTED,
	ATT_STATUS_DISCONNECTED,
};
struct att_cb {
	void (*conn_change_cb)(bdaddr_t addr, int status, void *pdata);
	int (*onReceive)(bdaddr_t *addr, UINT8 opcode,
			const void *pdu, UINT16 length, void *pdata);
};

struct att {
	int (*connect)(bdaddr_t *addr);
	int (*disconnect)(bdaddr_t *addr);
	int (*send)(bdaddr_t *addr, struct att_send_op *att_send_op);
};
const struct att* register_att(int hdev, struct att_cb *io_cb, void *pdata);
UINT8 get_req_opcode(UINT8 rsp_opcode);

#endif /* __ATT_H__ */

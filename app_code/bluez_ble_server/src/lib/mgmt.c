/*
 * mgmt.c
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include "mgmt.h"

#define MGMT_OP_SET_BONDABLE      0x0009
#define MGMT_OP_SET_LE            0x000D
#define MGMT_OP_SET_LOCAL_NAME    0x000F
#define MGMT_OP_DISCONNECT        0x0014
#define MGMT_OP_SET_IO_CAPABILITY 0x0018
#define MGMT_HDR_SIZE         6

struct mgmt_hdr {
	uint16_t opcode;
	uint16_t index;
	uint16_t len;
} __attribute__((packed));

struct mgmt_mode {
	uint8_t val;
} __attribute__((packed));

struct mgmt_addr_info {
		bdaddr_t bdaddr;
			uint8_t type;
} __attribute__((packed));

struct mgmt_cp_set_io_capability {
	    uint8_t io_capability;
} __attribute__((packed));

struct mgmt_cp_disconnect {
		struct mgmt_addr_info addr;
} __attribute__((packed));

struct mgmt_cp_set_local_name {
	uint8_t name[MGMT_MAX_NAME_LENGTH];
	uint8_t short_name[MGMT_MAX_SHORT_NAME_LENGTH];
} __attribute__((packed));

int mgmt_set_name(int hdev, char *name, char *short_name)
{
	char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_cp_set_local_name)];
	struct mgmt_hdr *hdr = (void *) buf;
	struct mgmt_cp_set_local_name *cp = (void *) &buf[sizeof(*hdr)];

	int dd,ret;
	struct sockaddr_hci addr;
	ret = 0;
	dd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (dd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_CONTROL;

	if (bind(dd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(dd);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	hdr->opcode = htobs(MGMT_OP_SET_LOCAL_NAME);
	hdr->len = htobs(sizeof(*cp));
	hdr->index = htobs(hdev);

	strncpy((char*)cp->name, name, MGMT_MAX_NAME_LENGTH);
	strncpy((char*)cp->short_name, short_name, MGMT_MAX_SHORT_NAME_LENGTH);

	if (write(dd, buf, sizeof(buf)) < 0){
		printf("%s: failed\n",__FUNCTION__);
	}
	return ret;
}

static int mgmt_set_mode(int hdev, int mgmt_sock, uint16_t opcode, uint8_t val)
{
	char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_mode)];
	struct mgmt_hdr *hdr = (void *) buf;
	struct mgmt_mode *cp = (void *) &buf[sizeof(*hdr)];

	memset(buf, 0, sizeof(buf));
	hdr->opcode = htobs(opcode);
	hdr->index = htobs(hdev);
	hdr->len = htobs(sizeof(*cp));

	cp->val = val;

	if (write(mgmt_sock, buf, sizeof(buf)) < 0)
		return -1;

	return 0;
}

int mgmt_setup(int hdev)
{
	int dd,ret;
	struct sockaddr_hci addr;
	ret = 0;
	dd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (dd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_CONTROL;

	if (bind(dd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(dd);
		return -1;
	}

	ret = mgmt_set_mode(hdev, dd, MGMT_OP_SET_LE, 1);
	ret = mgmt_set_mode(hdev,dd, MGMT_OP_SET_BONDABLE, 1);

	close(dd);
	return ret;
}

int mgmt_set_iocap(int hdev, uint8_t io_cap)
{
	char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_cp_set_io_capability)];
	struct mgmt_hdr *hdr = (void *) buf;
	struct mgmt_cp_set_io_capability *cp = (void *) &buf[sizeof(*hdr)];
	int dd,ret;
	struct sockaddr_hci addr;
	ret = 0;
	dd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (dd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_CONTROL;

	if (bind(dd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(dd);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	hdr->opcode = htobs(MGMT_OP_SET_IO_CAPABILITY);
	hdr->len = htobs(sizeof(*cp));
	hdr->index = htobs(hdev);

	cp->io_capability = io_cap;

	if (write(dd, buf, sizeof(buf)) < 0){
		printf("%s: failed\n",__FUNCTION__);
	}
	return ret;
}
int mgmt_disconnect(int hdev, bdaddr_t *bdaddr, uint8_t bdaddr_type)
{
	char buf[MGMT_HDR_SIZE + sizeof(struct mgmt_cp_disconnect)];
	struct mgmt_hdr *hdr = (void *) buf;
	struct mgmt_cp_disconnect *cp = (void *) &buf[sizeof(*hdr)];
	int dd,ret;
	struct sockaddr_hci addr;
	ret = 0;
	dd = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (dd < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_CONTROL;

	if (bind(dd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(dd);
		return -1;
	}
	memset(buf, 0, sizeof(buf));
	hdr->opcode = htobs(MGMT_OP_DISCONNECT);
	hdr->len = htobs(sizeof(*cp));
	hdr->index = htobs(hdev);

	bacpy(&cp->addr.bdaddr, bdaddr);
	cp->addr.type = bdaddr_type;

	if (write(dd, buf, sizeof(buf)) < 0){
		printf("%s: failed\n",__FUNCTION__);
	}
	return ret;
}

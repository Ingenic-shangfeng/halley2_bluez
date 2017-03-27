/*
 * bt_util.h
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

#ifndef __BT_UTIL_H__
#define __BT_UTIL_H__
#include "types.h"
#include "bluetooth.h"
#include "mgmt.h"
#include "hci.h"

int l2cap_le_att_listen_and_accept(int hdev, int security,
		void (*connected_cb)(int fd, bdaddr_t addr));
int l2cap_le_att_connect(int hdev, bdaddr_t *dst, int security,
		void (*connected_cb)(int fd, bdaddr_t addr));
int le_set_random_address(int hdev);
int le_set_advertise_enable(int hdev);
int le_set_advertise_parameters(int hdev);
int le_set_advertise_data(int hdev, char *data);
void le_disconnect(int hdev, bdaddr_t *addr);
void dump_btaddr(char *prefix, bdaddr_t *addr);

#endif /* __BT_UTIL_H__ */

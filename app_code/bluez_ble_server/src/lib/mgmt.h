/*
 * mgmt.h
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

#ifndef __MGMT_H__
#define __MGMT_H__

#include "bluetooth.h"
#include "hci.h"

/* IO Capabilities values */
#define SMP_IO_DISPLAY_ONLY 0x00
#define SMP_IO_DISPLAY_YESNO    0x01
#define SMP_IO_KEYBOARD_ONLY    0x02
#define SMP_IO_NO_INPUT_OUTPUT  0x03
#define SMP_IO_KEYBOARD_DISPLAY 0x04

#define MGMT_MAX_NAME_LENGTH        (248 + 1)
#define MGMT_MAX_SHORT_NAME_LENGTH  (10 + 1)

int mgmt_setup(int hdev);
int mgmt_set_iocap(int hdev, uint8_t io_cap);
int mgmt_disconnect(int hdev, bdaddr_t *bdaddr, uint8_t bdaddr_type);
int mgmt_set_name(int hdev, char *name, char *short_name);
#endif /* __MGMT_H__ */

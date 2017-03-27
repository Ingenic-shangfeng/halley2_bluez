/*
 * main.c
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
#include "gatt.h"
#include "mgmt.h"
#include "log.h"
#include "mem_manage.h"
#define HCI_DEV_ID 0

void init_gatt_services(void);
int main(int argc, char *argv[])
{
	mgmt_setup(HCI_DEV_ID);
	mgmt_set_name(HCI_DEV_ID,"ble_server", "ble");
	mgmt_set_iocap(HCI_DEV_ID, SMP_IO_NO_INPUT_OUTPUT);
	le_set_random_address(HCI_DEV_ID);
	le_set_advertise_parameters(HCI_DEV_ID);
	init_gatt(HCI_DEV_ID);
	init_gatt_services();
	le_set_advertise_enable(HCI_DEV_ID);
	while(1){
		usleep(1000000);
	}
	return 0;
}

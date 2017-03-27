/*
 * bt_util.c
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
#include <unistd.h>
#include "l2cap.h"
#include "bt_util.h"
#include "bluetooth.h"
#include "hci.h"
#include "log.h"

#define ATT_CID 4

int l2cap_le_att_listen_and_accept(int hdev, int security,
		void (*connected_cb)(int fd, bdaddr_t addr))
{
	int sk;
	struct sockaddr_l2 srcaddr;
	socklen_t optlen;
	struct bt_security btsec;
	//bdaddr_t rand_addr;
	bdaddr_t src_addr;

	if(!connected_cb){
		return -1;
	}
	//hci_le_get_random_address(&rand_addr);
	hci_devba(hdev, &src_addr);
	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		Log.e("Failed to create L2CAP socket");
		return -1;
	}

	/* Set up source address */
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(ATT_CID);
	srcaddr.l2_bdaddr_type = BDADDR_LE_RANDOM;
	bacpy(&srcaddr.l2_bdaddr, &src_addr);

	/* Set the security level */
	memset(&btsec, 0, sizeof(btsec));
	btsec.level = security;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec,
				sizeof(btsec)) != 0) {
		Log.e("Failed to set L2CAP security level");
		goto fail;
	}

	if (bind(sk, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
		Log.e("Failed to bind L2CAP socket");
		goto fail;
	}

	if (listen(sk, 10) < 0) {
		Log.e("Listening on socket failed");
		goto fail;
	}
	Log.v("Started listening on ATT channel. Waiting for connections");

	while(1){
		struct sockaddr_l2 addr;
		char ba[18];
		int nsk;
		memset(&addr, 0, sizeof(addr));
		optlen = sizeof(addr);
		nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
		if (nsk < 0) {
			Log.e("Accept failed");
			goto fail;
		}
		ba2str(&addr.l2_bdaddr, ba);
		Log.v("Connect from %s", ba);
		connected_cb(nsk, addr.l2_bdaddr);
	}
fail:
	close(sk);
	return -1;
}

int l2cap_le_att_connect(int hdev, bdaddr_t *dst, int security,
		void (*connected_cb)(int fd, bdaddr_t addr))
{
	int sock;
	struct sockaddr_l2 srcaddr, dstaddr;
	bdaddr_t src;
	struct bt_security btsec;

	hci_devba(hdev, &src);
	char srcaddr_str[18], dstaddr_str[18];
	ba2str(&src, srcaddr_str);
	ba2str(dst, dstaddr_str);
	Log.v("LE connection on ATT\n\tsrc: %s\n\tdest: %s",
			srcaddr_str, dstaddr_str);

	sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock < 0) {
		Log.e("Failed to create L2CAP socket");
		return -1;
	}

	/* Set up source address */
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(ATT_CID);
	srcaddr.l2_bdaddr_type = 0;
	bacpy(&srcaddr.l2_bdaddr, &src);

	if (bind(sock, (struct sockaddr *)&srcaddr, sizeof(srcaddr)) < 0) {
		Log.e("Failed to bind L2CAP socket");
		close(sock);
		return -1;
	}

	/* Set the security level */
	memset(&btsec, 0, sizeof(btsec));
	btsec.level = security;
	if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &btsec,
							sizeof(btsec)) != 0) {
		Log.e("Failed to set L2CAP security level");
		close(sock);
		return -1;
	}

	/* Set up destination address */
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.l2_family = AF_BLUETOOTH;
	dstaddr.l2_cid = htobs(ATT_CID);
	dstaddr.l2_bdaddr_type = BDADDR_LE_RANDOM;
	bacpy(&dstaddr.l2_bdaddr, dst);

	if (connect(sock, (struct sockaddr *) &dstaddr, sizeof(dstaddr)) < 0) {
		Log.e(" Failed to connect");
		close(sock);
		return -1;
	}
	if(connected_cb){
		connected_cb(sock, dstaddr.l2_bdaddr);
	}
	Log.v("Connected !\n");
	return sock;
}

int le_set_random_address(int hdev)
{
	int timeout = 1000;
	int dd;
	bdaddr_t src_addr;
	dd = hci_open_dev(hdev);
	if (dd < 0) {
		Log.e("Could not open device");
		return -1;
	}
	hci_devba(hdev, &src_addr);
	//translate public address to random address
	src_addr.b[5] = ~src_addr.b[5];
	src_addr.b[5] &= 0x7F;
	src_addr.b[5] |= 0x40;
	hci_le_set_random_address(dd, &src_addr, timeout);
	hci_close_dev(dd);
	return 0;
}

int le_set_advertise_parameters(int hdev)
{
	int timeout = 1000;
	int dd;
	bdaddr_t direct_bdaddr = {{0}};
	dd = hci_open_dev(hdev);
	if (dd < 0) {
		Log.e("Could not open device");
		return -1;
	}
	hci_le_set_advertise_parameters(dd,
			0x0020, 0x0040,
			0,      LE_RANDOM_ADDRESS,
			0,      direct_bdaddr,
			7,      0, timeout);
	hci_close_dev(dd);
	return 0;
}

int le_set_advertise_data(int hdev, char *data)
{
	int timeout = 1000;
	int dd;
	dd = hci_open_dev(hdev);
	if (dd < 0) {
		Log.e("Could not open device");
		return -1;
	}
	hci_le_set_advertise_data(dd, data, timeout);
	hci_close_dev(dd);
	return 0;
}

int le_set_advertise_enable(int hdev)
{
	int timeout = 1000;
	int dd;
	dd = hci_open_dev(hdev);
	if (dd < 0) {
		Log.e("Could not open device");
		return -1;
	}
	hci_le_set_advertise_enable(dd, 1, timeout);
	hci_close_dev(dd);
	return 0;
}
void dump_btaddr(char *prefix, bdaddr_t *addr)
{
	char ba[18];
	ba2str(addr, ba);
	Log.v("%s %s", prefix, ba);
}
void le_disconnect(int hdev, bdaddr_t *addr)
{
	mgmt_disconnect(hdev, addr, BDADDR_LE_RANDOM);
}

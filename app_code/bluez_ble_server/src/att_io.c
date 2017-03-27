/*
 * att_io.c
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
#include <pthread.h>
#include "att_io.h"
#include "bt_util.h"
#include "log.h"

static int att_io_connect(bdaddr_t *addr);
static int att_io_disconnect(bdaddr_t *addr);
static int att_io_send(bdaddr_t *addr, UINT8 opcode, UINT8 *pdu, UINT32 len);
static void *listen_thd(void *argv);
static void *receive_thd(void *argv);

static void *user_data;
static int hci_device;
static struct att_io_cb att_io_cb;
static struct att_io att_io = {
	att_io_connect,
	att_io_disconnect,
	att_io_send,
	timeout_add,
	timeout_remove
};
#define IO_DATA_NUM 32
struct io_data {
	struct io *io;
	int fd;
	bdaddr_t addr;
	void *pdata;
}io_data[IO_DATA_NUM];

const struct att_io* register_att_io(int hdev, struct att_io_cb *io_cb, void *pdata)
{
	int ret;
	pthread_t pthread_listen;
	pthread_t pthread_receive;
	if(io_cb == NULL || 
			io_cb->conn_change_cb == NULL || 
			io_cb->receive == NULL){
		Log.e("att_io regist callbacks failed");
		return NULL;
	}
	if(att_io_cb.conn_change_cb != NULL && att_io_cb.receive != NULL){
		Log.e("att_io already registed");
		return NULL;
	}
	att_io_cb = *io_cb;
	user_data = pdata;
	hci_device = hdev;

	ret = pthread_create(&pthread_receive, NULL, receive_thd, user_data);
	if(ret){  
		Log.e("create thread \"%s\" failed", "receive_thd");
		return NULL;  
	}  
	ret = pthread_create(&pthread_listen, NULL, listen_thd, user_data);
	if(ret){  
		Log.e("create thread \"%s\" failed", "listen_thd");
		return NULL;  
	}
	usleep(50000);//wait for thread running
	return &att_io;
}

static struct io_data* search_empty_io_data(void)
{
	int i;
	for(i=0;i<IO_DATA_NUM;i++){
		if(io_data[i].io == NULL){
			return &io_data[i];
		}
	}
	return NULL;
}
static struct io_data* search_io_data_by_addr(bdaddr_t *addr)
{
	int i;
	for(i=0;i<IO_DATA_NUM;i++){
		if(io_data[i].io && !bcmp(&io_data[i].addr, addr, sizeof(bdaddr_t))){
			return &io_data[i];
		}
	}
	return NULL;
}
static struct io_data* search_io_data(struct io *io)
{
	int i;
	for(i=0;i<IO_DATA_NUM;i++){
		if(io_data[i].io == io){
			return &io_data[i];
		}
	}
	return NULL;
}
static struct io_data* new_io_data(int fd, bdaddr_t addr)
{
	struct io_data* io_data = search_empty_io_data();
	if(!io_data){
		return NULL;
	}
	io_data->io= io_new(fd);
	if(!io_data->io){
		return NULL;
	}
	io_data->fd = fd;
	io_data->addr = addr;
	return io_data;
}
static void free_io_data(struct io *io)
{
	struct io_data* io_data = search_io_data(io);
	if(io_data){
		io_destroy(io_data->io);
		io_data->io = NULL;
	}
}
static bool io_read_callback(struct io *io, void *pdata)
{
	struct io_data *io_data = pdata;
	int read_num;
	char buf[1024];
	read_num = read(io_data->fd, buf, 1024);
	if(read_num < 0)return false;
	//dump_btaddr("ATT: Read from:", &io_data->addr);
	att_io_cb.receive(io_data->addr, (UINT8)buf[0], (UINT8*)(buf+1), read_num-1, user_data);
	return true;
}
static bool io_destroy_callback(struct io *io, void *pdata)
{
	struct io_data *io_data = pdata;
	dump_btaddr("ATT: Disconnected from:", &io_data->addr);
	att_io_cb.conn_change_cb(io_data->addr, ATT_IO_STATUS_DISCONNECTED, user_data);
	free_io_data(io);
	return true;
}
static void connected_cb(int fd, bdaddr_t addr)
{
	struct io_data *io_data = new_io_data(fd, addr);
	if(!io_data){
		Log.e("memory error! %s@%d", __func__, __LINE__);
		return;
	}
	io_set_read_handler(io_data->io, io_read_callback, io_data, NULL);
	io_set_disconnect_handler(io_data->io, io_destroy_callback, io_data, NULL);
	dump_btaddr("ATT: Connecte :", &io_data->addr);
	att_io_cb.conn_change_cb(io_data->addr, ATT_IO_STATUS_CONNECTED, user_data);
	return;
}
static void *listen_thd(void *argv)  
{
	int security = BT_SECURITY_LOW;
	l2cap_le_att_listen_and_accept(hci_device, security, connected_cb);
	return NULL;
}
static void *receive_thd(void *argv)  
{
	mainloop_init();
	mainloop_run();
	return NULL;
}
static int att_io_connect(bdaddr_t *addr)
{
	struct io_data *io_data = search_io_data_by_addr(addr);
	if(io_data){
		char dstaddr_str[18];
		ba2str(addr, dstaddr_str);
		Log.v("LE device %s already connected", dstaddr_str);
		return ATT_IO_SUCCESS;
	}
	int security = BT_SECURITY_LOW;
	if(l2cap_le_att_connect(hci_device, addr, security, connected_cb)<0){
		return ATT_IO_FAILED_NOEXIST;
	}
	return ATT_IO_SUCCESS;
}
static int att_io_disconnect(bdaddr_t *addr)
{
	struct io_data *io_data = search_io_data_by_addr(addr);
	if(io_data){
		le_disconnect(hci_device, addr);
		return ATT_IO_SUCCESS;
	}else{
		return ATT_IO_FAILED_NOEXIST;
	}
}
static int att_io_send(bdaddr_t *addr, UINT8 opcode, UINT8 *pdu, UINT32 len)
{
	int ret;
	struct iovec iov[2];
	struct io_data *io_data = search_io_data_by_addr(addr);
	if(!io_data){
		return ATT_IO_FAILED_NOEXIST;
	}
	iov[0].iov_base = &opcode;
	iov[0].iov_len = 1;
	iov[1].iov_base = pdu;
	iov[1].iov_len = len;
	ret = io_send(io_data->io, iov, 2);
	if(ret<0){
		return ATT_IO_FAILED_BUSY;
	}
	return ATT_IO_SUCCESS;
}

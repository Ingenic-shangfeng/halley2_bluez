/*
 * pulse_pacmd.c
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

#include <assert.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <getopt.h>
#include <locale.h>
#include <pthread.h>
#include <config.h>
#include <dbus/dbus.h>

#include <pulse/util.h>
#include <pulse/xmalloc.h>

#include <pulsecore/i18n.h>
#include <pulsecore/poll.h>
#include <pulsecore/macro.h>
#include <pulsecore/core-util.h>
#include <pulsecore/log.h>
#include <pulsecore/pid.h>
#include "pulse_pacmd.h"

static pid_t pid;
static int fd = -1 ,fd_type = 0;
static struct sockaddr_un sa;
static char *cli;

static void* init_pacmd_socket(void *p)
{
	int i;
    setlocale(LC_ALL, "");
	while(1){
		if (pa_pid_file_check_running(&pid, "pulseaudio") < 0) {
			pa_log(_("No PulseAudio daemon running, or not running as session daemon."));
			goto reset;
		}

		if ((fd = pa_socket_cloexec(PF_UNIX, SOCK_STREAM, 0)) < 0) {
			pa_log(_("socket(PF_UNIX, SOCK_STREAM, 0): %s"), strerror(errno));
			goto reset;
		}
		pa_zero(sa);
		sa.sun_family = AF_UNIX;
		if (!(cli = pa_runtime_path("cli")))
			goto reset;
		pa_strlcpy(sa.sun_path, cli, sizeof(sa.sun_path));
		pa_xfree(cli);
		for (i = 0; i < 5; i++) {
			int r;
			if ((r = connect(fd, (struct sockaddr*) &sa, sizeof(sa))) < 0 && (errno != ECONNREFUSED && errno != ENOENT)) {
				pa_log(_("connect(): %s"), strerror(errno));
				goto reset;
			}
			if (r >= 0)
				break;
			if (pa_pid_file_kill(SIGUSR2, NULL, "pulseaudio") < 0) {
				pa_log(_("Failed to kill PulseAudio daemon."));
				goto reset;
			}
			pa_msleep(300);
		}
		if (i >= 5) {
			pa_log(_("Daemon not responding."));
			goto reset;
		}
		/* send hello to enable interactive mode (welcome message, prompt) */
		if (pa_write(fd, "hello\n", 6, &fd_type) < 0) {
			pa_log(_("write(): %s"), strerror(errno));
			goto reset;
		}
		printf("pacmd ready.\n");
		while(1){
			char buf[128];
			int cnt = 128;
			cnt = pa_read(fd, buf, cnt, &fd_type);
			if (cnt <= 0) {
				pa_log(_("read(): %s"), strerror(errno));
				goto reset;
			}
		}
reset:
		if (fd >= 0){
			pa_close(fd);
			fd = -1;
		}
		pa_msleep(3000);
	}
	return NULL;
}
void pulse_pacmd_init(DBusConnection *conn)
{
	pthread_t th;
	// add a rule for which messages we want to see
	dbus_bus_add_match(conn,
			"type='signal',"
			"interface='org.freedesktop.DBus.Properties'",
			NULL);
	dbus_connection_flush(conn);
	if(pthread_create(&th, NULL, init_pacmd_socket, NULL) < 0){
		printf("Create pulse volume control thread failed\n");
	}
}
int pulse_pacmd_send(const char *str)
{
	if(fd >= 0){
		if (pa_write(fd, str, strlen(str), &fd_type) < 0) {
			pa_log(_("write(): %s"), strerror(errno));
			return -1;
		}
		return 0;
	}else{
		return -1;
	}
}

dbus_bool_t process_pulse_pacmd(DBusConnection *conn, DBusMessage *msg)
{
	const char *member, *path;
	path = dbus_message_get_path(msg);
	if(path && !strcmp(path, PATH_DBUS_INGENIC_PULSE_PACMD)){
		member = dbus_message_get_member(msg);
		if(!strcmp(member, "command")){
			const char *command;
			DBusMessageIter iter;
			dbus_message_iter_init(msg, &iter);
			dbus_message_iter_get_basic(&iter, &command); 
			printf(command);
			pulse_pacmd_send(command);
			return TRUE;
		}else{
			printf("Not support pacmd command\n");
		}
	}
	return FALSE;
}

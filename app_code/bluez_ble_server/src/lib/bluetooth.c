/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "bluetooth.h"
#include "hci.h"
#include "mem_manage.h"

void baswap(bdaddr_t *dst, const bdaddr_t *src)
{
	register unsigned char *d = (unsigned char *) dst;
	register const unsigned char *s = (const unsigned char *) src;
	register int i;

	for (i = 0; i < 6; i++)
		d[i] = s[5-i];
}

char *batostr(const bdaddr_t *ba)
{
	char *str = bt_malloc(18);
	if (!str)
		return NULL;

	sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		ba->b[0], ba->b[1], ba->b[2],
		ba->b[3], ba->b[4], ba->b[5]);

	return str;
}

bdaddr_t *strtoba(const char *str)
{
	bdaddr_t b;
	bdaddr_t *ba = bt_malloc(sizeof(*ba));

	if (ba) {
		str2ba(str, &b);
		baswap(ba, &b);
	}

	return ba;
}

int ba2str(const bdaddr_t *ba, char *str)
{
	return sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
}

int str2ba(const char *str, bdaddr_t *ba)
{
	int i;

	if (bachk(str) < 0) {
		memset(ba, 0, sizeof(*ba));
		return -1;
	}

	for (i = 5; i >= 0; i--, str += 3)
		ba->b[i] = strtol(str, NULL, 16);

	return 0;
}

int ba2oui(const bdaddr_t *ba, char *str)
{
	return sprintf(str, "%2.2X-%2.2X-%2.2X", ba->b[5], ba->b[4], ba->b[3]);
}

int bachk(const char *str)
{
	if (!str)
		return -1;

	if (strlen(str) != 17)
		return -1;

	while (*str) {
		if (!isxdigit(*str++))
			return -1;

		if (!isxdigit(*str++))
			return -1;

		if (*str == 0)
			break;

		if (*str++ != ':')
			return -1;
	}

	return 0;
}

int baprintf(const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vprintf(format, ap);
	va_end(ap);

	return len;
}

int bafprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vfprintf(stream, format, ap);
	va_end(ap);

	return len;
}

int basprintf(char *str, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, (~0U) >> 1, format, ap);
	va_end(ap);

	return len;
}

int basnprintf(char *str, size_t size, const char *format, ...)
{
	va_list ap;
	int len;

	va_start(ap, format);
	len = vsnprintf(str, size, format, ap);
	va_end(ap);

	return len;
}

void *bt_malloc(size_t size)
{
	return mem_malloc(size);
}

void bt_free(void *ptr)
{
	mem_free(ptr);
}

/* Bluetooth error codes to Unix errno mapping */
int bt_error(uint16_t code)
{
	switch (code) {
	case 0:
		return 0;
	case HCI_UNKNOWN_COMMAND:
		return EBADRQC;
	case HCI_NO_CONNECTION:
		return ENOTCONN;
	case HCI_HARDWARE_FAILURE:
		return EIO;
	case HCI_PAGE_TIMEOUT:
		return EHOSTDOWN;
	case HCI_AUTHENTICATION_FAILURE:
		return EACCES;
	case HCI_PIN_OR_KEY_MISSING:
		return EINVAL;
	case HCI_MEMORY_FULL:
		return ENOMEM;
	case HCI_CONNECTION_TIMEOUT:
		return ETIMEDOUT;
	case HCI_MAX_NUMBER_OF_CONNECTIONS:
	case HCI_MAX_NUMBER_OF_SCO_CONNECTIONS:
		return EMLINK;
	case HCI_ACL_CONNECTION_EXISTS:
		return EALREADY;
	case HCI_COMMAND_DISALLOWED:
	case HCI_TRANSACTION_COLLISION:
	case HCI_ROLE_SWITCH_PENDING:
		return EBUSY;
	case HCI_REJECTED_LIMITED_RESOURCES:
	case HCI_REJECTED_PERSONAL:
	case HCI_QOS_REJECTED:
		return ECONNREFUSED;
	case HCI_HOST_TIMEOUT:
		return ETIMEDOUT;
	case HCI_UNSUPPORTED_FEATURE:
	case HCI_QOS_NOT_SUPPORTED:
	case HCI_PAIRING_NOT_SUPPORTED:
	case HCI_CLASSIFICATION_NOT_SUPPORTED:
	case HCI_UNSUPPORTED_LMP_PARAMETER_VALUE:
	case HCI_PARAMETER_OUT_OF_RANGE:
	case HCI_QOS_UNACCEPTABLE_PARAMETER:
		return EOPNOTSUPP;
	case HCI_INVALID_PARAMETERS:
	case HCI_SLOT_VIOLATION:
		return EINVAL;
	case HCI_OE_USER_ENDED_CONNECTION:
	case HCI_OE_LOW_RESOURCES:
	case HCI_OE_POWER_OFF:
		return ECONNRESET;
	case HCI_CONNECTION_TERMINATED:
		return ECONNABORTED;
	case HCI_REPEATED_ATTEMPTS:
		return ELOOP;
	case HCI_REJECTED_SECURITY:
	case HCI_PAIRING_NOT_ALLOWED:
	case HCI_INSUFFICIENT_SECURITY:
		return EACCES;
	case HCI_UNSUPPORTED_REMOTE_FEATURE:
		return EPROTONOSUPPORT;
	case HCI_SCO_OFFSET_REJECTED:
		return ECONNREFUSED;
	case HCI_UNKNOWN_LMP_PDU:
	case HCI_INVALID_LMP_PARAMETERS:
	case HCI_LMP_ERROR_TRANSACTION_COLLISION:
	case HCI_LMP_PDU_NOT_ALLOWED:
	case HCI_ENCRYPTION_MODE_NOT_ACCEPTED:
		return EPROTO;
	default:
		return ENOSYS;
	}
}

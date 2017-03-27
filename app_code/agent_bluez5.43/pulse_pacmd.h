/*
 * pulse_pacmd.h
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

#ifndef PULSE_PACMD_H__
#define PULSE_PACMD_H__

#define PATH_DBUS_INGENIC_PULSE_PACMD   "/ingenic/pulse/pacmd"

void pulse_pacmd_init(DBusConnection *conn);
dbus_bool_t process_pulse_pacmd(DBusConnection *conn, DBusMessage *msg);

#endif /* PULSE_PACMD_H__ */

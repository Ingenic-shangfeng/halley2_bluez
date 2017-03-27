/*
 * bt_mutex.h
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

#ifndef __BT_NUTEX_H__
#define __BT_NUTEX_H__

#include <pthread.h>
typedef pthread_mutex_t bt_mutex_t;

int bt_mutex_init(bt_mutex_t *mutex);
int bt_mutex_destroy(bt_mutex_t *mutex);
int bt_mutex_trylock(bt_mutex_t *mutex);
int bt_mutex_lock(bt_mutex_t *mutex);
int bt_mutex_unlock(bt_mutex_t *mutex);

#endif /* __BT_NUTEX_H__ */

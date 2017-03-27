/*
 * bt_semaphore.h
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

#ifndef __BT_SEMAPHORE_H__
#define __BT_SEMAPHORE_H__

#include <semaphore.h>
typedef sem_t bt_sem_t;

int bt_sem_init(bt_sem_t *sem, unsigned int value);
int bt_sem_wait(bt_sem_t *sem);
int bt_sem_try(bt_sem_t *sem);
int bt_sem_timedwait(bt_sem_t *sem,int time_s);
int bt_sem_post(bt_sem_t *sem);
int bt_sem_destroy(bt_sem_t *sem);

#endif /* __BT_SEMAPHORE_H__ */

/*
 * bt_semaphore.c
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

#include "bt_semaphore.h"
#include <time.h>

int bt_sem_init(bt_sem_t *sem, unsigned int value)
{
	return sem_init(sem, 0, value);
}

int bt_sem_wait(bt_sem_t *sem)
{
	return sem_wait(sem);
}

int bt_sem_try(bt_sem_t *sem)
{
	return sem_trywait(sem);
}

int bt_sem_timedwait(bt_sem_t *sem,int time_s)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += time_s;
	return sem_timedwait(sem, &ts);
}

int bt_sem_post(bt_sem_t *sem)
{
	return sem_post(sem);
}

int bt_sem_destroy(bt_sem_t *sem)
{
	return sem_destroy(sem);
}

/*
 * mem_manage.c
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
#include <string.h>
#include <stdlib.h>
#include "mem_manage.h"

#ifdef __MEMORY_DEBUG__

#define MEMORY_IN_USE  1
#define MEMORY_NOT_USE 0
struct mem_record{
	int addr;
	char func[32];
	int line;
	int state;
};

struct mem_record mem_record[4096];
int mem_record_cnt;

static void add_rec(int addr, const char *func, int line)
{
	struct mem_record *p;
	int i;
	for(i=0;i<mem_record_cnt;i++){
		if(addr == mem_record[i].addr){
			mem_record[i].state++;
			return;
		}
	}
	p = &mem_record[mem_record_cnt];
	p->addr = addr;
	memcpy(p->func, func, 31);
	p->line = line;
	mem_record[i].state = MEMORY_IN_USE;
	mem_record_cnt++;
}
static void rm_rec(int addr)
{
	int i;
	for(i=0;i<mem_record_cnt;i++){
		if(addr == mem_record[i].addr){
			mem_record[i].state--;
		}
	}
}
void _mem_dump(void)
{
	int i;
	Log.v("Total:%d(freed included)", mem_record_cnt);
	for(i=0;i<mem_record_cnt;i++){
		if(mem_record[i].state != MEMORY_NOT_USE){
			Log.v(" @%08x  %4d  %32s:%d",
					mem_record[i].addr, 
					mem_record[i].state,
					mem_record[i].func,
					mem_record[i].line);
		}
	}
}

void* _mem_malloc(int size, const char *func, int line)
{
	void *p;
	p = malloc(size);
	Log.v("%32s@%4d => malloc:%8p", func, line, p);
	add_rec((int)p, func, line);
	return p;
}

void* _mem_calloc(int size, int num, const char *func, int line)
{
	void *p;
	p = calloc(size, num);
	Log.v("%32s@%4d => calloc:%8p", func, line, p);
	add_rec((int)p, func, line);
	return p;
}

void _mem_free(void *ptr, const char *func, int line)
{
	rm_rec((int)ptr);
	free(ptr);
	Log.v("%32s@%4d =>   free:%8p", func, line, ptr);
	ptr = 0;
}

#endif


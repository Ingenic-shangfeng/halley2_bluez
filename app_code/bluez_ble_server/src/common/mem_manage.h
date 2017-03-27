/*
 * mem_manage.h
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

#ifndef __MEM_MANAGE_H__
#define __MEM_MANAGE_H__

#include "stdlib.h"
#include "log.h"

//#define __MEMORY_DEBUG__

#ifdef __MEMORY_DEBUG__
void* _mem_malloc(int size, const char *func, int line);
void* _mem_calloc(int size, int num, const char *func, int line);
void _mem_free(void *ptr, const char *func, int line);
void _mem_dump(void);

#define mem_malloc(size)       _mem_malloc ( size,       __func__, __LINE__)
#define mem_calloc(size, num)  _mem_calloc ( size, num,  __func__, __LINE__)
#define mem_calloc(size, num)  _mem_calloc ( size, num,  __func__, __LINE__)
#define mem_realloc(ptr, size) _mem_realloc( ptr,  size, __func__, __LINE__)
#define mem_free(ptr)          _mem_free   ( ptr,        __func__, __LINE__)
#define mem_dump()             _mem_dump   ()

#else

#define mem_malloc(size)         malloc(size)
#define mem_calloc(nmemb, size)  calloc(nmemb, size)
#define mem_realloc(ptr, size)   realloc(ptr, size)
#define mem_free(ptr)            free(ptr)
#define mem_dump()               do{}while(0)

#endif

#endif /* __MEM_MANAGE_H__ */

/*
 * log.c
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

#include <stdarg.h>
#include "log.h"

void log_v(const char* fmt, ...);
void log_d(const char* fmt, ...);
void log_e(const char* fmt, ...);

struct log Log = {
	log_v,
	log_d,
	log_e,
};

void log_v(const char* fmt, ...)
{
	va_list args;
	printf("V:");
	va_start(args,fmt);
	vprintf(fmt,args);
	va_end(args);
	printf("\n");
}
void log_d(const char* fmt, ...)
{
	va_list args;
	printf("D:");
	va_start(args,fmt);
	vprintf(fmt,args);
	va_end(args);
	printf("\n");
}
void log_e(const char* fmt, ...)
{
	va_list args;
	printf("E:");
	va_start(args,fmt);
	vprintf(fmt,args);
	va_end(args);
	printf("\n");
}

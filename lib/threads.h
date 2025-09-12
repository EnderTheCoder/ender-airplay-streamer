/**
*  Copyright (C) 2011-2012  Juho Vähä-Herttua
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 */

#ifndef THREADS_H
#define THREADS_H

#if defined(_WIN32) || defined(WIN32)
/* Windows平台 */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <pthread.h>  // 使用pthreads4w

#define sleepms(x) Sleep(x)

#else
/* Unix-like平台 */
#include <pthread.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define sleepms(x) usleep((x)*1000)

#endif

/* 现在所有平台都使用pthread类型和函数 */
typedef pthread_t thread_handle_t;

#define THREAD_RETVAL void*
#define THREAD_CREATE(handle, func, arg) \
pthread_create(&(handle), NULL, func, arg)
#define THREAD_JOIN(handle) pthread_join((handle), NULL)

typedef pthread_mutex_t mutex_handle_t;

#define MUTEX_CREATE(handle) pthread_mutex_init(&(handle), NULL)
#define MUTEX_LOCK(handle) pthread_mutex_lock(&(handle))
#define MUTEX_UNLOCK(handle) pthread_mutex_unlock(&(handle))
#define MUTEX_DESTROY(handle) pthread_mutex_destroy(&(handle))

typedef pthread_cond_t cond_handle_t;

#define COND_CREATE(handle) pthread_cond_init(&(handle), NULL)
#define COND_SIGNAL(handle) pthread_cond_signal(&(handle))
#define COND_BROADCAST(handle) pthread_cond_broadcast(&(handle))
#define COND_WAIT(cond, mutex) pthread_cond_wait(&(cond), &(mutex))
#define COND_DESTROY(handle) pthread_cond_destroy(&(handle))

#endif /* THREADS_H */
/////////////////////////////////////////////////////////////////////////
// $Id: bxcompat.h 14074 2021-01-09 16:51:52Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2013       Volker Ruppert
//  Copyright (C) 2001-2021  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
/////////////////////////////////////////////////////////////////////////

#ifndef BX_COMPAT_H
#define BX_COMPAT_H

// copied from bochs.h
#ifdef WIN32
#  include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifndef _MSC_VER
#  include <unistd.h>
#else
#  include <io.h>
#endif
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// definitions for compatibility with Bochs
#ifndef UNUSED
#  define UNUSED(x) ((void)x)
#endif

#ifdef BXIMAGE
#define BX_DEBUG(x)
#define BX_INFO(x)  { if (bx_interactive) { (printf) x ; printf("\n"); } }
#define BX_ERROR(x) { (printf) x ; printf("\n"); }
#define BX_PANIC(x) { (printf) x ; printf("\n"); myexit(1); }
#define BX_FATAL(x) { (printf) x ; printf("\n"); myexit(1); }
#else
#define BX_DEBUG(x) { if (bx_loglev == 3) { (bx_printf) x ; } }
#define BX_INFO(x)  { if (bx_loglev >= 2) { (bx_printf) x ; } }
#define BX_ERROR(x) { if (bx_loglev >= 1) { (bx_printf) x ; } }
#endif
#define BX_ASSERT(x)

#ifdef BXIMAGE
extern int bx_interactive;

class device_image_t;

void myexit(int code);
device_image_t* init_image(const char *image_mode);

#define DEV_hdimage_init_image(a,b,c) init_image(a)

#else

#define BX_PATHNAME_LEN 512

extern int bx_loglev;

#endif

#endif

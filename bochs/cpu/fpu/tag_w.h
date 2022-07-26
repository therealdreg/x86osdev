/////////////////////////////////////////////////////////////////////////
// $Id: tag_w.h 12857 2015-10-09 19:33:36Z sshwarts $
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2003-2015 Stanislav Shwartsman
//          Written by Stanislav Shwartsman [sshwarts at sourceforge net]
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
//
/////////////////////////////////////////////////////////////////////////

#ifndef _TAG_W_H
#define _TAG_W_H

/* Tag Word */
enum {
  FPU_Tag_Valid   = 0x00,
  FPU_Tag_Zero    = 0x01,
  FPU_Tag_Special = 0x02,
  FPU_Tag_Empty   = 0x03
};

#endif

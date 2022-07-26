/////////////////////////////////////////////////////////////////////////
// $Id: ia_opcodes.h 13828 2020-03-28 14:23:54Z sshwarts $
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2017 Stanislav Shwartsman
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
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#ifndef BX_OPCODES_ENUM
#define BX_OPCODES_ENUM

enum {
#define bx_define_opcode(a, b, c, d, e, f, s1, s2, s3, s4, g) a,
#include "ia_opcodes.def"
   BX_IA_LAST
};
#undef  bx_define_opcode

#endif

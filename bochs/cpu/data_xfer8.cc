/////////////////////////////////////////////////////////////////////////
// $Id: data_xfer8.cc 13466 2018-02-16 07:57:32Z sshwarts $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001-2018  The Bochs Project
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
/////////////////////////////////////////////////////////////////////////

#define NEED_CPU_REG_SHORTCUTS 1
#include "bochs.h"
#include "cpu.h"
#define LOG_THIS BX_CPU_THIS_PTR

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_EbIbR(bxInstruction_c *i)
{
  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), i->Ib());

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_EbGbM(bxInstruction_c *i)
{
  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);

  write_virtual_byte(i->seg(), eaddr, BX_READ_8BIT_REGx(i->src(), i->extend8bitL()));

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_GbEbM(bxInstruction_c *i)
{
  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);

  Bit8u val8 = read_virtual_byte(i->seg(), eaddr);
  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), val8);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_GbEbR(bxInstruction_c *i)
{
  Bit8u op2 = BX_READ_8BIT_REGx(i->src(), i->extend8bitL());
  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), op2);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_ALOd(bxInstruction_c *i)
{
  AL = read_virtual_byte_32(i->seg(), i->Id());

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_OdAL(bxInstruction_c *i)
{
  write_virtual_byte_32(i->seg(), i->Id(), AL);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::MOV_EbIbM(bxInstruction_c *i)
{
  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);

  write_virtual_byte(i->seg(), eaddr, i->Ib());

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::XLAT(bxInstruction_c *i)
{
#if BX_SUPPORT_X86_64
  if (i->as64L()) {
    AL = read_linear_byte(i->seg(), get_laddr64(i->seg(), RBX + AL));
  }
  else
#endif
  {
    AL = read_virtual_byte(i->seg(), (EBX + AL) & i->asize_mask());
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::XCHG_EbGbM(bxInstruction_c *i)
{
  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);

  Bit8u op1 = read_RMW_virtual_byte(i->seg(), eaddr);
  Bit8u op2 = BX_READ_8BIT_REGx(i->src(), i->extend8bitL());

  write_RMW_linear_byte(op2);
  BX_WRITE_8BIT_REGx(i->src(), i->extend8bitL(), op1);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::XCHG_EbGbR(bxInstruction_c *i)
{
  Bit8u op1 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());
  Bit8u op2 = BX_READ_8BIT_REGx(i->src(), i->extend8bitL());

  BX_WRITE_8BIT_REGx(i->src(), i->extend8bitL(), op1);
  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), op2);

  BX_NEXT_INSTR(i);
}

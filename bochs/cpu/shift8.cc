/////////////////////////////////////////////////////////////////////////
// $Id: shift8.cc 13466 2018-02-16 07:57:32Z sshwarts $
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

#include "decoder/ia_opcodes.h"

void BX_CPP_AttrRegparmN(1) BX_CPU_C::ROL_EbR(bxInstruction_c *i)
{
  unsigned count;
  unsigned bit0, bit7;

  if (i->getIaOpcode() == BX_IA_ROL_Eb)
    count = CL;
  else
    count = i->Ib();

  Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());

  if ((count & 0x07) == 0) {
    if (count & 0x18) {
      bit0 = (op1_8 &  1);
      bit7 = (op1_8 >> 7);
      BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit0 ^ bit7, bit0);
    }
  }
  else {
    count &= 0x7; // use only lowest 3 bits

    Bit8u result_8 = (op1_8 << count) | (op1_8 >> (8 - count));

    BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

    /* set eflags:
     * ROL count affects the following flags: C, O
     */
    bit0 = (result_8 &  1);
    bit7 = (result_8 >> 7);

    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit0 ^ bit7, bit0);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::ROL_EbM(bxInstruction_c *i)
{
  unsigned count;
  unsigned bit0, bit7;

  if (i->getIaOpcode() == BX_IA_ROL_Eb)
    count = CL;
  else
    count = i->Ib();

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  if ((count & 0x07) == 0) {
    if (count & 0x18) {
      bit0 = (op1_8 &  1);
      bit7 = (op1_8 >> 7);
      BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit0 ^ bit7, bit0);
    }
  }
  else {
    count &= 0x7; // use only lowest 3 bits

    Bit8u result_8 = (op1_8 << count) | (op1_8 >> (8 - count));

    write_RMW_linear_byte(result_8);

    /* set eflags:
     * ROL count affects the following flags: C, O
     */
    bit0 = (result_8 &  1);
    bit7 = (result_8 >> 7);

    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit0 ^ bit7, bit0);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::ROR_EbR(bxInstruction_c *i)
{
  unsigned count;
  unsigned bit6, bit7;

  if (i->getIaOpcode() == BX_IA_ROR_Eb)
    count = CL;
  else
    count = i->Ib();

  Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());

  if ((count & 0x07) == 0) {
    if (count & 0x18) {
      bit6 = (op1_8 >> 6) & 1;
      bit7 = (op1_8 >> 7) & 1;

      BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit6 ^ bit7, bit7);
    }
  }
  else {
    count &= 0x7; /* use only bottom 3 bits */

    Bit8u result_8 = (op1_8 >> count) | (op1_8 << (8 - count));

    BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

    /* set eflags:
     * ROR count affects the following flags: C, O
     */
    bit6 = (result_8 >> 6) & 1;
    bit7 = (result_8 >> 7) & 1;

    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit6 ^ bit7, bit7);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::ROR_EbM(bxInstruction_c *i)
{
  unsigned count;
  unsigned bit6, bit7;

  if (i->getIaOpcode() == BX_IA_ROR_Eb)
    count = CL;
  else
    count = i->Ib();

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  if ((count & 0x07) == 0) {
    if (count & 0x18) {
      bit6 = (op1_8 >> 6) & 1;
      bit7 = (op1_8 >> 7) & 1;

      BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit6 ^ bit7, bit7);
    }
  }
  else {
    count &= 0x7; /* use only bottom 3 bits */

    Bit8u result_8 = (op1_8 >> count) | (op1_8 << (8 - count));

    write_RMW_linear_byte(result_8);

    /* set eflags:
     * ROR count affects the following flags: C, O
     */
    bit6 = (result_8 >> 6) & 1;
    bit7 = (result_8 >> 7) & 1;

    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(bit6 ^ bit7, bit7);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::RCL_EbR(bxInstruction_c *i)
{
  Bit8u result_8;
  unsigned count;
  unsigned of, cf;

  if (i->getIaOpcode() == BX_IA_RCL_Eb)
    count = CL;
  else
    count = i->Ib();

  count = (count & 0x1f) % 9;

  if (! count) {
    BX_NEXT_INSTR(i);
  }

  Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());

  unsigned temp_CF = getB_CF();

  if (count==1) {
    result_8 = (op1_8 << 1) | temp_CF;
  }
  else {
    result_8 = (op1_8 << count) | (temp_CF << (count - 1)) |
               (op1_8 >> (9 - count));
  }

  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

  cf = (op1_8 >> (8 - count)) & 0x01;
  of = cf ^ (result_8 >> 7);  // of = cf ^ result7
  BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::RCL_EbM(bxInstruction_c *i)
{
  Bit8u result_8;
  unsigned count;
  unsigned of, cf;

  if (i->getIaOpcode() == BX_IA_RCL_Eb)
    count = CL;
  else
    count = i->Ib();

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  count = (count & 0x1f) % 9;

  if (! count) {
    BX_NEXT_INSTR(i);
  }

  unsigned temp_CF = getB_CF();

  if (count==1) {
    result_8 = (op1_8 << 1) | temp_CF;
  }
  else {
    result_8 = (op1_8 << count) | (temp_CF << (count - 1)) |
               (op1_8 >> (9 - count));
  }

  write_RMW_linear_byte(result_8);

  cf = (op1_8 >> (8 - count)) & 0x01;
  of = cf ^ (result_8 >> 7);  // of = cf ^ result7
  BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::RCR_EbR(bxInstruction_c *i)
{
  unsigned count;
  unsigned cf, of;

  if (i->getIaOpcode() == BX_IA_RCR_Eb)
    count = CL;
  else
    count = i->Ib();

  count = (count & 0x1f) % 9;

  if (count) {
    Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());

    unsigned temp_CF = getB_CF();

    Bit8u result_8 = (op1_8 >> count) | (temp_CF << (8 - count)) |
                     (op1_8 << (9 - count));

    BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

    cf = (op1_8 >> (count - 1)) & 0x1;
    of = (((result_8 << 1) ^ result_8) >> 7) & 0x1; // of = result6 ^ result7
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::RCR_EbM(bxInstruction_c *i)
{
  unsigned count;
  unsigned cf, of;

  if (i->getIaOpcode() == BX_IA_RCR_Eb)
    count = CL;
  else
    count = i->Ib();

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  count = (count & 0x1f) % 9;

  if (count) {
    unsigned temp_CF = getB_CF();

    Bit8u result_8 = (op1_8 >> count) | (temp_CF << (8 - count)) |
                     (op1_8 << (9 - count));

    write_RMW_linear_byte(result_8);

    cf = (op1_8 >> (count - 1)) & 0x1;
    of = (((result_8 << 1) ^ result_8) >> 7) & 0x1; // of = result6 ^ result7
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SHL_EbR(bxInstruction_c *i)
{
  Bit8u result_8;
  unsigned count;
  unsigned of = 0, cf = 0;

  if (i->getIaOpcode() == BX_IA_SHL_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  if (!count) {
    BX_NEXT_INSTR(i);
  }

  Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());

  if (count <= 8) {
    result_8 = (op1_8 << count);
    cf = (op1_8 >> (8 - count)) & 0x1;
    of = cf ^ (result_8 >> 7);
  }
  else {
    result_8 = 0;
  }

  BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

  SET_FLAGS_OSZAPC_LOGIC_8(result_8);
  BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SHL_EbM(bxInstruction_c *i)
{
  Bit8u result_8;
  unsigned count;
  unsigned of = 0, cf = 0;

  if (i->getIaOpcode() == BX_IA_SHL_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  if (!count) {
    BX_NEXT_INSTR(i);
  }

  if (count <= 8) {
    result_8 = (op1_8 << count);
    cf = (op1_8 >> (8 - count)) & 0x1;
    of = cf ^ (result_8 >> 7);
  }
  else {
    result_8 = 0;
  }

  write_RMW_linear_byte(result_8);

  SET_FLAGS_OSZAPC_LOGIC_8(result_8);
  BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SHR_EbR(bxInstruction_c *i)
{
  unsigned count;

  if (i->getIaOpcode() == BX_IA_SHR_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  if (count) {
    Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());
    Bit8u result_8 = (op1_8 >> count);
    BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

    unsigned cf = (op1_8 >> (count - 1)) & 0x1;
    // note, that of == result7 if count == 1 and
    //            of == 0       if count >= 2
    unsigned of = (((result_8 << 1) ^ result_8) >> 7) & 0x1;

    SET_FLAGS_OSZAPC_LOGIC_8(result_8);
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SHR_EbM(bxInstruction_c *i)
{
  unsigned count;

  if (i->getIaOpcode() == BX_IA_SHR_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  if (count) {
    Bit8u result_8 = (op1_8 >> count);

    write_RMW_linear_byte(result_8);

    unsigned cf = (op1_8 >> (count - 1)) & 0x1;
    // note, that of == result7 if count == 1 and
    //            of == 0       if count >= 2
    unsigned of = (((result_8 << 1) ^ result_8) >> 7) & 0x1;

    SET_FLAGS_OSZAPC_LOGIC_8(result_8);
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(of, cf);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SAR_EbR(bxInstruction_c *i)
{
  unsigned count;

  if (i->getIaOpcode() == BX_IA_SAR_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  if (count) {
    Bit8u op1_8 = BX_READ_8BIT_REGx(i->dst(), i->extend8bitL());
    Bit8u result_8 = ((Bit8s) op1_8) >> count;
    BX_WRITE_8BIT_REGx(i->dst(), i->extend8bitL(), result_8);

    unsigned cf = (((Bit8s) op1_8) >> (count - 1)) & 0x1;

    SET_FLAGS_OSZAPC_LOGIC_8(result_8);
    /* signed overflow cannot happen in SAR instruction */
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(0, cf);
  }

  BX_NEXT_INSTR(i);
}

void BX_CPP_AttrRegparmN(1) BX_CPU_C::SAR_EbM(bxInstruction_c *i)
{
  unsigned count;

  if (i->getIaOpcode() == BX_IA_SAR_Eb)
    count = CL;
  else
    count = i->Ib();

  count &= 0x1f;

  bx_address eaddr = BX_CPU_RESOLVE_ADDR(i);
  /* pointer, segment address pair */
  Bit8u op1_8 = read_RMW_virtual_byte(i->seg(), eaddr);

  if (count) {
    Bit8u result_8 = ((Bit8s) op1_8) >> count;

    write_RMW_linear_byte(result_8);

    unsigned cf = (((Bit8s) op1_8) >> (count - 1)) & 0x1;

    SET_FLAGS_OSZAPC_LOGIC_8(result_8);
    /* signed overflow cannot happen in SAR instruction */
    BX_CPU_THIS_PTR oszapc.set_flags_OxxxxC(0, cf);
  }

  BX_NEXT_INSTR(i);
}

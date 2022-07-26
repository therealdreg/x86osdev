/////////////////////////////////////////////////////////////////////////
// $Id: busmouse.h 14112 2021-01-31 10:50:53Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004-2021  The Bochs Project
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


#ifndef _PCBUSM_H
#define _PCBUSM_H

#if BX_SUPPORT_BUSMOUSE

// these keywords should only be used in busmouse.cc
#if BX_USE_BUSM_SMF
#  define BX_BUSM_SMF  static
#  define BX_BUSM_THIS theBusMouse->
#else
#  define BX_BUSM_SMF
#  define BX_BUSM_THIS
#endif

class bx_busm_c : public bx_devmodel_c {
public:
  bx_busm_c();
  virtual ~bx_busm_c();

  virtual void init(void);
  virtual void reset(unsigned type) {}
  virtual void register_state(void);

private:
  static void timer_handler(void *);
  void busm_timer(void);

  static void mouse_enq_static(void *dev, int delta_x, int delta_y, int delta_z, unsigned button_state, bool absxy);
  void mouse_enq(int delta_x, int delta_y, int delta_z, unsigned button_state);
  void update_mouse_data(void);

  static Bit32u read_handler(void *this_ptr, Bit32u address, unsigned io_len);
  static void   write_handler(void *this_ptr, Bit32u address, Bit32u value, unsigned io_len);
#if !BX_USE_BUSM_SMF
  void   write(Bit32u address, Bit32u value, unsigned io_len);
  Bit32u  read(Bit32u address, unsigned io_len);
#endif

  int   type;
  int   timer_index; // our timer index

  int   mouse_delayed_dx;
  int   mouse_delayed_dy;
  Bit8u mouse_buttons;
  Bit8u mouse_buttons_last;
  Bit8u current_x, current_y, current_b;

  Bit8u control_val;
  Bit8u command_val;     // current command val
  Bit8u config_val;
  Bit8u sig_val;
  Bit16u toggle_counter;
  bool interrupts;       // 0 or 1.  interrupts off or on.
};

#endif  // BX_SUPPORT_BUSMOUSE

#endif  // #ifndef _PCBUSM_H

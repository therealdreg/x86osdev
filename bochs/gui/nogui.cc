/////////////////////////////////////////////////////////////////////////
// $Id: nogui.cc 14099 2021-01-30 19:20:45Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
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


// Define BX_PLUGGABLE in files that can be compiled into plugins.  For
// platforms that require a special tag on exported symbols, BX_PLUGGABLE
// is used to know when we are exporting symbols and when we are importing.
#define BX_PLUGGABLE

#include "bochs.h"
#include "gui.h"
#include "plugin.h"
#include "param_names.h"

#if BX_WITH_NOGUI
#include "icon_bochs.h"

class bx_nogui_gui_c : public bx_gui_c {
public:
  bx_nogui_gui_c (void) {}
  DECLARE_GUI_VIRTUAL_METHODS()
};

// declare one instance of the gui object and call macro to insert the
// plugin code
static bx_nogui_gui_c *theGui = NULL;
IMPLEMENT_GUI_PLUGIN_CODE(nogui)

#define LOG_THIS theGui->

// This file defines stubs for the GUI interface, which is a
// place to start if you want to port bochs to a platform, for
// which there is no support for your native GUI, or if you want to compile
// bochs without any native GUI support (no output window or
// keyboard input will be possible).
// Look in 'x.cc', 'carbon.cc', and 'win32.cc' for specific
// implementations of this interface.  -Kevin



// ::SPECIFIC_INIT()
//
// Called from gui.cc, once upon program startup, to allow for the
// specific GUI code (X11, Win32, ...) to be initialized.
//
// argc, argv: these arguments can be used to initialize the GUI with
//     specific options (X11 options, Win32 options,...)
//
// headerbar_y:  A headerbar (toolbar) is display on the top of the
//     VGA window, showing floppy status, and other information.  It
//     always assumes the width of the current VGA mode width, but
//     it's height is defined by this parameter.

void bx_nogui_gui_c::specific_init(int argc, char **argv, unsigned headerbar_y)
{
  put("NOGUI");
  UNUSED(argc);
  UNUSED(argv);
  UNUSED(headerbar_y);

  UNUSED(bochs_icon_bits);  // global variable

  if (SIM->get_param_bool(BXPN_PRIVATE_COLORMAP)->get()) {
    BX_INFO(("private_colormap option ignored."));
  }
}


// ::HANDLE_EVENTS()
//
// Called periodically (every 1 virtual millisecond) so the
// the gui code can poll for keyboard, mouse, and other
// relevant events.

void bx_nogui_gui_c::handle_events(void)
{
}


// ::FLUSH()
//
// Called periodically, requesting that the gui code flush all pending
// screen update requests.

void bx_nogui_gui_c::flush(void)
{
}


// ::CLEAR_SCREEN()
//
// Called to request that the VGA region is cleared.  Don't
// clear the area that defines the headerbar.

void bx_nogui_gui_c::clear_screen(void)
{
}



// ::TEXT_UPDATE()
//
// Called in a VGA text mode, to update the screen with
// new content.
//
// old_text: array of character/attributes making up the contents
//           of the screen from the last call.  See below
// new_text: array of character/attributes making up the current
//           contents, which should now be displayed.  See below
//
// format of old_text & new_text: each is tm_info->line_offset*text_rows
//     bytes long. Each character consists of 2 bytes.  The first by is
//     the character value, the second is the attribute byte.
//
// cursor_x: new x location of cursor
// cursor_y: new y location of cursor
// tm_info:  this structure contains information for additional
//           features in text mode (cursor shape, line offset,...)

void bx_nogui_gui_c::text_update(Bit8u *old_text, Bit8u *new_text,
                      unsigned long cursor_x, unsigned long cursor_y,
                      bx_vga_tminfo_t *tm_info)
{
  UNUSED(old_text);
  UNUSED(new_text);
  UNUSED(cursor_x);
  UNUSED(cursor_y);
  UNUSED(tm_info);
}


// ::GET_CLIPBOARD_TEXT()
//
// Called to get text from the GUI clipboard. Returns 1 if successful.

int bx_nogui_gui_c::get_clipboard_text(Bit8u **bytes, Bit32s *nbytes)
{
  UNUSED(bytes);
  UNUSED(nbytes);
  return 0;
}


// ::SET_CLIPBOARD_TEXT()
//
// Called to copy the text screen contents to the GUI clipboard.
// Returns 1 if successful.

int bx_nogui_gui_c::set_clipboard_text(char *text_snapshot, Bit32u len)
{
  UNUSED(text_snapshot);
  UNUSED(len);
  return 0;
}


// ::PALETTE_CHANGE()
//
// Allocate a color in the native GUI, for this color, and put
// it in the colormap location 'index'.
// returns: 0=no screen update needed (color map change has direct effect)
//          1=screen updated needed (redraw using current colormap)

bool bx_nogui_gui_c::palette_change(Bit8u index, Bit8u red, Bit8u green, Bit8u blue)
{
  UNUSED(index);
  UNUSED(red);
  UNUSED(green);
  UNUSED(blue);
  return(0);
}


// ::GRAPHICS_TILE_UPDATE()
//
// Called to request that a tile of graphics be drawn to the
// screen, since info in this region has changed.
//
// tile: array of 8bit values representing a block of pixels with
//       dimension equal to the 'x_tilesize' & 'y_tilesize' members.
//       Each value specifies an index into the
//       array of colors you allocated for ::palette_change()
// x0: x origin of tile
// y0: y origin of tile
//
// note: origin of tile and of window based on (0,0) being in the upper
//       left of the window.

void bx_nogui_gui_c::graphics_tile_update(Bit8u *tile, unsigned x0, unsigned y0)
{
  UNUSED(tile);
  UNUSED(x0);
  UNUSED(y0);
}


// ::DIMENSION_UPDATE()
//
// Called when the VGA mode changes it's X,Y dimensions.
// Resize the window to this size, but you need to add on
// the height of the headerbar to the Y value.
//
// x: new VGA x size
// y: new VGA y size (add headerbar_y parameter from ::specific_init().
// fheight: new VGA character height in text mode
// fwidth : new VGA character width in text mode
// bpp : bits per pixel in graphics mode

void bx_nogui_gui_c::dimension_update(unsigned x, unsigned y, unsigned fheight, unsigned fwidth, unsigned bpp)
{
  guest_textmode = (fheight > 0);
  guest_xres = x;
  guest_yres = y;
  guest_bpp = bpp;
  UNUSED(fwidth);
}


// ::CREATE_BITMAP()
//
// Create a monochrome bitmap of size 'xdim' by 'ydim', which will
// be drawn in the headerbar.  Return an integer ID to the bitmap,
// with which the bitmap can be referenced later.
//
// bmap: packed 8 pixels-per-byte bitmap.  The pixel order is:
//       bit0 is the left most pixel, bit7 is the right most pixel.
// xdim: x dimension of bitmap
// ydim: y dimension of bitmap

unsigned bx_nogui_gui_c::create_bitmap(const unsigned char *bmap, unsigned xdim, unsigned ydim)
{
  UNUSED(bmap);
  UNUSED(xdim);
  UNUSED(ydim);
  return(0);
}


// ::HEADERBAR_BITMAP()
//
// Called to install a bitmap in the bochs headerbar (toolbar).
//
// bmap_id: will correspond to an ID returned from
//     ::create_bitmap().  'alignment' is either BX_GRAVITY_LEFT
//     or BX_GRAVITY_RIGHT, meaning install the bitmap in the next
//     available leftmost or rightmost space.
// alignment: is either BX_GRAVITY_LEFT or BX_GRAVITY_RIGHT,
//     meaning install the bitmap in the next
//     available leftmost or rightmost space.
// f: a 'C' function pointer to callback when the mouse is clicked in
//     the boundaries of this bitmap.

unsigned bx_nogui_gui_c::headerbar_bitmap(unsigned bmap_id, unsigned alignment, void (*f)(void))
{
  UNUSED(bmap_id);
  UNUSED(alignment);
  UNUSED(f);
  return(0);
}


// ::SHOW_HEADERBAR()
//
// Show (redraw) the current headerbar, which is composed of
// currently installed bitmaps.

void bx_nogui_gui_c::show_headerbar(void)
{
}


// ::REPLACE_BITMAP()
//
// Replace the bitmap installed in the headerbar ID slot 'hbar_id',
// with the one specified by 'bmap_id'.  'bmap_id' will have
// been generated by ::create_bitmap().  The old and new bitmap
// must be of the same size.  This allows the bitmap the user
// sees to change, when some action occurs.  For example when
// the user presses on the floppy icon, it then displays
// the ejected status.
//
// hbar_id: headerbar slot ID
// bmap_id: bitmap ID

void bx_nogui_gui_c::replace_bitmap(unsigned hbar_id, unsigned bmap_id)
{
  UNUSED(hbar_id);
  UNUSED(bmap_id);
}


// ::EXIT()
//
// Called before bochs terminates, to allow for a graceful
// exit from the native GUI mechanism.

void bx_nogui_gui_c::exit(void)
{
  BX_INFO(("bx_nogui_gui_c::exit() not implemented yet."));
}


// ::MOUSE_ENABLED_CHANGED_SPECIFIC()
//
// Called whenever the mouse capture mode should be changed. It can change
// because of a gui event such as clicking on the mouse bitmap / button of
// the header / tool bar, toggle the mouse capture using the configured
// method with keyboard or mouse, or from the configuration interface.

void bx_nogui_gui_c::mouse_enabled_changed_specific(bool val)
{
}

#endif /* if BX_WITH_NOGUI */

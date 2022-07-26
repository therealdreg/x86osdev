/////////////////////////////////////////////////////////////////////////
// $Id: term.cc 14094 2021-01-30 18:32:52Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2000-2021  The Bochs Project
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
#include "param_names.h"
#include "iodev.h"
#if BX_WITH_TERM

extern "C" {
#include <curses.h>
#include <signal.h>
};

#define BX_DEBUGGER_TERM (BX_DEBUGGER && !defined(__OpenBSD__))

class bx_term_gui_c : public bx_gui_c {
public:
  bx_term_gui_c(void) {}
  DECLARE_GUI_VIRTUAL_METHODS()
#if BX_USE_IDLE_HACK
  virtual void sim_is_idle(void);
#endif

  virtual Bit32u get_sighandler_mask();

  // called when registered signal arrives
  virtual void sighandler(int sig);

#if BX_SHOW_IPS
  void show_ips(Bit32u ips_count);
#endif
};

// declare one instance of the gui object and call macro to insert the
// plugin code
static bx_term_gui_c *theGui = NULL;
#if BX_DEBUGGER_TERM
static int scr_fd = -1;
#endif
IMPLEMENT_GUI_PLUGIN_CODE(term)

#define LOG_THIS theGui->

bool initialized = 0;
bool termHideIPS = 0;
static unsigned int text_rows = 25, text_cols = 80;
static unsigned long last_cursor_x, last_cursor_y;

static short curses_color[8] = {
  /* 0 */ COLOR_BLACK,
  /* 1 */ COLOR_BLUE,
  /* 2 */ COLOR_GREEN,
  /* 3 */ COLOR_CYAN,
  /* 4 */ COLOR_RED,
  /* 5 */ COLOR_MAGENTA,
  /* 6 */ COLOR_YELLOW,
  /* 7 */ COLOR_WHITE
};

static chtype vga_to_term[128] = {
  0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7,
  0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,
  0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9,
  0xff, 0xd6, 0xdc, 0xe7, 0xa3, 0xa5, ' ',  ' ',
  0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba,
  0xbf, ' ',  0xac, ' ',  ' ',  0xa1, 0xab, 0xbb,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, ' ',  ' ',  ' ',  ' ',
  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  0xb5, ' ',
  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',  ' ',
  ' ',  0xb1, ' ',  ' ',  ' ',  ' ',  0xf7, ' ',
  0xb0, ' ',  ' ',  ' ',  ' ',  0xb2, ' ',  ' '
};

static void do_scan(int key_event, int shift, int ctrl, int alt)
{
  /* At some point, cache alt/ctrl/shift so only when the state
     changes do we simulate a press or release, to cut down on
     keyboard input to the simulated machine */

  BX_DEBUG(("key_event %d/0x%x %s%s%s", key_event, key_event,
                  shift ? "(shift)" : "",
                  ctrl ? "(ctrl)" : "",
                  alt ? "(alt)" : ""));
  if(shift)
    DEV_kbd_gen_scancode(BX_KEY_SHIFT_L);
  if(ctrl)
    DEV_kbd_gen_scancode(BX_KEY_CTRL_L);
  if(alt)
    DEV_kbd_gen_scancode(BX_KEY_ALT_L);
  DEV_kbd_gen_scancode(key_event);
  key_event |= BX_KEY_RELEASED;

  DEV_kbd_gen_scancode(key_event);
  if(alt)
    DEV_kbd_gen_scancode(BX_KEY_ALT_L|BX_KEY_RELEASED);
  if(ctrl)
    DEV_kbd_gen_scancode(BX_KEY_CTRL_L|BX_KEY_RELEASED);
  if(shift)
    DEV_kbd_gen_scancode(BX_KEY_SHIFT_L|BX_KEY_RELEASED);
}

Bit32u bx_term_gui_c::get_sighandler_mask()
{
  return 0
#ifdef SIGHUP
       | (1<<SIGHUP)
#endif
       | (1<<SIGINT)
#ifdef SIGQUIT
       | (1<<SIGQUIT)
#endif
#ifdef SIGSTOP
       | (1<<SIGSTOP)
#endif
#ifdef SIGTSTP
       | (1<<SIGTSTP)
#endif
       | (1<<SIGTERM);
}

void bx_term_gui_c::sighandler(int signo)
{
  switch(signo) {
    case SIGINT:
      do_scan(BX_KEY_C,0,1,0);
      break;
#ifdef SIGSTOP
    case SIGSTOP:
      do_scan(BX_KEY_S,0,1,0);
      break;
#endif
#ifdef SIGTSTP
    case SIGTSTP:
      do_scan(BX_KEY_Z,0,1,0);
      break;
#endif
#ifdef SIGHUP
    case SIGHUP:
      BX_PANIC(("Received SIGHUP: quit simulation"));
      break;
#endif
    default:
      BX_INFO(("sig %d caught",signo));
      break;
  }
}

// TERM implementation of the bx_gui_c methods (see nogui.cc for details)

void bx_term_gui_c::specific_init(int argc, char **argv, unsigned headerbar_y)
{
  int i;

  put("TERM");
  // the ask menu causes trouble
  io->set_log_action(LOGLEV_PANIC, ACT_FATAL);
#if !BX_DEBUGGER_TERM
  // logfile should be different from stderr, otherwise terminal mode
  // really ends up having fun
  if (!strcmp(SIM->get_param_string(BXPN_LOG_FILENAME)->getptr(), "-"))
    BX_PANIC(("cannot log to stderr in term mode"));
#else
  FILE *old_stdin = stdin;
  FILE *old_stdout = stdout;
  scr_fd = open("/dev/ptmx",O_RDWR);
  if(scr_fd > 0){
    stdin = stdout = fdopen(scr_fd,"wr");
    grantpt(scr_fd);
    unlockpt(scr_fd);
    fprintf(stderr, "\nBochs connected to screen \"%s\"\n",ptsname(scr_fd));
  }
#endif
  initscr();
#if BX_DEBUGGER_TERM
  stdin = old_stdin;
  stdout = old_stdout;
#endif
  start_color();
  cbreak();
  curs_set(1);
  keypad(stdscr,TRUE);
  nodelay(stdscr, TRUE);
  noecho();

#if BX_HAVE_COLOR_SET
  if (has_colors()) {
    for (i = 0; i < 8; i++) {
      for (int j=0; j<8; j++) {
        if ((i!=0)||(j!=0)) init_pair(i * 8 + j, j, i);
      }
    }
  }
#endif

  // parse term specific options
  if (argc > 1) {
    for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "hideIPS")) {
#if BX_SHOW_IPS
        BX_INFO(("hide IPS display in status bar"));
        termHideIPS = 1;
#else
        BX_ERROR(("Show IPS not available"));
#endif
      } else {
        BX_PANIC(("Unknown rfb option '%s'", argv[i]));
      }
    }
  }

  if (SIM->get_param_bool(BXPN_PRIVATE_COLORMAP)->get())
    BX_ERROR(("WARNING: private_colormap option ignored."));

  initialized = 1;
}

void do_char(int character, int alt)
{
  switch (character) {
    // control keys
    case   0x9: do_scan(BX_KEY_TAB,0,0,alt); break;
    case   0xa: do_scan(BX_KEY_KP_ENTER,0,0,alt); break;
    case   0xd: do_scan(BX_KEY_KP_DELETE,0,0,alt); break;
    case   0x1: do_scan(BX_KEY_A,0,1,alt); break;
    case   0x2: do_scan(BX_KEY_B,0,1,alt); break;
    case   0x3: do_scan(BX_KEY_C,0,1,alt); break;
    case   0x4: do_scan(BX_KEY_D,0,1,alt); break;
    case   0x5: do_scan(BX_KEY_E,0,1,alt); break;
    case   0x6: do_scan(BX_KEY_F,0,1,alt); break;
    case   0x7: do_scan(BX_KEY_G,0,1,alt); break;
    case   0x8: do_scan(BX_KEY_H,0,1,alt); break;
    case   0xb: do_scan(BX_KEY_K,0,1,alt); break;
    case   0xc: do_scan(BX_KEY_L,0,1,alt); break;
    case   0xe: do_scan(BX_KEY_N,0,1,alt); break;
    case   0xf: do_scan(BX_KEY_O,0,1,alt); break;
    case  0x10: do_scan(BX_KEY_P,0,1,alt); break;
    case  0x11: do_scan(BX_KEY_Q,0,1,alt); break;
    case  0x12: do_scan(BX_KEY_R,0,1,alt); break;
    case  0x13: do_scan(BX_KEY_S,0,1,alt); break;
    case  0x14: do_scan(BX_KEY_T,0,1,alt); break;
    case  0x15: do_scan(BX_KEY_U,0,1,alt); break;
    case  0x16: do_scan(BX_KEY_V,0,1,alt); break;
    case  0x17: do_scan(BX_KEY_W,0,1,alt); break;
    case  0x18: do_scan(BX_KEY_X,0,1,alt); break;
    case  0x19: do_scan(BX_KEY_Y,0,1,alt); break;
    case  0x1a: do_scan(BX_KEY_Z,0,1,alt); break;
    case  0x20: do_scan(BX_KEY_SPACE,0,0,alt); break;
    case 0x107: do_scan(BX_KEY_BACKSPACE,0,0,alt); break;
    case 0x102: do_scan(BX_KEY_DOWN,0,0,alt); break;
    case 0x103: do_scan(BX_KEY_UP,0,0,alt); break;
    case 0x104: do_scan(BX_KEY_LEFT,0,0,alt); break;
    case 0x105: do_scan(BX_KEY_RIGHT,0,0,alt); break;
    case 0x152: do_scan(BX_KEY_PAGE_DOWN,0,0,alt); break;
    case 0x153: do_scan(BX_KEY_PAGE_UP,0,0,alt); break;
    case 0x106: do_scan(BX_KEY_HOME,0,0,alt); break;
    case 0x168: do_scan(BX_KEY_END,0,0,alt); break;
    case 0x14b: do_scan(BX_KEY_INSERT,0,0,alt); break;
    case 0x7f:  do_scan(BX_KEY_DELETE,0,0,alt); break;
    case 0x1b:  do_scan(BX_KEY_ESC,0,0,alt); break;
    case '!': do_scan(BX_KEY_1,1,0,alt); break;
    case '\'': do_scan(BX_KEY_SINGLE_QUOTE,0,0,alt); break;
    case '#': do_scan(BX_KEY_3,1,0,alt); break;
    case '$': do_scan(BX_KEY_4,1,0,alt); break;
    case '%': do_scan(BX_KEY_5,1,0,alt); break;
    case '^': do_scan(BX_KEY_6,1,0,alt); break;
    case '&': do_scan(BX_KEY_7,1,0,alt); break;
    case '"': do_scan(BX_KEY_SINGLE_QUOTE,1,0,alt); break;

    // ()*+,-./
    case '(': do_scan(BX_KEY_9,1,0,alt); break;
    case ')': do_scan(BX_KEY_0,1,0,alt); break;
    case '*': do_scan(BX_KEY_8,1,0,alt); break;
    case '+': do_scan(BX_KEY_EQUALS,1,0,alt); break;
    case ',': do_scan(BX_KEY_COMMA,0,0,alt); break;
    case '-': do_scan(BX_KEY_MINUS,0,0,alt); break;
    case '.': do_scan(BX_KEY_PERIOD,0,0,alt); break;
    case '/': do_scan(BX_KEY_SLASH,0,0,alt); break;

    // 01234567
    case '0': do_scan(BX_KEY_0,0,0,alt); break;
    case '1': do_scan(BX_KEY_1,0,0,alt); break;
    case '2': do_scan(BX_KEY_2,0,0,alt); break;
    case '3': do_scan(BX_KEY_3,0,0,alt); break;
    case '4': do_scan(BX_KEY_4,0,0,alt); break;
    case '5': do_scan(BX_KEY_5,0,0,alt); break;
    case '6': do_scan(BX_KEY_6,0,0,alt); break;
    case '7': do_scan(BX_KEY_7,0,0,alt); break;

    // 89:;<=>?
    case '8': do_scan(BX_KEY_8,0,0,alt); break;
    case '9': do_scan(BX_KEY_9,0,0,alt); break;
    case ':': do_scan(BX_KEY_SEMICOLON,1,0,alt); break;
    case ';': do_scan(BX_KEY_SEMICOLON,0,0,alt); break;
    case '<': do_scan(BX_KEY_COMMA,1,0,alt); break;
    case '=': do_scan(BX_KEY_EQUALS,0,0,alt); break;
    case '>': do_scan(BX_KEY_PERIOD,1,0,alt); break;
    case '?': do_scan(BX_KEY_SLASH,1,0,alt); break;

    // @ABCDEFG
    case '@': do_scan(BX_KEY_2,1,0,alt); break;
    case 'A': do_scan(BX_KEY_A,1,0,alt); break;
    case 'B': do_scan(BX_KEY_B,1,0,alt); break;
    case 'C': do_scan(BX_KEY_C,1,0,alt); break;
    case 'D': do_scan(BX_KEY_D,1,0,alt); break;
    case 'E': do_scan(BX_KEY_E,1,0,alt); break;
    case 'F': do_scan(BX_KEY_F,1,0,alt); break;
    case 'G': do_scan(BX_KEY_G,1,0,alt); break;

    // HIJKLMNO
    case 'H': do_scan(BX_KEY_H,1,0,alt); break;
    case 'I': do_scan(BX_KEY_I,1,0,alt); break;
    case 'J': do_scan(BX_KEY_J,1,0,alt); break;
    case 'K': do_scan(BX_KEY_K,1,0,alt); break;
    case 'L': do_scan(BX_KEY_L,1,0,alt); break;
    case 'M': do_scan(BX_KEY_M,1,0,alt); break;
    case 'N': do_scan(BX_KEY_N,1,0,alt); break;
    case 'O': do_scan(BX_KEY_O,1,0,alt); break;

    // PQRSTUVW
    case 'P': do_scan(BX_KEY_P,1,0,alt); break;
    case 'Q': do_scan(BX_KEY_Q,1,0,alt); break;
    case 'R': do_scan(BX_KEY_R,1,0,alt); break;
    case 'S': do_scan(BX_KEY_S,1,0,alt); break;
    case 'T': do_scan(BX_KEY_T,1,0,alt); break;
    case 'U': do_scan(BX_KEY_U,1,0,alt); break;
    case 'V': do_scan(BX_KEY_V,1,0,alt); break;
    case 'W': do_scan(BX_KEY_W,1,0,alt); break;

    // XYZ[\]^_
    case 'X': do_scan(BX_KEY_X,1,0,alt); break;
    case 'Y': do_scan(BX_KEY_Y,1,0,alt); break;
    case 'Z': do_scan(BX_KEY_Z,1,0,alt); break;
    case '{': do_scan(BX_KEY_LEFT_BRACKET,1,0,alt); break;
    case '|': do_scan(BX_KEY_BACKSLASH,1,0,alt); break;
    case '}': do_scan(BX_KEY_RIGHT_BRACKET,1,0,alt); break;
    case '_': do_scan(BX_KEY_MINUS,1,0,alt); break;

    // `abcdefg
    case '`': do_scan(BX_KEY_GRAVE,0,0,alt); break;
    case 'a': do_scan(BX_KEY_A,0,0,alt); break;
    case 'b': do_scan(BX_KEY_B,0,0,alt); break;
    case 'c': do_scan(BX_KEY_C,0,0,alt); break;
    case 'd': do_scan(BX_KEY_D,0,0,alt); break;
    case 'e': do_scan(BX_KEY_E,0,0,alt); break;
    case 'f': do_scan(BX_KEY_F,0,0,alt); break;
    case 'g': do_scan(BX_KEY_G,0,0,alt); break;

    // hijklmno
    case 'h': do_scan(BX_KEY_H,0,0,alt); break;
    case 'i': do_scan(BX_KEY_I,0,0,alt); break;
    case 'j': do_scan(BX_KEY_J,0,0,alt); break;
    case 'k': do_scan(BX_KEY_K,0,0,alt); break;
    case 'l': do_scan(BX_KEY_L,0,0,alt); break;
    case 'm': do_scan(BX_KEY_M,0,0,alt); break;
    case 'n': do_scan(BX_KEY_N,0,0,alt); break;
    case 'o': do_scan(BX_KEY_O,0,0,alt); break;

    // pqrstuvw
    case 'p': do_scan(BX_KEY_P,0,0,alt); break;
    case 'q': do_scan(BX_KEY_Q,0,0,alt); break;
    case 'r': do_scan(BX_KEY_R,0,0,alt); break;
    case 's': do_scan(BX_KEY_S,0,0,alt); break;
    case 't': do_scan(BX_KEY_T,0,0,alt); break;
    case 'u': do_scan(BX_KEY_U,0,0,alt); break;
    case 'v': do_scan(BX_KEY_V,0,0,alt); break;
    case 'w': do_scan(BX_KEY_W,0,0,alt); break;

    // xyz{|}~
    case 'x': do_scan(BX_KEY_X,0,0,alt); break;
    case 'y': do_scan(BX_KEY_Y,0,0,alt); break;
    case 'z': do_scan(BX_KEY_Z,0,0,alt); break;
    case '[': do_scan(BX_KEY_LEFT_BRACKET,0,0,alt); break;
    case '\\': do_scan(BX_KEY_BACKSLASH,0,0,alt); break;
    case ']': do_scan(BX_KEY_RIGHT_BRACKET,0,0,alt); break;
    case '~': do_scan(BX_KEY_GRAVE,1,0,alt); break;

    // function keys
    case KEY_F(1): do_scan(BX_KEY_F1,0,0,alt); break;
    case KEY_F(2): do_scan(BX_KEY_F2,0,0,alt); break;
    case KEY_F(3): do_scan(BX_KEY_F3,0,0,alt); break;
    case KEY_F(4): do_scan(BX_KEY_F4,0,0,alt); break;
    case KEY_F(5): do_scan(BX_KEY_F5,0,0,alt); break;
    case KEY_F(6): do_scan(BX_KEY_F6,0,0,alt); break;
    case KEY_F(7): do_scan(BX_KEY_F7,0,0,alt); break;
    case KEY_F(8): do_scan(BX_KEY_F8,0,0,alt); break;
    case KEY_F(9): do_scan(BX_KEY_F9,0,0,alt); break;
    case KEY_F(10): do_scan(BX_KEY_F10,0,0,alt); break;
    case KEY_F(11): do_scan(BX_KEY_F11,0,0,alt); break;
    case KEY_F(12): do_scan(BX_KEY_F12,0,0,alt); break;

    // shifted function keys
    case KEY_F(13): do_scan(BX_KEY_F1,1,0,alt); break;
    case KEY_F(14): do_scan(BX_KEY_F2,1,0,alt); break;
    case KEY_F(15): do_scan(BX_KEY_F3,1,0,alt); break;
    case KEY_F(16): do_scan(BX_KEY_F4,1,0,alt); break;
    case KEY_F(17): do_scan(BX_KEY_F5,1,0,alt); break;
    case KEY_F(18): do_scan(BX_KEY_F6,1,0,alt); break;
    case KEY_F(19): do_scan(BX_KEY_F7,1,0,alt); break;
    case KEY_F(20): do_scan(BX_KEY_F8,1,0,alt); break;

    default:
      if(character > 0x79) {
         do_char(character - 0x80,1);
         break;
      }

      BX_INFO(("character unhandled: 0x%x",character));
      break;
  }
}

void bx_term_gui_c::handle_events(void)
{
  int character;
  while((character = getch()) != ERR) {
    BX_DEBUG(("scancode(0x%x)",character));
    do_char(character,0);
  }
}

void bx_term_gui_c::flush(void)
{
  if (initialized) refresh();
}

void bx_term_gui_c::clear_screen(void)
{
  clear();
#if BX_HAVE_COLOR_SET
  color_set(7, NULL);
#endif
#if BX_HAVE_MVHLINE
  if (LINES > (int)text_rows) {
    mvhline(text_rows, 0, ACS_HLINE, text_cols);
  }
#endif
#if BX_HAVE_MVVLINE
  if (COLS > (int)text_cols) {
    mvvline(0, text_cols, ACS_VLINE, text_rows);
  }
#endif
  if ((LINES > (int)text_rows) && (COLS > (int)text_cols)) {
    mvaddch(text_rows, text_cols, ACS_LRCORNER);
  }
}

int get_color_pair(Bit8u vga_attr)
{
  int term_attr = curses_color[vga_attr & 0x07];
  term_attr |= (curses_color[(vga_attr & 0x70) >> 4] << 3);
  return term_attr;
}

chtype get_term_char(Bit8u vga_char[])
{
  int term_char;

  if ((vga_char[1] & 0x0f) == ((vga_char[1] >> 4) & 0x0f)) {
    return ' ';
  }
  switch (vga_char[0]) {
    case 0x04: term_char = ACS_DIAMOND; break;
    case 0x18: term_char = ACS_UARROW; break;
    case 0x19: term_char = ACS_DARROW; break;
    case 0x1a: term_char = ACS_RARROW; break;
    case 0x1b: term_char = ACS_LARROW; break;
    case 0xc4:
    case 0xcd: term_char = ACS_HLINE; break;
    case 0xb3:
    case 0xba: term_char = ACS_VLINE; break;
    case 0xc9:
    case 0xd5:
    case 0xd6:
    case 0xda: term_char = ACS_ULCORNER; break;
    case 0xb7:
    case 0xb8:
    case 0xbb:
    case 0xbf: term_char = ACS_URCORNER; break;
    case 0xc0:
    case 0xc8:
    case 0xd3:
    case 0xd4: term_char = ACS_LLCORNER; break;
    case 0xbc:
    case 0xbd:
    case 0xbe:
    case 0xd9: term_char = ACS_LRCORNER; break;
    case 0xc3:
    case 0xc6:
    case 0xc7:
    case 0xcc: term_char = ACS_LTEE; break;
    case 0xb4:
    case 0xb5:
    case 0xb6:
    case 0xb9: term_char = ACS_RTEE; break;
    case 0xc2:
    case 0xcb:
    case 0xd1:
    case 0xd2: term_char = ACS_TTEE; break;
    case 0xc1:
    case 0xca:
    case 0xcf:
    case 0xd0: term_char = ACS_BTEE; break;
    case 0xc5:
    case 0xce:
    case 0xd7:
    case 0xd8: term_char = ACS_PLUS; break;
    case 0xb0:
    case 0xb1: term_char = ACS_CKBOARD; break;
    case 0xb2: term_char = ACS_BOARD; break;
    case 0xdb: term_char = ACS_BLOCK; break;
    default:
      if (vga_char[0] > 0x7f) {
        term_char = vga_to_term[vga_char[0]-0x80];
      } else if (vga_char[0] > 0x1f) {
        term_char = vga_char[0];
      } else {
        term_char = ' ';
      }
  }

  return term_char;
}

void bx_term_gui_c::text_update(Bit8u *old_text, Bit8u *new_text,
        unsigned long cursor_x, unsigned long cursor_y,
        bx_vga_tminfo_t *tm_info)
{
  unsigned char *old_line, *new_line;
  unsigned int hchars, rows, x, y;
  chtype ch;
  bool force_update = 0;

  if(charmap_updated) {
    force_update = 1;
    charmap_updated = 0;
  }

  rows = text_rows;
  y = 0;
  do {
    hchars = text_cols;
    new_line = new_text;
    old_line = old_text;
    x = 0;
    do {
      if (force_update || (old_text[0] != new_text[0])
          || (old_text[1] != new_text[1])) {
#if BX_HAVE_COLOR_SET
        if (has_colors()) {
          color_set(get_color_pair(new_text[1]), NULL);
        }
#endif
        ch = get_term_char(&new_text[0]);
        if ((new_text[1] & 0x08) > 0) ch |= A_BOLD;
        if ((new_text[1] & 0x80) > 0) ch |= A_BLINK;
        mvaddch(y, x, ch);
      }
      x++;
      new_text+=2;
      old_text+=2;
    } while (--hchars);
    y++;
    new_text = new_line + tm_info->line_offset;
    old_text = old_line + tm_info->line_offset;
  } while (--rows);

  if ((cursor_x < text_cols) && (cursor_y < text_rows)
      && (tm_info->cs_start <= tm_info->cs_end)) {
    move(cursor_y, cursor_x);
    if ((tm_info->cs_end - tm_info->cs_start) <= 2) {
      curs_set(1);
    } else {
      curs_set(2);
    }
    last_cursor_x = cursor_x;
    last_cursor_y = cursor_y;
  } else {
    curs_set(0);
    last_cursor_y = -1;
  }
}

int bx_term_gui_c::get_clipboard_text(Bit8u **bytes, Bit32s *nbytes)
{
  return 0;
}

int bx_term_gui_c::set_clipboard_text(char *text_snapshot, Bit32u len)
{
  return 0;
}

bool bx_term_gui_c::palette_change(Bit8u index, Bit8u red, Bit8u green, Bit8u blue)
{
  BX_DEBUG(("color palette request (%d,%d,%d,%d) ignored", index,red,green,blue));
  return(0);
}


void bx_term_gui_c::graphics_tile_update(Bit8u *tile, unsigned x0, unsigned y0)
{
}

void bx_term_gui_c::dimension_update(unsigned x, unsigned y, unsigned fheight, unsigned fwidth, unsigned bpp)
{
  if (bpp > 8) {
    BX_PANIC(("%d bpp graphics mode not supported", bpp));
  }
  guest_textmode = (fheight > 0);
  guest_xres = x;
  guest_yres = y;
  guest_bpp = bpp;
  if (guest_textmode) {
    text_cols = x / fwidth;
    text_rows = y / fheight;
#if BX_HAVE_COLOR_SET
    color_set(7, NULL);
#endif
#if BX_HAVE_MVVLINE
    if (COLS > (int)text_cols) {
      mvvline(0, text_cols, ACS_VLINE, text_rows);
    }
#endif
#if BX_HAVE_MVHLINE
    if (LINES > (int)text_rows) {
      mvhline(text_rows, 0, ACS_HLINE, text_cols);
      if (COLS > (int)text_cols) {
        mvaddch(text_rows, text_cols, ACS_LRCORNER);
      }
    }
    if (LINES > (int)(text_rows + 2)) {
      mvhline(text_rows + 2, 0, ACS_HLINE, text_cols);
      if (COLS > (int)text_cols) {
        mvaddch(text_rows + 1, text_cols, ACS_VLINE);
        mvaddch(text_rows + 2, text_cols, ACS_LRCORNER);
      }
#if BX_HAVE_COLOR_SET
      color_set(7 << 3, NULL);
#endif
      mvhline(text_rows + 1, 0, ' ', text_cols);
    }
#endif
  }
}

unsigned bx_term_gui_c::create_bitmap(const unsigned char *bmap, unsigned xdim, unsigned ydim)
{
  return(0);
}

unsigned bx_term_gui_c::headerbar_bitmap(unsigned bmap_id, unsigned alignment, void (*f)(void))
{
  return(0);
}

void bx_term_gui_c::show_headerbar(void)
{
}

void bx_term_gui_c::replace_bitmap(unsigned hbar_id, unsigned bmap_id)
{
}

void bx_term_gui_c::exit(void)
{
  if (!initialized) return;
#if BX_DEBUGGER_TERM
  if(scr_fd > 0)
    close(scr_fd);
#endif
  clear();
  flush();
  endwin();
  BX_DEBUG(("exiting"));
}

void bx_term_gui_c::mouse_enabled_changed_specific(bool val)
{
}

#if BX_USE_IDLE_HACK
void bx_term_gui_c::sim_is_idle()
{
  int    res;
  fd_set readfds;

  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 1000; /* 1/1000 s */

  FD_ZERO(&readfds);
  FD_SET(0, &readfds); // Wait for input

  res = select(1, &readfds, NULL, NULL, &timeout);

  switch(res)
  {
    case -1: /* select() error - should not happen */
      // This can happen when we have a alarm running, lets return
      // perror("sim_is_idle: select() failure\n");
      //handle_events();
      return;
    case  0: /* timeout */
      //handle_events();
      return;
  }
  //handle_events();
}
#endif

#if BX_SHOW_IPS
void bx_term_gui_c::show_ips(Bit32u ips_count)
{
  char ips_text[20];

  if (!termHideIPS && (LINES > (int)(text_rows + 1))) {
    ips_count /= 1000;
    sprintf(ips_text, "IPS: %u.%3.3uM ", ips_count / 1000, ips_count % 1000);
#if BX_HAVE_COLOR_SET
    color_set(7 << 3, NULL);
#endif
    mvaddstr(text_rows + 1, 0, ips_text);
    if (last_cursor_y >= 0) {
      move(last_cursor_y, last_cursor_x);
    }
  }
}

#endif

#endif /* if BX_WITH_TERM */

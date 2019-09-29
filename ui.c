#include <stdbool.h>
#include <string.h>

#include "hd44780.h"
#include "macros.h"
#include "cube.h"

void ui_writeString(const char* str, u8 pos, u8 len) {
  if (pos >= 16)
    setDDRAMAddress(0x40+pos-16);
  else
    setDDRAMAddress(pos);
  if (pos+len > 32)
    len = 32-pos;
  u8 i;
  for (i = 0; i < len; i++) {
    if (pos+i == 16)
      setDDRAMAddress(0x40);
    if (str)
      writeRAM(str[i]);
    else
      writeRAM(' ');
  }
}

void ui_writeRowPadded(const char* str, u8 row) {
  setDDRAMAddress(row ? 0x40 : 0);
  u8 len = strlen(str);
  for (u8 i=0; i<len; i++)
    writeRAM(str[i]);
  for (u8 i=len; i<16; i++)
    writeRAM(' ');
}

void ui_init() {
  BACKLIGHT_OFF;

  init();
  setMode1(1,0);
  setMode2(1,0,0);
  setMovement(0,1);
  setMode3(0,1,0);

  ui_writeRowPadded("LEDCUBE", 0);
  ui_writeRowPadded("Fabian Lesniak", 1);

  BACKLIGHT_ON;
}

static bool in_menu = false;
static u8 menu_anim_idx = 0;

static void ui_render_menu() {
  ui_writeRowPadded("Choose Animation", 0);
  ui_writeRowPadded(cube_get_anim_name(menu_anim_idx), 1);
}

static void ui_render_anim() {
  ui_writeRowPadded(cube_get_anim_name(menu_anim_idx), 0);
  // cube_get_anim_status = 0 clears text
  ui_writeRowPadded(cube_get_anim_status(), 1);
}

static void ui_render() {
  if (in_menu) {
    ui_render_menu();
  } else {
    ui_render_anim();
  }
}

static void ui_handle_buttons(bool btn_l, bool btn_r) {
  if (btn_l) {
    if (menu_anim_idx == 0)
      menu_anim_idx = cube_get_anim_count()-1;
    else
      menu_anim_idx--;
  }

  if (btn_r) {
    if (menu_anim_idx == cube_get_anim_count()-1)
      menu_anim_idx = 0;
    else
      menu_anim_idx++;
  }
}

static void ui_buttons(bool btn_l, bool btn_r) {
  if (btn_l && btn_r) {
    if (in_menu) {
      cube_set_anim(menu_anim_idx);
      in_menu = false;
    } else {
      in_menu = true;
    }
  } else {
    if (in_menu)
      ui_handle_buttons(btn_l, btn_r);
    else
      cube_buttons(btn_l, btn_r);
  }
  ui_render();
}

void ui_pollButtons() {
  // 0 -> no buttons down
  // 1 -> left button down
  // 2 -> right button down
  // 3 -> all buttons down
  // 4 -> wait for all buttons to be released
  static u8 state = 0;
  enum {none, left, right, both} action = none;

  switch (state) {
    case 0:
      if (BUTTON_L && BUTTON_R)
        state = 3;
      else if (BUTTON_L)
        state = 1;
      else if (BUTTON_R)
        state = 2;
      break;
    case 1:
      if (BUTTON_R)
        state = 3;
      else if (!(BUTTON_L && BUTTON_R)) {
        action = left;
        state = 0;
      }
      break;
    case 2:
      if (BUTTON_L)
        state = 3;
      else if (!(BUTTON_L && BUTTON_R)) {
        action = right;
        state = 0;
      }
      break;
    case 3:
      state = 4;
      break;
    case 4:
      if (!(BUTTON_L && BUTTON_R)) {
        action = both;
        state = 0;
      }
      break;
    default:
      state = 0;
  }

  if (action == both)
    ui_buttons(true, true);
  else if (action != none)
    ui_buttons(action == left, action == right);
}

void ui_pollButtons_debounced() {
  static u8 delay = 0;

  if (delay == 60) {
    ui_pollButtons();
    delay = 0;
  } else
    delay++;
}

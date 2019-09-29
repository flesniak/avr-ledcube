#include <stdbool.h>
#include <string.h>

#include "hd44780.h"
#include "macros.h"

void ui_writeString(const char* str, unsigned char pos, unsigned char len) {
  if (pos > 16)
    setDDRAMAddress(0x40+pos-16);
  else
    setDDRAMAddress(pos);
  if (pos+len > 32)
    len = 32-pos;
  unsigned char i;
  for (i = 0; i < len; i++) {
    if (pos+i == 16)
      setDDRAMAddress(0x40);
    writeRAM(str[i]);
  }
}

void ui_init() {
  BACKLIGHT_OFF;

  init();
  setMode1(1,0);
  setMode2(1,0,0);
  setMovement(0,1);
  setMode3(0,1,0);

  const char helloStr[] = "LEDCUBE         Fabian Lesniak";
  ui_writeString(helloStr, 0, strlen(helloStr));

  BACKLIGHT_ON;
}

extern unsigned char cube_interactivetest_col;
extern unsigned char cube_interactivetest_row;

void ui_leftButtonPressed() {
  if (cube_interactivetest_col == 7)
    cube_interactivetest_col = 0;
  else
    cube_interactivetest_col++;
}

void ui_rightButtonPressed() {
  if (cube_interactivetest_row == 7)
    cube_interactivetest_row = 0;
  else
    cube_interactivetest_row++;
}

void ui_pollButtons() {
  static unsigned char btnStateL = 0, btnStateR = 0;

  if (BUTTON_L) {
    if (btnStateL == 0) {
      ui_leftButtonPressed();
      btnStateL = 1;
    }
  } else
    btnStateL = 0;

  if (BUTTON_R) {
    if (btnStateR == 0) {
      ui_rightButtonPressed();
      btnStateR = 1;
    }
  } else
    btnStateR = 0;
}

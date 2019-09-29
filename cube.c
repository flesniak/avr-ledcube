#include <avr/io.h>
#include <string.h>

#include "cube.h"

unsigned short frameCounter = 0;
unsigned char currentLayer = 0; //currently multiplexed segment (equals decade)
unsigned char currentRow = 0; //currently transmitted row (if 7 after transfer, row is complete)
unsigned char currentImage[64] = {0}; //segment code to display on every segment

static unsigned char rand8() {
  static unsigned char value = 0xad;
  unsigned char tmp;

  tmp = value >> 1;
  if (value & 1)
    tmp ^= 0xB8; // x^8 + x^6 + x^5 + x^4 + 1

  value = tmp;
  return value;
}

static unsigned char cube_rain_state;

void cube_rain_init() {
  memset(currentImage, 0xff, 64);
  cube_rain_state = 0;
}

void cube_rain_run() {
  if (frameCounter < 60)
    return;
  unsigned char i;
  if (cube_rain_state == 64)
    cube_rain_state = 0;
  else
    cube_rain_state++;
  for (i=56; i!=255; i--)
    currentImage[i+8] = currentImage[i];
  for (i=0; i<8; i++)
    currentImage[i] = 0xff;
  currentImage[cube_rain_state >> 3] = ~(1 << (cube_rain_state & 7));
  frameCounter = 0;
}

unsigned char cube_interactivetest_col;
unsigned char cube_interactivetest_row;
static unsigned char cube_interactivetest_layer;

void cube_interactivetest_init() {
  cube_interactivetest_col = 0;
  cube_interactivetest_row = 0;
  cube_interactivetest_layer = 0;
}

void cube_interactivetest_run() {
  memset(currentImage, 0xff, 64);
  if (frameCounter > 60) {
    if (cube_interactivetest_layer == 7)
      cube_interactivetest_layer = 0;
    else
      cube_interactivetest_layer++;
    frameCounter = 0;
  }
  currentImage[8*cube_interactivetest_layer+cube_interactivetest_col] = ~(1 << cube_interactivetest_row);
}

void cube_init() {
  memset(currentImage, 0xff, 64);
  //cube_rain_init();
  cube_interactivetest_init();
}

// calculate the next frame if necessary
void cube_run() {
  //cube_rain_run();
  cube_interactivetest_run();
}

void single_movable_led(bool invert) {
  static unsigned char x=0, y=0, z=0, debounce=0;

  // for (unsigned char z=0; z<8; z++)
  //   currentImage[8*z+y] = invert?0:0xff;
  currentImage[8*z+y] = invert?0:0xff;

  if (BUTTON_L) {
    if (debounce==0) {
      x==7?x=0:x++;
      debounce=1;
    }
  } else {
    if (BUTTON_R) {
      if (debounce==0) {
        y==7?y=0:y++;
        debounce=1;
      }
    } else
      debounce=0;
  }

  z = z<7 ? z+1 : 0;

  // for (unsigned char z=0; z<8; z++)
  //   currentImage[8*z+y] = invert?(1<<x):~(1<<x);
  currentImage[8*z+y] = invert?(1<<x):~(1<<x);
}

void single_movable_column(bool invert) {
  static unsigned char x=0, y=0, debounce=0;

  for (unsigned char z=0; z<8; z++)
    currentImage[8*z+y] = invert?0:0xff;

  if (BUTTON_L) {
    if (debounce==0) {
      x==7?x=0:x++;
      debounce=1;
    }
  } else {
    if (BUTTON_R) {
      if (debounce==0) {
        y==7?y=0:y++;
        debounce=1;
      }
    } else
      debounce=0;
  }

  for (unsigned char z=0; z<8; z++)
    currentImage[8*z+y] = invert?(1<<x):~(1<<x);
}

void single_random_led() {
  static unsigned char x=0, y=0, z=0;
  // unsigned char rnd = rand8();

  currentImage[8*z+y] = 0xff;
  // x = rnd & 7;
  // y = (rnd>>3) & 7;
  // z = ((rnd>>6) ^ rnd) & 7;
  x = rand8() & 7;
  y = rand8() & 7;
  z = rand8() & 7;
  currentImage[8*z+y] = ~(1<<x);
}

void rain() {
  static unsigned char z=0;
  unsigned char tmp;

  // move layers down by one
  for (z=7; z>0; z--)
    memcpy(&currentImage[8*z], &currentImage[8*(z-1)], 8);

  // create raindrops on top layer
  memset(currentImage, 0xff, 8);
  for (int i=0; i<5; i++) {
    tmp = rand8();
    currentImage[tmp&7] &= ~(1<<(tmp>>4&7));
  }
}

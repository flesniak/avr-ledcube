#include <avr/io.h>
#include <string.h>

#include "cube.h"

u16 frameCounter = 0;
u8 currentLayer = 0; //currently multiplexed segment (equals decade)
u8 currentRow = 0; //currently transmitted row (if 7 after transfer, row is complete)
u8 currentImage[64] = {0}; //segment code to display on every segment

/* VERTICAL RAIN */

static u8 vertical_rain_state;

static void vertical_rain_init() {
  memset(currentImage, 0xff, 64);
  vertical_rain_state = 0;
}

static void vertical_rain_run() {
  u8 i;
  if (vertical_rain_state == 64)
    vertical_rain_state = 0;
  else
    vertical_rain_state++;
  for (i=56; i!=255; i--)
    currentImage[i+8] = currentImage[i];
  for (i=0; i<8; i++)
    currentImage[i] = 0xff;
  currentImage[vertical_rain_state >> 3] = ~(1 << (vertical_rain_state & 7));
}

static struct animation anim_vertical_rain = {
  .name = "Vertical Rain",
  .init = vertical_rain_init,
  .run = vertical_rain_run,
  .buttons = 0,
  .status = 0,
  .interval = 15
};

/* INTERACTIVETEST */

static u8 ledtest_col;
static u8 ledtest_row;
static u8 ledtest_layer;

static void ledtest_init() {
  ledtest_col = 0;
  ledtest_row = 0;
  ledtest_layer = 0;
}

static void ledtest_run() {
  memset(currentImage, 0xff, 64);

  if (ledtest_layer == 7)
    ledtest_layer = 0;
  else
    ledtest_layer++;

  currentImage[8*ledtest_layer+ledtest_col] = ~(1 << ledtest_row);
}

static void ledtest_buttons(bool btn_l, bool btn_r) {
  if (btn_l) {
    if (ledtest_col == 7)
      ledtest_col = 0;
    else
      ledtest_col++;
  }

  if (btn_r) {
    if (ledtest_row == 7)
      ledtest_row = 0;
    else
      ledtest_row++;
  }
}

static void ledtest_status(char* buf) {
  strcpy(buf, "X 0 Y 0 Z 0");
  buf[2] += ledtest_row;
  buf[6] += ledtest_col;
  buf[10] += ledtest_layer;
}

struct animation anim_ledtest = {
  .name = "Interactive Test",
  .init = ledtest_init,
  .run = ledtest_run,
  .buttons = ledtest_buttons,
  .status = ledtest_status,
  .interval = 60
};

/* PLANES */

static u8 planes_dir;
static u8 planes_idx;

static void planes_init() {
  planes_dir = 0;
  planes_idx = 0;
}

static void planes_run() {
  memset(currentImage, 0xff, 64);

  if (planes_idx == 7)
    planes_idx = 0;
  else
    planes_idx++;

  switch (planes_dir) {
    case 0: // X direction
      memset(currentImage+8*planes_idx, 0, 8);
      break;
    case 1: // Y direction
      for (u8 i=0; i<8; i++)
        currentImage[8*i+planes_idx] = 0;
      break;
    case 2: // Z direction
      for (u8 i=0; i<64; i++)
        currentImage[i] = ~(1<<planes_idx);
      break;
  }
}

static void planes_buttons(bool btn_l, bool btn_r) {
  if (btn_l) {
    if (planes_dir == 2)
      planes_dir = 0;
    else
      planes_dir++;
  }

  if (btn_r) {
    if (planes_idx == 7)
      planes_idx = 0;
    else
      planes_idx++;
  }
}

static void planes_status(char* buf) {
  strcpy(buf, "X Planes 0");
  buf[0] += planes_dir;
  buf[10] += planes_idx;
}

struct animation anim_planes = {
  .name = "Moving Planes",
  .init = planes_init,
  .run = planes_run,
  .buttons = planes_buttons,
  .status = planes_status,
  .interval = 60
};

/* RANDOM RAIN */

static u8 rand8() {
  static u8 value = 0xa6;
  u8 tmp;

  tmp = value >> 1;
  if (value & 1)
    tmp ^= 0xB8; // x^8 + x^6 + x^5 + x^4 + 1

  value = tmp;
  return value;
}

static void random_rain_init() {
  memset(currentImage, 0xff, 64);
}

static void random_rain_run() {
  static u8 z=0;
  u8 tmp;

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

struct animation anim_random_rain = {
  .name = "Random Rain",
  .init = random_rain_init,
  .run = random_rain_run,
  .buttons = 0,
  .status = 0,
  .interval = 5
};

/* GENERIC CUBE FUNCTIONS */

static struct animation* animations[] = {
  &anim_random_rain,
  &anim_vertical_rain,
  &anim_planes,
  &anim_ledtest,
};
static struct animation* current_anim;

void cube_init() {
  memset(currentImage, 0xff, 64);
  cube_set_anim(0);
}

void cube_run() {
  if (frameCounter >= current_anim->interval) {
    current_anim->run();
    frameCounter = 0;
  }
}

void cube_set_anim(u8 idx) {
  if (idx >= cube_get_anim_count())
    return;
  current_anim = animations[idx];
  current_anim->init();
}

const char* cube_get_anim_name(u8 idx) {
  if (idx >= cube_get_anim_count())
    return 0;
  return animations[idx]->name;
}

static char* cube_status_buf;
const char* cube_get_anim_status() {
  if (!current_anim->status)
    return 0;
  current_anim->status(cube_status_buf);
  return cube_status_buf;
}

u8 cube_get_anim_count() {
  return sizeof(animations)/sizeof(animations[0]);
}

void cube_buttons(bool btn_l, bool btn_r) {
  if (current_anim->buttons)
    current_anim->buttons(btn_l, btn_r);
}


/* UNUSED */

void single_movable_led(bool invert) {
  static u8 x=0, y=0, z=0, debounce=0;

  // for (u8 z=0; z<8; z++)
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

  // for (u8 z=0; z<8; z++)
  //   currentImage[8*z+y] = invert?(1<<x):~(1<<x);
  currentImage[8*z+y] = invert?(1<<x):~(1<<x);
}

void single_movable_column(bool invert) {
  static u8 x=0, y=0, debounce=0;

  for (u8 z=0; z<8; z++)
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

  for (u8 z=0; z<8; z++)
    currentImage[8*z+y] = invert?(1<<x):~(1<<x);
}

void single_random_led() {
  static u8 x=0, y=0, z=0;
  // u8 rnd = rand8();

  currentImage[8*z+y] = 0xff;
  // x = rnd & 7;
  // y = (rnd>>3) & 7;
  // z = ((rnd>>6) ^ rnd) & 7;
  x = rand8() & 7;
  y = rand8() & 7;
  z = rand8() & 7;
  currentImage[8*z+y] = ~(1<<x);
}

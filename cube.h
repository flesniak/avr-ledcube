#ifndef CUBE_H
#define CUBE_H

#include "macros.h"

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

struct animation {
    char name[16];
    void (*init)();
    void (*run)();
    void (*buttons)(bool,bool);
    void (*status)(char*);
    u8 interval; // frames at 60 fps
};

void cube_init();
void cube_run();
void cube_buttons(bool btn_l, bool btn_r);
void cube_set_anim(u8 idx);
const char* cube_get_anim_name(u8 idx);
const char* cube_get_anim_status();
u8 cube_get_anim_count();

#endif //CUBE_H

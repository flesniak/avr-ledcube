#ifndef CUBE_H
#define CUBE_H

#include "macros.h"

#include <avr/io.h>
#include <stdbool.h>

void cube_init();
void cube_run();

extern unsigned char cube_interactivetest_col;
extern unsigned char cube_interactivetest_row;

#endif //CUBE_H

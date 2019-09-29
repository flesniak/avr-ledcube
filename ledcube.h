#ifndef LEDCUBE_H
#define LEDCUBE_H

#include "macros.h"

extern unsigned short frameCounter;

extern unsigned char currentLayer;
extern unsigned char currentRow;
extern unsigned char currentImage[];

static inline void cube_frameInterrupt() {
  if (currentRow != 7) { //spi was too slow!
    ; //TODO error signaling
  }
  if (currentLayer == 7) {
    frameCounter++;
    currentLayer = 0;
  } else
    currentLayer++;
  currentRow = 1; //row 0 is transferred by this function
  SPDR = currentImage[8*currentLayer+0];
}

static inline void cube_rowInterrupt() {
  if (currentRow == 8) { //layer complete, shift layer driver
    RCK_ON;
    PORTC = currentLayer | (PORTC & ~7);
    RCK_OFF;
  } else { //layer not complete yet, shift next row
    SPDR = currentImage[8*currentLayer+currentRow];
    currentRow++;
  }
}

#endif //LEDCUBE_H

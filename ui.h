#ifndef UI_H
#define UI_H

void ui_writeString(const char* str, unsigned char pos, unsigned char len);

void ui_init();

void ui_leftButtonPressed();

void ui_rightButtonPressed();

void ui_pollButtons();

#endif //UI_H

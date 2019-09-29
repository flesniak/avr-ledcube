#ifndef UI_H
#define UI_H

void ui_writeString(const char* str, unsigned char pos, unsigned char len);

void ui_init();

void ui_pollButtons();

void ui_pollButtons_debounced();

#endif //UI_H

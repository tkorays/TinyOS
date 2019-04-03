#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

void wait_KBC_sendready(void);
void init_keyboard(void);
void inthandler21(int* esp);

#endif /* __KEYBOARD_H__ */

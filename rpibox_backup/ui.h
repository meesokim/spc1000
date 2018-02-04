/*
 * ui.h
 *
 * user interface interface
 */

/* $Id: ui.h,v 1.11 2000/06/29 01:06:05 nyef Exp $ */

#ifndef UI_H
#define UI_H

#include <circle/types.h>

/* debug console interface */

void deb_printf(const char *fmt, ...);


/* timeslicer interface */

void set_timeslice(void (*proc)(void *), void *data);
void unset_timeslice(void);


/* shutdown callback */

typedef void (*shutdown_t)(void);
extern shutdown_t dn_shutdown;


/* joypad interface */

/*
 * joypad buttons are: UP, DOWN, LEFT, RIGHT, whatever,
 *    START (opt), SELECT (opt)
 */

struct joypad_button_template {
    int num_buttons;
    unsigned long buttons[0];
};

struct joypad {
    const struct joypad_button_template *button_template;
    unsigned long data;
};

int ui_register_joypad(struct joypad *pad);
void ui_update_joypad(struct joypad *pad);

/* keypad interface */

struct keypad {
    u16 data;
};

int keypad_register(struct keypad *pad);
void keypad_update(struct keypad *pad);

#define KPD_0 0x0001
#define KPD_1 0x0002
#define KPD_2 0x0004
#define KPD_3 0x0008
#define KPD_4 0x0010
#define KPD_5 0x0020
#define KPD_6 0x0040
#define KPD_7 0x0080
#define KPD_8 0x0100
#define KPD_9 0x0200
#define KPD_STAR 0x0400
#define KPD_HASH 0x0800
#define KPD_A 0x1000 /* Unused */
#define KPD_B 0x2000 /* Unused */
#define KPD_C 0x4000 /* Unused */
#define KPD_D 0x8000 /* Unused */

#endif /* UI_H */

/*
 * $Log: ui.h,v $
 * Revision 1.11  2000/06/29 01:06:05  nyef
 * moved menu interface out from ui.h to menu.h
 *
 * Revision 1.10  2000/06/29 00:57:16  nyef
 * fixed redundant include guards (not supposed to start with underbars)
 *
 * Revision 1.9  2000/06/25 19:22:47  nyef
 * added ui_{en,dis}able_item() for controling menu items
 *
 * Revision 1.8  2000/06/25 18:57:19  nyef
 * added support for parameters with menu callbacks
 * added support for changing the label on a menu
 *
 * Revision 1.7  2000/06/25 17:20:14  nyef
 * added per-driver menu interface
 *
 * Revision 1.6  2000/05/07 00:24:12  nyef
 * changed joypad interface to not break on C++ compilers
 *
 * Revision 1.5  2000/01/01 03:22:52  nyef
 * added preliminary keypad interface
 *
 * Revision 1.4  1999/06/05 02:42:04  nyef
 * added new joypad interface
 *
 * Revision 1.3  1999/04/17 20:11:27  nyef
 * changed shutdown() to dn_shutdown().
 *
 * Revision 1.2  1998/12/21 02:58:13  nyef
 * added a shutdown callback.
 *
 * Revision 1.1  1998/08/02 04:13:25  nyef
 * Initial revision
 *
 */

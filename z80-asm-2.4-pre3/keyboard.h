extern unsigned char  no_key_code, unknown_code;
extern unsigned char  char_map[256];
extern unsigned char  key_map[256];

extern int init_keyboard_map(char *keyboardmapfile);
extern void keystrobe(unsigned char *byte);

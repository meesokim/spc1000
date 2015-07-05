
void osc_init(void);
void uart_init(void);
void putch(unsigned char a);
void puts(unsigned char str[]);
void puthex(unsigned char a);
void puthexshort(unsigned short a);
unsigned short getshort(void);
void putshort(unsigned short data);

unsigned char getch(void);
unsigned char keyhit(void);
void putdecimal(unsigned short data);
unsigned char tochar(unsigned char a);

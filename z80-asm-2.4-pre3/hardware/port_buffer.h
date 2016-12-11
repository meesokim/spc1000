#ifndef  __PORT_BUFFER_H
#define  __PORT_BUFFER_H

#include "../z80-global"


extern int init_port_buffer(void);
#ifndef  OLD_STYLE
extern int close_port_buffer(void);
#endif

extern void send_pulse_to_port_buffer(void);
extern int port_access(_uchar id, _uchar *data, bit write_enabled);
extern void info_port_io(unsigned *port_ticks, char *last_io_port);

#endif

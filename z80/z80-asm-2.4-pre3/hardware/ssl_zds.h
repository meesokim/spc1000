#ifndef __SSL__ZDS_H
#define __SSL__ZDS_H

#include "../z80-global"

extern void	trigger_ssl(void);
extern void	reset_ssl_trigger(void);
extern void	set_ssl_trigger(void);
extern bool is_ssl_port(unsigned char port);

#endif

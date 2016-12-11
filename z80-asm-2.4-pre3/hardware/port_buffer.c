#include <stdio.h>
#include "../z80-cpu.h"
#include "../z80-mon.h"

#ifndef  OLD_STYLE
static  FILE  *ports;
static  char  buffer[256];
static  unsigned  last_access = ~0;
static  char  last_port;
#endif

int
init_port_buffer(void)
{  
#ifndef  OLD_STYLE
   ports= fopen(Z80_PORTS,"r+b");   /* read_writable_binary file */
   if (ports)
      if (256 != fread(buffer,1,256,ports))
      {  fclose(ports), ports= (FILE*)0;  }
   return !ports;
#else
   return 0;
#endif
}


#ifndef  OLD_STYLE
int close_port_buffer(void)
{
   return  ports ? fclose(ports) : 0;
}
#endif


void send_pulse_to_port_buffer(void)
{
#ifndef  OLD_STYLE
static unsigned  cycle_counter=0;
   if (!ports || !cpu_pin[iorq])
      ;
   else if (cpu_pin[wr])
   {  int  id= ADDRESS&255;
#ifndef  NO_EXTERN_PORTFILE_WRITE
      if (buffer[id] != DATA)
#endif
      {  int  i;
         i=fseek(ports,(long)id,SEEK_SET);
         if (i || 1 != fwrite(&DATA,1,1,ports))
            error(i,"hardware malfunction","port write:");
         fflush(ports);
         buffer[id]=DATA;
      }
      last_access=cycle_counter;
      last_port= id;
   }
   else if (cpu_pin[rd])
   {  int  id= ADDRESS&255;
#ifndef  NO_EXTERN_PORTFILE_WRITE
      int  i;
      fflush(ports);
      i=fseek(ports,(long)id,SEEK_SET);
      if (i || 1 != fread(&DATA,1,1,ports))
         error(i,"hardware malfunction","port read:");
      buffer[id]=DATA;
#endif
      DATA=buffer[id];
      last_access=cycle_counter;
      last_port= id;
   }
cycle_counter++;
#endif
}


int  port_access(_uchar id, _uchar *data, bit write_enabled)
{
   if (cpu_pin[iorq])   /* now CPU may NOT access IO-Ports */
      return  1;
   if (write_enabled)
   {  if (ports)
      {  int i=fseek(ports,(long)id,SEEK_SET);
         if (i || 1 != fwrite(data,1,1,ports))
            error(i,"hardware malfunction","port write:");
         fflush(ports);
      }
      buffer[id]= *data;
   }
   else
   {
#ifndef  NO_EXTERN_PORTFILE_WRITE
      int  i;
      if (ports)
      {  fflush(ports);
         i=fseek(ports,(long)id,SEEK_SET);
         if (i || 1 != fread(data,1,1,ports))
            error(i,"hardware malfunction","port read:");
         buffer[id]=*data;
      }
#endif
      *data= buffer[id];
   }
   return  0;
}


void  info_port_io(unsigned *port_ticks, char *last_io_port)
{
   *port_ticks= last_access;
   *last_io_port= last_port;
   return;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "z80-cpu.h"
#include "z80-mon.h"

_uchar  *io_address;
static _uchar  memory[1<<16];

#define  MEMORY_INIT_BYTE  0x0  /* also used for accessing unreadable memory */

static unsigned char  empty = MEMORY_INIT_BYTE;

struct port_map {
   bit  virtuel;
   unsigned short offset;
   short  port_no;
   unsigned char mask;
   unsigned short count;
   unsigned char *bankids;
} ;

static struct {
   unsigned char  type;   /* type=='-' means virtuel bank */
   unsigned short log2_size;
   unsigned char  *back_up;
   unsigned char  mapped_from;  /* relevant for virtuel bank only */
} bank[256];

static unsigned  no_maps, delta;
static bit  all_readable;
struct port_map *map;
static unsigned char  **address_map; /* map of bank_id's in current memory */
static unsigned char  *mapped_from;  /* reverse map of bank_id's */
static unsigned long  access_type;   /* holds two bits for each possible bank */


#ifdef  NEEDED
static void
set_bank_port(unsigned char port_id, unsigned char value)
{
#ifdef  OLD_STYLE
   FILE *fp;
   if (!(fp=fopen(Z80_PORTS,"r+b")))
      return;
   fseek(fp,port_id,SEEK_SET);
   fwrite(&value,1,1,fp);
   fclose(fp);
#endif
}
#endif

#define  MAX_NEST  2  /* unbelievable hardware if greater than 2 **/

static unsigned char *
update_all_adress_maps(unsigned char bank_id)
{
   unsigned  i, j, t=0;
   unsigned char  id, end=0; /* to prevent compiler warning uninitialization */
   bool  first=1;
   for (i=1<<16-delta;i--;)
   {  bool update=0;
      if (address_map[i] == memory)  continue;
      id=mapped_from[i];
      for (j=0; bank[id].type=='-' && (!first || j < MAX_NEST); j++)
      {  if (id == bank_id)
         {  update= 1;
            if (!first)  break;
         }
         id= bank[id].mapped_from;
      }
      if (update)
      {
         if (!first)
            address_map[i]= bank[end=id].back_up;
         else
            address_map[i]= bank[end].back_up;
         i *= 2;
         access_type &= ~(3<<i);
         if (!first)
            access_type |= t<<i;
         else if (bank[id].type == '+')
            t=0;
         else if (bank[id].type == 'r')
            access_type |= (t=2)<<i;
         else if (bank[id].type == 'w')
            access_type |= (t=1)<<i;
         else
            access_type |= (t=3)<<i;
         first=0;
      }
      else if (bank[id].type=='-')
         return (unsigned char *)0;
   }
   return  first ? (unsigned char *)0 : bank[end].back_up;
}
#undef  MAX_NEST


static void
bank_switch(unsigned i, unsigned char id)
{
   if (map[i].virtuel)
   {  unsigned char  bank_id= map[i].offset;
      if (bank[bank_id].back_up=update_all_adress_maps(bank_id))
         bank[bank_id].mapped_from= id;
   }
   else
   {
      unsigned  index = map[i].offset>>delta;
#ifdef  NEEDED
      unsigned j;
      for (j=0;j<no_maps;j++)
         if (!map[j].virtuel && map[i].offset == map[j].offset)
         {  if (i != j && map[j].port_no >= 0)
    /* different ports have equal offset!! */
               set_bank_port((unsigned char)map[j].port_no,id);
         }
#endif
      mapped_from[index]= id;
      address_map[index] = bank[id].back_up;
      index *= 2;
      access_type &= ~(3<<index);
      if (bank[id].type == '+')
         ;
      else if (bank[id].type == 'r')
         access_type |= 2<<index;
      else if (bank[id].type == 'w')
         access_type |= 1<<index;
      else
         access_type |= 3<<index;
   }
}


void  set_default_byte(_uchar val)
{
   empty= val;
}


void
switch_bank(_uchar val, unsigned map_no)
{
   unsigned i, j;
   unsigned short portno;
   if (map_no > no_maps || !map_no || map[map_no-1].port_no < 0)
      return;
   portno= map[map_no-1].port_no;
   for (i=0;i<no_maps;i++)
      if (portno == map[i].port_no)
      {  unsigned char  id= val & map[i].mask;
         for (j=0;j<map[i].count;j++)
            if (id == *(map[i].bankids+j))
               bank_switch(i, id);
      }
}


unsigned
bank_port_index(unsigned char id)
{
   unsigned i;
   for (i=0;i<no_maps;i++)
      if (map[i].port_no == id)
         return i+1;
   return 0;
}


void
reset_banks(void)
{
   unsigned  i;
   if (!no_maps)  return;
   for (i= 1<<(16-delta); i-- ; )
      address_map[i]= memory;
   for (i=0;i<no_maps;i++)
   {  if (!i || map[i].offset != map[i-1].offset)
         bank_switch(i,*(map[i].bankids));
   }
}


void dealloc_banks_and_maps(void)
{
   unsigned  i;
   for (i=0;i<256;i++)
      if (bank[i].type  &&  bank[i].back_up)
         free(bank[i].back_up);
   for (i=0;i<no_maps;i++)
      free(map[i].bankids);
   free(map);
   free(address_map);
   free(mapped_from);
   no_maps=0;
}


#define  ERROR_DESCR "desription ignored in line %u:"
#define  ERROR_BANK  "bank desription ignored in line %u:"
#define  ERROR_MAP   "map description ignored in line %u:"
int
init_banks(char *rom_path, char *bank_mapping_descr)
{
char buffer[800], filename[1024], err_line[64];
unsigned  k, b, pre_id=0, pre_off=0;
short  pre_port= -1;
FILE *fp;
if (!bank_mapping_descr || !*bank_mapping_descr)
{  sprintf(err_line,"no bank_description file given");
   error(0,err_line,"");
   return -1;
}
else if (!(fp = fopen(bank_mapping_descr,"r")))
{  sprintf(err_line,"can't open bank_description file: ");
   error(0,err_line,bank_mapping_descr);
   return -2;
}
delta=0;
no_maps=0;
map= (struct port_map *) 0;
all_readable=1;
if (rom_path && *rom_path)
{  strcpy(filename,rom_path);
   strcat(filename,"/");
   b=strlen(filename);
}
else
   b=0;
for (k=1; fgets(buffer,800,fp) ; k++)
{
   char  tok1[5], tok2[5], tok3[5], tok4[512];
   unsigned  i, offset, mask, id, log2; 
   FILE *rom_fp;
   short portno;

   for (i=0;buffer[i] && buffer[i] < ' '; i++);
   if (!buffer[i]  || buffer[i]=='#')  continue;

   if (4 != sscanf(buffer,"%4s %4s %4s %511s",tok1,tok2,tok3,tok4))
   {  sprintf(err_line,ERROR_DESCR,k);
      error(0,err_line," not 4 entries or entry too long");
      continue;
   }
   if (strlen(tok1) != 1 && !delta)
   {  sprintf(err_line,ERROR_DESCR,k);
      error(0,err_line," bank description expected");
      continue;
   }
   if (strlen(tok1) == 1 && no_maps)
   {  sprintf(err_line,ERROR_DESCR,k);
      error(0,err_line," map description expected");
      continue;
   }
   if (strlen(tok1) == 1)  /* bank description line */
   {  
      if (tok1[0] != '+' && tok1[0] != 'w' && tok1[0] != 'r' && tok1[0] != '-')
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," invalid access_type");
         continue;
      }
      if (1 != sscanf(tok2,"%x",&id) || id >= 256)
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," invalid bank_id");
         continue;
      }
      if (bank[id].type)
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," bank_id already defined");
         continue;
      }
      if (delta && id < pre_id)
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," bank_id not ascending sorted");
         continue;
      }
      if (1 != sscanf(tok3,"%u",&log2) || (log2 != 12 && log2 != 14))
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," invalid log2_size");
         continue;
      }
      if (delta && log2 != delta)
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," different bank sizes");
         continue;
      }
      strcpy(filename+b,tok4);
      if (tok4[0] == '-' && tok4[1] == '\0')
         rom_fp = NULL;
      else if (tok1[0]=='-')
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," virtuel bank has bank file");
         continue;
      }
      else if (!(rom_fp = fopen(filename,"r")))
      {  char  err_msg[64];
         sprintf(err_line,ERROR_BANK,k);
         sprintf(err_msg," can't open bank file %63s", filename);
         error(0,err_line,err_msg);
         continue;
      }
      if (!(bank[id].back_up= (unsigned char *) malloc(1<<log2)))
      {  sprintf(err_line,ERROR_BANK,k);
         error(0,err_line," insuffient memory");
         continue;
      }
      if (rom_fp)
      {  fread(bank[id].back_up,1,1<<log2,rom_fp);
         fclose(rom_fp);
      }
      delta=log2;
      bank[id].log2_size = 1<<log2;
      bank[id].type = tok1[0];
      pre_id=id;
      if (tok1[0] == 'w')
         all_readable=0;
   }
   else  /* page description line */
   {  unsigned  h, j;
      char *p;
      struct port_map *mmm;
      for (i=0;i<5;i++)
         if (tok1[i] >= '0' && tok1[i] <= '9')  continue;
         else if (tok1[i] >= 'a' && tok1[i] <= 'f')  continue;
         else if (tok1[i] >= 'A' && tok1[i] <= 'F')  continue;
         else
            break;
      if (i != 2 && i != 4)
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," invalid offset or bank_id");
         continue;
      }
      if (i == 2 && bank[i].type != '-')
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," offset is no virtuel bank_id");
         continue;
      }
      sscanf(tok1,"%x",&offset);
      if (no_maps && offset < pre_off)
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," offset/bank_id not ascending sorted");
         continue;
      }
      if (i==4 && (offset & (1<<delta)-1))
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," offset not multiple of bank size");
         continue;
      }
      if (tok2[0] == '-' && !tok2[1])
      {  portno= -1;
         if (i==2)
         {  sprintf(err_line,ERROR_MAP,k);
            error(0,err_line," map to virtuel bank without port");
            continue;
         }
      }
      else
      {  portno = strtoul(tok2,&p,0);
         if (portno >= 256 || p && *p || tok2[0]=='-' || tok2[0]=='+')
         {  sprintf(err_line,ERROR_MAP,k);
            error(0,err_line," invalid port_id");
            continue;
         }
      }
      if (no_maps && offset == pre_off && portno < pre_port)
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," offset/port_no not ascending sorted");
         continue;
      }
      for (j=0;j<2;j++)
         if (tok3[j] >= '0' && tok3[j] <= '9')  continue;
         else if (tok3[j] >= 'a' && tok3[j] <= 'f')  continue;
         else if (tok3[j] >= 'A' && tok3[j] <= 'F')  continue;
         else
            break;
      if (j != 2)
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," invalid mask");
         continue;
      }
      sscanf(tok3,"%x",&mask);
      for (j=0;tok4[j];j++)
         if (j%3 == 2 && tok4[j]!=',')
            break;
      if (tok4[j] || j%3 != 2)
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," invalid format of bank_ids");
         continue;
      }
      h= (j+1)/3;
      pre_id= 0;
      for (j=0;j<h;j++)
      {  id = strtoul(tok4+3*j,&p,16);
         if (id >= 256 || (p && *p && *p != ',') || tok4[3*j]=='-' || tok4[3*j]=='+')
         {  char  err_msg[32];
            sprintf(err_line,ERROR_MAP,k);
            sprintf(err_msg," invalid %u-th bank_id: %3s",j+1,tok4+3*j);
            error(0,err_line,err_msg);
            break;
         }
         if (j && id <= pre_id)
         {  sprintf(err_line,ERROR_MAP,k);
            error(0,err_line," bank_ids not ascending ordered");
            break;
         }
         if (!bank[id].type)
         {  char  err_msg[32];
            sprintf(err_line,ERROR_MAP,k);
            sprintf(err_msg," bank_id %u not defined",id);
            error(0,err_line,err_msg);
            break;
         }
         pre_id=id;
      }
      if (j < h)  continue;
      if (!(mmm= (struct port_map *)realloc(map,(no_maps+1)*sizeof(struct port_map))))
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," insufficent memory");
         continue;
      }
      map=mmm;
      map[no_maps].virtuel= (i==2);
      map[no_maps].offset= offset;
      pre_off=offset;
      map[no_maps].port_no = portno;
      pre_port=portno;
      map[no_maps].mask = mask;
      map[no_maps].count = h;
      if (!(map[no_maps].bankids= (unsigned char*)malloc(h*sizeof(_uchar))))
      {  sprintf(err_line,ERROR_MAP,k);
         error(0,err_line," insufficent memory");
         continue;
      }
      for (j=0;j<h;j++)
      {  sscanf(tok4+3*j,"%2x",&id);
         *(map[no_maps].bankids+j) = id;
      }
      no_maps++;
   }
}
fclose(fp);
if (no_maps)
{  if (!(address_map = (unsigned char **) malloc(sizeof(unsigned char*)<<(16-delta))))
   {  error(0,"init_banks","bank mapping: insufficent memory");
      dealloc_banks_and_maps();
   }
   else if (!(mapped_from = (unsigned char *) malloc(sizeof(unsigned char)<<(16-delta))))
   {  error(0,"init_banks","bank mapping: insufficent memory");
      dealloc_banks_and_maps();
   }
   reset_banks();
}
return  no_maps;
}
#undef  ERROR_DESCR
#undef  ERROR_MAP
#undef  ERROR_BANK


_uchar memory_at(unsigned short index)
{
   return !no_maps ? memory[index] :
          !all_readable && (access_type>>(2*(index>>delta))&1) ?
          empty : *(address_map[index>>delta]+index) ;
}


_uchar read_memo(unsigned short index)
{
   if (cpu_pin[busrq]) acknowledge_bus_request();
   if(!cpu_is_in_disassemble) ADDRESS=index;
   set_cpu_pin(rd,1);
   set_cpu_pin(mreq,1);
   if(!cpu_is_in_disassemble) wait_tics(TICS_MEMO);
   DATA= (!no_maps ? memory[index] :
          !all_readable && (access_type>>(2*(index>>delta))&1) ?
          empty : *(address_map[index>>delta]+index)) ;
   set_cpu_pin(mreq,0);
   set_cpu_pin(rd,0);
   return  DATA;
}


_uchar read_opcode(unsigned short index, bool set_m1)
{
   if (cpu_pin[busrq]) acknowledge_bus_request();
   if (set_m1)  set_cpu_pin(m1,1);
   if(!cpu_is_in_disassemble) ADDRESS=index;
   set_cpu_pin(rd,1);
   if (io_address)
   {  
      DATA= *(io_address+index);
      if(!cpu_is_in_disassemble) wait_tics(TICS_MEMO); 
   }
   else
   {
      set_cpu_pin(mreq,1);
      DATA= (!no_maps ? memory[index] :
             !all_readable && (access_type>>(2*(index>>delta))&1) ?
             empty : *(address_map[index>>delta]+index)) ;
      if(!cpu_is_in_disassemble) wait_tics(TICS_MEMO); 
      set_cpu_pin(mreq,0);
   }
   set_cpu_pin(rd,0);
   if (set_m1)  set_cpu_pin(m1,0);
   return  DATA;
}


void write_memo(unsigned short index, unsigned char data)
{
   if (cpu_pin[busrq]) acknowledge_bus_request();
   if(!cpu_is_in_disassemble) ADDRESS=index;
   DATA=data;
   set_cpu_pin(wr,1);
   set_cpu_pin(mreq,1);
   if(!cpu_is_in_disassemble) wait_tics(TICS_MEMO);
   if (!no_maps)
      memory[index]=DATA;
   else if (!(access_type>>(2*(index>>delta))&2))
      *(address_map[index>>delta]+index)=DATA;
   set_cpu_pin(mreq,0);
   set_cpu_pin(wr,0);
}


_uchar write_to_memory(_ushort index, _uchar data)
{
   _uchar  previous;
   set_cpu_pin(wr,1);
   if (io_address)
   {  previous= *(io_address+index);
      *(io_address+index) = data;
   }
   else
   {
      set_cpu_pin(mreq,1);
      previous= (!no_maps ? memory[index] :
                 !all_readable && (access_type>>(2*(index>>delta))&1) ?
                 empty : *(address_map[index>>delta]+index)) ;
      if (!no_maps)
         memory[index]= data;
      else if (!(access_type>>(2*(index>>delta))&2))
         *(address_map[index>>delta]+index)= data;
      set_cpu_pin(mreq,0);
   }
   set_cpu_pin(wr,0);
   return  previous;
}


void
clear_memory(void)
{
  int  i;
  if (!no_maps)
     memset(memory,empty,65536);
  else
     for (i=0;i<16;i++)
        if (!(access_type>>(2*i)&2))
           memset(memory+(i<<delta),empty,1<<delta);
}


unsigned  dma_write(unsigned short offset, unsigned count, FILE *from)
{
   return  fread(memory+offset,1,count+offset<65536U?count:65536U-offset,from);
}


unsigned  dma_read(unsigned short offset, unsigned count, FILE *to)
{
   return  fwrite(memory+offset,1,count+offset<65536U?count:65536U-offset,to);
}

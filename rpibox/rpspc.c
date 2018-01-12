#define BCM2708_PERI_BASE        0x3F000000 // Raspberry Pi 2
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
  
#include <stdio.h>
#include <stdlib.h>  
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <bcm2835.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;
 
// I/O access
volatile unsigned *gpio;
volatile unsigned *gpio10;
volatile unsigned *gpio7;
volatile unsigned *gpio13;
volatile unsigned *gpio1;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET *(gpio7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio10) // clears bits which are 1 ignores bits which are 0
 
#define GET_GPIO(g) (*(gpio13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
#define GPIO (*(gpio13))
 
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

#define SET_OUTPUT(g) {INP_GPIO(g); OUT_GPIO(g);}
#define SET_INPUT(g)  INP_GPIO(g)
#define SET_CLOCK(g)  INP_GPIO(g); ALT0_GPIO(g)

#define RPSPC_D0	(1 << 0)
#define MD00_PIN	0
#define RPSPC_WR	(1 << 14)
#define RPSPC_RD	(1 << 15)
#define RPSPC_EXT	(1 << 18)
#define RPSPC_RST   (1 << 17)
#define RPSPC_CLK	(1 << 27)
#define RPSPC_A0	(1 << 22)
#define RPSPC_A1	(1 << 23)
#define RPSPC_A11	(1 << 24)
#define RPSPC_A0_PIN 22

#define ADDR  (RPSPC_A0 | RPSPC_A1)
#define ADDR0 0
#define ADDR1 RPSPC_A0
#define ADDR2 RPSPC_A1


void setup_io();
void clear_io();
void setDataIn();
void setDataOut();

void setup_io() 
{
	int i;
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }
 
   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );
 
   close(mem_fd); //No need to keep mem_fd open after mmap
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }
 
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
   gpio10 = gpio+10;
   gpio7 = gpio+7;
   gpio13 = gpio+13;
   gpio1 = gpio+1;
   for (i = 0; i < 32; i++)
	SET_OUTPUT(i);
   GPIO_SET = 0xffffffff;

   
} // setup_io

void clear_io() 
{
	
}

#define SDINIT		0
#define SDWRITE		1
#define SDREAD		2
#define SDSEND  	3
#define SDCOPY		4
#define SDFORMAT	5
#define SDSTATUS	6
#define SDDRVSTS	7
#define SDRAMTST	8
#define SDTRANS2	9
#define SDNOACT		10
#define SDTRANS1	11
#define SDRCVE		12
#define SDGO		13
#define SDLOAD		14
#define SDSAVE		15
#define SDLDNGO		16

#define READY 			0
#define READ_FOR_DATA  	1
#define DATA_VALID 		2
#define RECEIVED		3
#define rATN 0x80
#define rDAC 0x40
#define rRFD 0x20
#define rDAV 0x10
#define wDAC 0x04
#define wRFD 0x02
#define wDAV 0x01

char* fdd[3];

int main(int argc, char **argv)
{
  unsigned int a, pAddr, addr;
  int i = 0, p = 0, res = 0, rw, pmode;
  int mode = 0;
  unsigned char buffer[256*256], cmd, data, buf, value;
  unsigned char *tmpbuf;
  int cmd_params[] = {0,4,4,0,7,1,1,0,4,4,0,4,4,2,6,6,0};
  int param[10];
  char f[] = {0x11,0x7,0xcb,0xcd,0xf3,0x07,0xc9,0x48,0x65,0x6c,0x6c,0x20,0x77,0x6f,0x72,0x6c,0x64,0x21,0x00};
  fdd[0] = f;
  setup_io();
  GPIO_SET = data = res;
	GPIO_CLR = 0xff;
	mode = READY;
  while(1)
  {

   	  a = GPIO;
	  if (!(a & RPSPC_EXT))
	  {
		a = GPIO;
		addr = (a & ADDR) >> RPSPC_A0_PIN;
		if (!(a & RPSPC_WR))
		{
			switch (addr)
			{
				case 0:
					if (mode == READ_FOR_DATA)	
					{
						cmd = a;
					}
					else
					{
						param[p++] = a;
					}
					mode = RECEIVED;
					if (cmd_params[cmd] < p)
					{
						switch (cmd)
						{
							case SDINIT:
//								printf("SD initialized\n");
								p = 0;
								break;
							case SDDRVSTS:
								data = 0x3f;
								break;
							case SDREAD:
								tmpbuf = fdd[cmd_params[1]];
								memcpy(buffer, tmpbuf+(cmd_params[2] * 16 + cmd_params[3])*256, 256 * cmd_params[0]);
							default:
								break;
						}
					}
					break;
				case 2:
					printf("RFD\n");
					if (a & 0xff == 0)
					{
						mode = READY;
						res = wRFD;
					}
					else if (mode == READY)
					{
						if (a & rATN)
						{
							res = wRFD;
							mode = READ_FOR_DATA;
						}
						else if (a & rRFD)
						{
							res = wDAV;
							data = buffer[i++];
							mode = DATA_VALID;
						}
					}
					else if ((a & rDAV))
					{
						res = wDAC;
						mode = RECEIVED;
					}
					else 
					{
						mode = READY;
					}
				default:
					break;
			}
			rw = 0;
			value = a;
		}
		else
		{
			GPIO_CLR = 0xff;
			switch (addr)
			{
				case 1:
					GPIO_SET = data;
					value = data;
					data = data + 1;
					break;
				case 2:
					GPIO_SET = res;
					value = res;
					break;
			}
			rw = 1;
		}
		while(!(GPIO & RPSPC_EXT));
		//if (((a & (ADDR | RPSPC_WR | RPSPC_RD)) != pAddr))
		if (a & RPSPC_RD)
		{
//			GPIO_CLR = 0xff;
			printf("%s %d %02x %d\n", rw ? "read" : "write", (a & ADDR) >> RPSPC_A0_PIN, (char) value, mode);
			pAddr = a & (ADDR | RPSPC_WR | RPSPC_RD);
		}
		if (mode != pmode)
		{
//			printf("mode changed:%d\n", mode);
			pmode = mode;
		}
	  }
	  else if (!(a & RPSPC_RST))
	  {
		  while((!GPIO & RPSPC_RST));
	  }
  }
}	
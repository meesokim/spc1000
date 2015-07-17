/***************************************
 SPC-1000 IOCS
"SPC-1000용 주소모음"
written by Mayhouse
blog.naver.com/mayhouse
**************************************/

#ifndef SPC_IOCS
#define SPC_IOCS

#define _ACCPRT 	0x0864
#define	_FLOAD		0x0114	
#define	_FLOAD1		0x0122	
#define	_MLOAD		0x0134
#define _MSAVE		0x00b6
#define MTBYTE		0x13a8
#define MTADRS		0x13aa
#define	_ASCGET		0x0c62
#define CLR2		0xad5

__sfr __banked __at 0 ioport;

#define SDINIT 0
#define SDWRITE	1
#define SDREAD	2
#define SDSEND  3
#define SDCOPY	4
#define SDFORMAT 5
#define SDSTATUS 6
#define SDDRVSTS 7
#define SDRAMTST 8
#define SDTRANS2 9
#define SDNOACT	0xa
#define SDTRANS1 0xb
#define SDRCVE	0xc
#define SDGO	0xd
#define SDLOAD	0xe
#define SDSAVE	0xf
#define SDLDNGO	0x10

#define uint8 unsigned char

void sd_init();
void sd_write(uint8 block_n, uint8 drive_n, uint8 track_t, uint8 sector_s, char *data);
void sd_read(uint8 block_n, uint8 drive_n, uint8 track_t, uint8 sector_s, char *data);
//void sd_send(); not useful for end developer, please use sd_read() instead of this.
void sd_copy(uint8 block_n, uint8 drive_n, uint8 track_t, uint8 sector_s, uint8 drive_n, uint8 track_t, uint8 sector_s);
void sd_format(uint8 drive_n);
unsigned char sd_drvstate(void);
unsigned char sd_sendstate(void);

char cas_load(unsigned char *data, int len);
void cas_save(unsigned char *data, int len);
#endif


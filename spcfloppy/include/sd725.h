#ifndef SD725
#define SD725

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

uint8 sd_init();
void sd_write(uint8 block_n, uint8 drive_n, uint8 track_t, uint8 sector_s, char *data);
void sd_read(uint8 block_n, uint8 drive_n, uint8 track_t, uint8 sector_s, char *data);
//void sd_send(); not useful for end developer, please use sd_read() instead of this.
void sd_copy(uint8 block_n, uint8 srcdrv_n, uint8 srctrk_t, uint8 srcsect, uint8 destdrv_n, uint8 track_t, uint8 sector_s1);
void sd_format(uint8 drive_n);
unsigned char sd_drvstate(void);
unsigned char sd_sendstate(void);

#endif

char cas_load(unsigned char *data, int len);
void cas_save(unsigned char *data, int len);


//
// mandelbrot.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2015  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "rpibox.h"

extern tms9918 vdp;
CScreenDevice8 *m_Screen;

CRpiBox::CRpiBox (CScreenDevice8 *pScreen, CLogger *pLogger, CEMMCDevice *pEmmc, CMemorySystem *pMemorySystem)
:	
#ifdef ARM_ALLOW_MULTI_CORE
	CMultiCoreSupport (pMemorySystem),
#endif
	m_pScreen (pScreen),
	m_pEmmc (pEmmc),
	m_pLogger (pLogger)	
{
	::m_Screen = pScreen;
}

CRpiBox::~CRpiBox (void)
{
	m_pScreen = 0;
}

void CRpiBox::Run (unsigned nCore)
{
	switch (nCore)
	{
	case 0:
		printf("RpiBox Started (Run)\n");
		Storage();
		break;
#ifdef ARM_ALLOW_MULTI_CORE
	case 1:
		printf("VdpBox Started (Run)\n");
		VdpBox();
		break;
#endif
	}
}

int CRpiBox::printf(const char* format, ...)
{
	CString str;
	va_list argptr;
    va_start(argptr, format);
    str.FormatV(format, argptr);
    va_end(argptr);
	m_pScreen->Write(str, str.GetLength());	
	return 0;
}

	static int a, addr, wr, datain, dataout, data3, readsize, data0;
	static unsigned char diskbuf[258*256*8], buffer[256*256*32], tapbuf[256*256*32], f[256*256];
	static unsigned char cflag, blocks, drv, tracks, sectors;
	static char *tmpbuf, *rpibuf;
	static int params[10], p, q, t, rpi_idx, len, len0, fileno, cmd, nBytesRead;
	static unsigned char * fdd[3];
	static char filename[256];
	static int oldnum = 0;
	static CString FileName;
	static CString Message;
	static FRESULT Result;
		char pattern[256];
	char files[256*256];
	char files2[256*256];
	char drive[256];

char *CRpiBox::strrchr(const char *s, int ch)
{
    char *start = (char *) s;

    while (*s++);
    while (--s != start && *s != (char) ch);
    if (*s == (char) ch) return (char *) s;

    return 0;
}

char * CRpiBox::fnRPI_FILES(char *drive, char *pattern)
{
	// Show contents of root directory
	DIR Directory;
	FILINFO FileInfo;
	CString FileName;
	FRESULT Result = f_findfirst (&Directory, &FileInfo,  DRIVE "/", "*tap;*.cas;");
	int len = 0, len2= 0, length = 0;
	memset(files, 0, 256*256);
	memset(files2, 0, 256*256);
	for (unsigned i = 0; Result == FR_OK && FileInfo.fname[0]; i++)
	{
		if (!(FileInfo.fattrib & (AM_HID | AM_SYS)))
		{
			strcpy(files+len, FileInfo.fname);
			strcpy(files2+len2, FileInfo.fname);
			length = strlen(FileInfo.fname);
			len += length;
			len2 += length > 24 ? 24 : length;
			*(files+(len++))='\\';
			*(files2+(len2++))='\\';
		}

		Result = f_findnext (&Directory, &FileInfo);
	}
	printf ("%s\n", files);
	return files;
}

void CRpiBox::Storage()
{
	p = 0;
	q = 0;
	t = 0;
	data0 = 0;
	dataout = 0;
	data3 = 0;
	datain = 0;
	nBytesRead = 0;
	tmpbuf = 0;
	rpi_idx = 0;
	oldnum = 0;
	readsize = 0;
	fdd[0] = f;
	rpibuf = 0;
	rpi_idx = 0;
	datain = 0;
	dataout = 0;
	blocks = cflag = p = q = t = data0 = data3 = 0;
	readsize = 0;
	tmpbuf = 0;
	data0 = 0xff;
	CSpinLock spinlock;
	bool vdp_enable = false;
	// Mount file system
	if (f_mount (&m_FileSystem, DRIVE, 1) != FR_OK)
	{
		error ("Cannot mount drive: %s", DRIVE);
	} 
	printf("FileSystem mounted (Storage)\n");
	printf("completed\n");
	write32 (ARM_GPIO_GPFSEL0, 0x249249);
	write32 (ARM_GPIO_GPFSEL0+4, 0);
	write32 (ARM_GPIO_GPFSEL0+8, 0);
	while(1)
	{
		if (!(GPIO & RPSPC_EXT))
		{
			a = GPIO;
			addr = (a & ADDR) >> RPSPC_A0_PIN;
			wr = (a & RPSPC_RD) != 0;
			if (a & RPSPC_A11)
			{
				if (wr)
				{
					vdp_enable = true;
					if (addr & 1)
						tms9918_writeport1(vdp, a & 0xff);
					else
						tms9918_writeport0(vdp, a & 0xff);
				}
				else
				{
					GPIO_CLR(0xff);
					if (addr & 1)
						GPIO_SET(tms9918_readport1(vdp));
					else
						GPIO_SET(tms9918_readport0(vdp));
				}
			}
			else
			{
				switch (addr)
				{
					case 0:
						if (wr) 
						{
							datain = a & 0xff;
						}
						else
						{
							GPIO_CLR(0xff);
							GPIO_SET(data0);
						}
						break;
					case 1:
						if (!wr)
						{
							GPIO_CLR(0xff);
							GPIO_SET(dataout);
						}
						break;
					case 2:
						if (wr)
						{
							switch (a & 0xf0)
							{
								case rATN: // 0x80 --> 0x02
									cflag = wRFD; 
									p = 0;
									break;
								case rDAV: // 0x10 --> 0x04
									if (!(cflag & wDAC))
									{
										cflag |= wDAC;
										if (p < 10)
											params[p] = datain;
										if (p == 0)
										{
											printf ("[%d]", datain);
										}
										q = 0;
										readCommand(params[0]);
									}									
									break;
								case rRFD: // 0x20 --> 0x01
									cflag |= wDAV;
									break;
								case rDAC: // 0x40 --> 0x00
									if (cflag & wDAV)
									{
										cflag &= ~wDAV;
										q++;
	//									data3 = q;
									}
									break;
								case 0: // 0x00 --> 0x00
									if (cflag & wDAC)
									{
										cflag &= ~wDAC;
										p++;
									}
									else if (cflag & wDAV)
									{
										dataout = buffer[q];
										//m_Screen.printf("%02x,", dataout);
									}
									break;
								default:
									break;
							}
						}
						else
						{
							GPIO_CLR(0xff);
							GPIO_SET(cflag);
						}
						break;
					case 3:
						if (!wr)
						{
							GPIO_CLR(0xff);
							GPIO_SET(data3);
						}
						else
						{
							data3 = tapbuf[t++] == '1' ? 1 : 0;	
						}
					default:
						break;
				}
			}
			while(!(GPIO & RPSPC_EXT));
		}
	}
}

void CRpiBox::readCommand(int cmd)
{
	switch (cmd)
	{
		case SDINIT:
			buffer[0] = 100;
			printf ("SDINIT\n", cmd);
			FIL File;
			Result = f_open (&File, DRIVE FILENAME, FA_READ | FA_OPEN_EXISTING);
			if (Result != FR_OK)
			{
				printf ("loading failed: %s\n", FILENAME);
			}
			else
			{
				unsigned nBytesRead;
				f_read(&File, f, sizeof f, &nBytesRead);
				f_close (&File);
				fdd[0] = f;
				printf ("loading: %s(%d) - %02x %02x\n", FILENAME, nBytesRead, f[0], f[1]);
			}
			
			break;
		case SDWRITE:
			if (p == 4)
			{
				blocks = params[1];
				drv = params[2];
				tracks = params[3];
				sectors = params[4];
				tmpbuf = (char *)fdd[drv] + (tracks * 16 + (sectors - 1))*256;
			} else if (p > 4)
			{
				tmpbuf[p - 5] = datain;
			}
			break;
		case SDREAD:
			if (p == 4) 
			{
				blocks = params[1];
				drv = params[2];
				tracks = params[3];
				sectors = params[4];												
				readsize = 256 * blocks;
				memcpy(diskbuf, fdd[drv]+(tracks * 16 + (sectors - 1))*256, readsize);
				printf ("SDREAD(%d) %d block(s), drive %d, track %d, sector %d, %dbytes)\n", p, blocks, drv, tracks, sectors, readsize);
			}
			break;
		case SDSEND:
			memcpy(buffer, diskbuf, readsize);
			printf ("SDSEND(%d) %02x %02x %02x %02x .. %02x\n", p, buffer[0], buffer[1], buffer[2], buffer[3], buffer[readsize-1]);
			break;
		case SDCOPY:
			memcpy(fdd[params[5]]+(params[6] * 16 + (params[7]-1))*256, fdd[params[2]]+(params[3]*16+(params[4]-1))*256, 256 * params[1]);
			break;
		case SDSTATUS:
			buffer[0] = 0xc0;
			printf ("SDSTATUS\n");
			break;
		case ((int)SDDRVSTS):
			buffer[0] = 0xff;
			printf ("SDDRVSTS\n");
			break;
		case RPI_FILES:
			if (p == 0)
			{
				rpi_idx = 0;
				strcpy(drive, "SD:/");
				strcpy(pattern, "*.tap");
				rpibuf = drive;
			} 
			if (datain == 0)
			{
				if (rpibuf == pattern || p == 0)
				{
					tmpbuf = fnRPI_FILES(drive, pattern);
					strcpy((char *)buffer, files2);
					printf ("RPI_FILES: drive=%s, pattern=%s\n%s\n", drive, pattern, files2);
				}
			}
			else if (params[p] == '\\')
			{
				rpibuf[rpi_idx] = 0;
				rpi_idx = 0;
				rpibuf = pattern;
			}
			else
				rpibuf[rpi_idx++] = datain;
			break;
		case RPI_LOAD:
			if (p == 2)
			{
				oldnum = fileno = params[1] + params[2] * 256;
				printf ("fileno:%d\n", fileno);
				len0 = 0;
				len = 0;
				while(len0 < fileno)
				{
					if (files[len++] == '\\')
						len0++;
				}	
				len0 = 0;
				while(files[len+len0++] != '\\'); 
				memcpy(filename, "SD:/", 5);
				memcpy(filename+4, files+len, len0);
				*(filename+4+len0-1)=0;
				FILINFO fno;
				f_stat(filename, &fno);
				printf ("RPI_LOAD: No.%d %s (%d)\n", fileno, filename, fno.fsize);
				Result = f_open (&File, filename, FA_READ | FA_OPEN_EXISTING);
				if (Result != FR_OK)
				{
					printf ("loading failed: %s\n", filename);
				}
				else
				{
					unsigned nBytesRead;
					f_read(&File, buffer, fno.fsize, &nBytesRead);
					f_close (&File);
					printf ("loading successful: %s\n", filename);
					unsigned char s = filename[strlen(filename)-1];
					if (s =='s' || s == 'S')
					{
						for(unsigned i = 15; i < nBytesRead; i++)
						{
							for (unsigned j = 0; j < 8; j++)
								tapbuf[i*8+j]   = ((buffer[i] >> (7-j)) & 1) + '0';
						}
					}
					else
						memcpy(tapbuf, buffer, nBytesRead);
					t = 0;
				}
			}
			break;
		case RPI_OLDNUM:
			buffer[0] = oldnum & 0xff;
			buffer[1] = oldnum >> 8;
			q = 0;
//											printf("oldnum = %d\n", oldnum);
			break;								
		default:
			buffer[0] = cmd * cmd;
			break;
	}
}

#ifdef ARM_ALLOW_MULTI_CORE
void CRpiBox::VdpBox()
{
	int time = 0;
	vdp = tms9918_create();	
	while(true)
	{
		if (time++ > (int)(100000000/30/261))
		{
			tms9918_periodic(vdp);
			time = 0;
		};
	}	
}
#endif

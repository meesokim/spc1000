/*
 * Write the CP/M systemfiles to system tracks of drive A
 *
 * Copyright (C) 1988-2014 by Udo Munk
 *
 * History:
 * 29-APR-88 Development on TARGON/35 with AT&T Unix System V.3
 * 11-MAR-93 comments in english and ported to COHERENT 4.0
 * 02-OCT-06 modified to compile on modern POSIX OS's
 * 10-JAN-14 lseek POSIX conformance
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include "imageCode.h"
#include "vf.h"
#include "logger.h"

/*
 *	This program writes the CP/M 2.2 OS from the following files
 *	onto the system tracks of the boot disk (drivea.cpm):
 *
 *	boot loader	boot.bin	(Mostek binary format)
 *	CCP		    cpm.bin		(binary format)
 *	BDOS		cpm.bin		(binary format)
 *	BIOS		bios.bin	(Mostek binary format)
 */

#define SECSIZE 256 
#define TRKSIZE 16

extern int writeDisk(char *buffer, long int location, unsigned int size, FILE *writeFile);
extern unsigned long int getSectorLocation(int track, int record);
int track, sectrk;

void setDiskSectorPos(int t, int s) {
	track = t;
	sectrk = s;
}
void writeDiskSector(char *data, FILE *f) {
	int pos;
	printf("Track:%d, Sector:%d Pos:%06x written\n", track*2+(sectrk > 15), (sectrk > 15 ? sectrk - 16 : sectrk)+1, pos = getSectorLocation(track, sectrk*2));
	writeDisk(data, pos, 256, f);
	if (++sectrk >= 32) {
		sectrk = 0; track++;
	}
}

int main(int argc, char *argv[])
{
	//unsigned char header[3];
	char sector[SECSIZE];
	//register int i;
	int fd, readed;
	
	char file[1024];
	if (argv[1] > 0)
		strcpy(file, (char*)argv[1]);
	else {
		printf("Usage: %s target_disk_image\n", argv[0]);
		exit(1);
	}
    // open image
	if (checkFileExistance(file) == 0) {
		msg(LOG_WARN, "Image '%s' not found, exiting.\n", argv[1]);
		exit(-1);
	}
	FILE *imageFile = openImageRW(file);
	/* open boot loader (boot.bin) for reading */
	if ((fd = open("boot.bin", O_RDONLY)) == -1) {
		perror("file boot.bin error");
		exit(1);
	}
	/* read boot loader */
	memset((char *) sector, 0, SECSIZE);
	read(fd, (char *) sector, SECSIZE);
	close(fd);
	setDiskSectorPos(0,0);
	//lseek(drivea, 0 * SECSIZE, SEEK_SET);
	/* and write it to disk in drive A */
	//write(drivea, (char *) sector, SECSIZE);
	writeDiskSector(sector, imageFile);
	/* position to CCP in cpm.bin, needed if created with SAVE or similar */
	/* open CP/M system file (cpm.bin) for reading */
	FILE *f;
	if ((f = fopen("cpm.bin", "rb")) < 0) {
		perror("file cpm.bin open error");
		exit(1);
	}

	/* read CCP and BDOS from cpm.bin and write them to disk in drive A */
	//int j;
	printf ("read (cpm.bin) ..\n");
	while ((readed = fread((char *) sector, 1, SECSIZE, f)) == SECSIZE) {
		writeDiskSector(sector, imageFile);
	}
	//printf("%d\n", readed);
	if (readed > 0)
	{
		memset(sector+readed, 0, SECSIZE-readed);
		//write(drivea, (char *) sector, SECSIZE);
		writeDiskSector(sector, imageFile);	
	}
	fclose(f);
	/* open BIOS (bios.bin) for reading */
	
	if ((f = fopen("bios.bin", "rb")) == NULL) {
		perror("file bios.bin open error");
		f = fopen("boot.bin", "rb");
		fseek(f, 0x1700, SEEK_SET);
		printf("read (boot.bin for bios) ..\n");
	}
	else
		printf("read (bios.bin) ..\n");
	while ((readed = fread((char *) sector, 1, SECSIZE, f)) == SECSIZE) {
		//write(drivea, (char *) sector, readed);
		writeDiskSector(sector, imageFile);		
		//printf("%d..", readed);
	}
	if (readed > 0) {
		//printf("%d\n", readed);
		memset(sector+readed, 0, SECSIZE-readed);
		//write(drivea, (char *) sector, SECSIZE);
		writeDiskSector(sector, imageFile);
	}
	fclose(f);
	fclose(imageFile);	
	return(0);
}

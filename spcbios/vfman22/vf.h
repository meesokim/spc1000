 /*
 * vf.h version 2.2 part of the Vfman 2.2 package
 *
 * This version delivered in 2013 by Fred Jan Kraan (fjkraan@xs4all.nl)
 *
 * vfman is placed under the GNU General Public License in March 2010.
 *
 *  This file is part of Vfman 2.2.
 *
 *  Vfman is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Vfman is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Vfman; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/*
 * vf.h - header file for vformat.c, vfread.c, vfwrite.c
 */



//Record - The logical CP/M sector of 128 bytes
//Sector - The actual Epson sector of 256 bytes
//Block  - The logical CP/M block of 2048 bytes or 16 records. This is for 
//         disks with less than 256 blocks (= 512 kByte)
//Extend - A directory entry location of 32 bytes. There are 64 extends in the 
//         directory area of 2048 bytes (one physical sector)

// CP/M Dimensions
#define RECORD   128
#define BLOCK    2048
#define EXTEND   32
#define HEADS    2
#define DIRTRACK 4
#define RECORDSPERBLOCK BLOCK / RECORD

#define D88_MAX_TRACK_SIDES  164
#define D88_HEADER_NAME_SIZE  17
#define D88_HEADER_RSRV_SIZE   9
#define D88_SECTOR_BASE_LOCATION 0x02B0   
#define D88_MAX_SECTORS     1280 // 40 * 2 * 16
#define D88_SECTORS_PER_SIDE 16

// Disk related 
#define SECTOR          0x100
#define TRACKS          0x28
#define BLOCKS_ON_DISK  0x8b
#define SECTORSPERBLOCK 0x08
#define SECTORSPERTRACK 0x10
#define DIRECTORYEXTENDS 0x40

//D88 image map related
#define SECTORHEADER 0x10
#define SECTORBASE   0x02B0
#define BLOCKBASE    0x8AB0
//#define BLOCKBASE    0x68B0
#define TRACK        (SECTORHEADER + SECTOR) * SECTORSPERTRACK

// Directory entry locations
#define STATUS	0x00            // UU byte
#define FILENAMEBASE	0x01   	// Fx bytes
#define FILEEXTBASE	0x09	// Tx bytes
#define FILENAMEBASESIZE 0x08
#define FILEEXTSIZE     0x03
#define EXTENDNO        0x0C    // Xl byte
#define EXTENDRECORDS	0x0F    // Rc byte
#define READONLYBIT     0x09    //
#define SYSTEMFILEBIT   0x0A    //
#define ARCHIVEBIT      0x0B    //

#define EXTBLOCKBASE    0x10
#define EXTBLOCKS       0x10

#define FORMATPATTERN   0xE5
#define CHARBITMASK     0x7F

// track header
struct d88sct_t {
	unsigned char c, h, r, n;
	unsigned short nsec;
	unsigned char dens, del, stat;
	unsigned char rsrv[5];
	unsigned short size;
}; 

typedef struct {
    char addrLLSB;
    char addrLSB;
    char addrMSB;
    char addrMMSB;
} SectorAddress;

typedef union {
	SectorAddress addrsize;
	int size;
} SectorAddress_t;

#if 0
typedef struct  {
	char title[17];
	unsigned char rsrv[9];
	unsigned char protect;
	unsigned char type;
	unsigned int size;
	unsigned int trkptr[164];
} d88Header_t;
#endif 

#if 1
typedef struct {
    char name[D88_HEADER_NAME_SIZE]; // 0x00 - 0x011
    char rsrv[D88_HEADER_RSRV_SIZE]; // 0x12 - 0x1B
    char protect;                    // 0x1C
    char type;                       // 0x1D
    SectorAddress_t size;            // 0x1E
    SectorAddress_t sectorAddress[D88_MAX_TRACK_SIDES];
} d88Header_t;
#endif

//d88Header_t d88Header;

// file data
typedef struct {
	unsigned char *fileName;
	FILE *file;
	unsigned char cpmFileName[11];
	unsigned int fileBlocks[BLOCKS_ON_DISK];
	unsigned int recordsInLastExtend;
	int extends[DIRECTORYEXTENDS];
	int found;
	unsigned int fileSize;
} fileData_t;

// file data instance
//fileData_t fileData; 

// image data
typedef struct {
	char *fileName;
	FILE *file;
} imageData_t;

// image data instance
//imageData_t imageData;

// arguments and options
typedef struct {
	char *imageName;
	char *fileName;
	int debug;
} argData_t;

// argument and options instance
//argData_t argData;

extern d88Header_t d88Header;
extern fileData_t fileData; 
extern imageData_t imageData;
extern argData_t argData;

void parseArgs (int argc, char **argv, argData_t *pArgData);
void reformatFileName(unsigned char *pFileName, const unsigned char *pExtend);
void printDirectory(FILE *imageFile);
char setCharOnHibit(unsigned char testChar, char replaceChar);
void convertFileName(fileData_t *pFileData);
void findFileExtends(FILE *imageFile, fileData_t *pFileData);
void stripHighbit(unsigned char *fileName);
void findFileBlocks(FILE *imageFile, fileData_t *pFileData);
void copyBlocks(FILE *imageFile, fileData_t *pFileData, int extendNumber);
void storeBlockNumber(fileData_t *pFileData, int block);
void createFile(FILE *imageFile, fileData_t *pFileData);
void lastBlockCopy(FILE *imageFile, fileData_t *pFileData, int block);
void otherBlockCopy(FILE *imageFile, fileData_t *pFileData, int block);

long int getDirExtendLocation (int count);
long int getBlockLocation(int block);
//long int getSectorLocation(int block, int sector);
long int getRecordLocation(int block, int record);

void fileDataInit(fileData_t *pFileData);
int checkFileExistance(char *fileName);
int getFreeBlockCount(FILE *imageFile);
int getImageFileBlockCount(fileData_t *pFileData);
int getImageFileExtendCount(fileData_t *pFileData);
unsigned int getFileSize(fileData_t *pFileData);
int getSizeInRecords(int size);
int getSizeInBlocks(int size);
int getSizeInExtends(int size);

int getFreeDirectorySpace(FILE *imageFile);
void eraseFile(FILE *imageFile, fileData_t *pFileData);
void findUnusedBlocksExtends(FILE *imageFile, fileData_t *pFileData);
void writeDirectory(FILE *imageFile, fileData_t *pFileData);
void writeFile(FILE *imageFile, fileData_t *pFileData);
int getNextFreeBlock(fileData_t *pFileData, int index);

void initImageHeader(d88Header_t *pD88Header);
void createImage(char *imageName, d88Header_t *pD88Header);

FILE *openImageRO(char *imageName);
FILE *openImageRW(char *imageName);
FILE *openFileRO(char *imageName);
FILE *openFileRW(char *imageName);
//void readDisk(char *buffer, long int location, unsigned int size, FILE *readFile);
//void writeDisk(char *buffer, long int location, unsigned int size, FILE *writeFile);

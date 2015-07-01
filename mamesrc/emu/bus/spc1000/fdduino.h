// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __SPC1000_FDDUINO_H__
#define __SPC1000_FDDUINO_H__

#include "exp.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
typedef unsigned char uint8;
typedef char byte;

#define ATN_S 1 << 7
#define DAC_S 1 << 6
#define RFD_S 1 << 5
#define DAV_S 1 << 4
#define DAC_F 1 << 2
#define RFD_F 1 << 1
#define DAV_F 1 

#define PB_O pB
#define PC_O pC

#include <stdio.h>
#include <String>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef bool boolean;

#define FILE_READ 1
#define FILE_WRITE 0
#define O_RDONLY 2

class File {
 private:
  char _name[13]; // our name
  FILE *_file;  // underlying file pointer

public:
  File(const char *name, const int m);     // wraps an underlying SdFile
  File(void);      // 'empty' constructor
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual int read();
  virtual int peek();
  virtual int available();
  virtual void flush();
  int read(void *buf, uint16_t nbyte);
  boolean seek(uint32_t pos);
  uint32_t position();
  uint32_t size();
  void close();
  operator bool();
  char * name();

  boolean isDirectory(void);
  File openNextFile(uint8_t mode = O_RDONLY);
  void rewindDirectory(void);
};

#define SD_CHIP_SELECT_PIN 0

class SD {
	public:
	SD();
	boolean begin(uint8_t csPin = SD_CHIP_SELECT_PIN);

	// Open the specified file/directory with the supplied mode (e.g. read or
	// write, etc). Returns a File object for interacting with the file.
	// Note that currently only one file can be open at a time.
	File open(const char *filename, uint8_t mode = FILE_READ);
//	File open(const String &filename, uint8_t mode = FILE_READ) { return open( filename.c_str(), mode ); }

	// Methods to determine if the requested file path exists.
	boolean exists(char *filepath);
//	boolean exists(const String &filepath) { return exists(filepath.c_str()); }

	// Create the requested directory heirarchy--if intermediate directories
	// do not exist they will be created.
	boolean mkdir(char *filepath);
//	boolean mkdir(const String &filepath) { return mkdir(filepath.c_str()); }

	// Delete the file.
	boolean remove(char *filepath);
//	boolean remove(const String &filepath) { return remove(filepath.c_str()); }

	boolean rmdir(char *filepath);
//	boolean rmdir(const String &filepath) { return rmdir(filepath.c_str()); }		
	
};

class Arduino2560 {
	public:
		Arduino2560() {
			pA = 0;
			pB = 0;
			pC = 0;
			cmd = -1;
			ih = iq = args = 0;
		}
		int getPortA() {
			return pA;
		}
		void setPortA(int val) {
			pA = val;
		}
		int getPortB() {
			return pB;
		}
		void setPortB(int val) {
			pB = val; // input only
		}
		void setPortC(int val) {
			pC = (pC & 0xf) | (val & 0xf0);
			doService();
			//printf("PC=%02x\n", pC);
		}
		int getPortC() {
			return pC;
		}
	private:
		int pA, pB, pC;
		int fs;
		
		File *file[2];
		byte param[2048];
		void doService();
		void doCommand(int data);
		void digitalWrite(int pin, int v) {
			if (v != 0)
				pC = pC | (1 << pin);
			else
				pC = pC & (0xf & ~(1 << pin));
		}
		byte digitalRead()
		{
			return 0;
		}
		enum FDD_CMD {
			FDUINZ,
			WRTDSK,
			RDDSK,
			TRNBUF,
			COPY,
			FORMAT,
			CMDSTAT,
			DRVSTAT,
			TSTMEM,
			READMEM,
			OUTPORT,
			GETMEM,
			SETMEM,
			EXEUSR,
			LOADMEM,
			SAVEMEM,
			LOADGO,
			FWRDSK,
			CMDMAX
		};
		static const int HIGH = 1;
		static const int LOW = 0;
		int args;
		int cmd;
		int iq;
		int ih;
		uint8 resq[2048];
		enum FDD_FLAG {
			FDD_DAV,
			FDD_RFD,
			FDD_DAC,
			SPC_DAV = 4,
			SPC_RFD,
			SPC_DAC,
			SPC_ATN
		};
		enum FDD_STATUS {
		  NONE,
		  ATTENSION,
		  DATAVALID,
		  DATACOMPLETED,
		  DATAREQ,
		  DATASEND,
		};
		void addByte(int b)
		{
			if (iq < sizeof(resq))
				resq[iq++] = b;
//			printf("addByte: resq[%d]=%d\n", iq-1,b);
		}
		byte getByte()
		{
//			printf("getByte: ih=%d, iq=%d\n", iq, ih);
			if (ih <= iq)
				return resq[ih++];
			return 0;
		}
		bool checkByte()
		{
//			printf("checkByte: ih=%d, iq=%d\n", iq, ih);
			if (ih <= iq)
				return true;
			return false;
		}
		void initByte()
		{
			memset((void*)resq, 0, sizeof(resq));
			ih = 0;
			iq = 0;
		}
		void resetCmd()
		{
			cmd = -1;
			args = 0;
		}		
};
// ======================> spc1000_fdduino_exp_device

class spc1000_fdduino_exp_device : public device_t,
						public device_spc1000_card_interface
{
public:
	// construction/destruction
	spc1000_fdduino_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

public:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(fdduino_interrupt);

private:
	// internal state
	Arduino2560 m2560;
};


// device type definition
extern const device_type SPC1000_FDDUINO_EXP;

#endif  /* __SPC1000_FDDUINO_H__ */

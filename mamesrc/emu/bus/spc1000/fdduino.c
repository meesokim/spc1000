// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    SPC-1000 FDDUINO expansion unit

***************************************************************************/

#include "emu.h"
#include "fdduino.h"
#define FILE_READ 1
#define FILE_WRITE 0
#define O_RDONLY 2

#include <fstream>
#include <iostream>

File::File(const char *name, int m)
{
	// wraps an underlying SdFile
	char *mode = "r";
	if (mode == FILE_READ)
		mode = "r";
	else if (mode == FILE_WRITE)
		mode = "w";
	else
		mode = "r";
	file = fopen(name, mode);
}

File::File()
{
}

size_t File::write(uint8_t b)
{
	return 0;
}

size_t File::write(const uint8_t *buf, size_t size)
{
	return 0;
}

int File::read() 
{
	return 0;
}

int File::peek()
{
	return 0;
}
int File::available()
{
	return 0;
}
void File::flush()
{
}
int File::read(void *buf, uint16_t nbyte)
{
	return 0;
}
boolean File::seek(uint32_t pos)
{
	return true;
}
uint32_t File::position()
{
	int pos = 0;
	return pos;
}
uint32_t File::size()
{
	return 0;
}
void File::Fileclose()
{
}
char * File::name()
{
}
boolean File::isDirectory()
{
	return true;
}
File File::openNextFile(uint8_t mode = O_RDONLY)
{
	return this;
}
void File::rewindDirectory(void)
{
	
};

#define SECBYTES 256

void Arduino2560::doCommand(int val)
{
	if (cmd < 0) {
		cmd = val;
		initByte();
	}
	switch (cmd)
	{
    case FDUINZ:
		printf("FDUINZ: FDD Initialized\n");
		resetCmd();
		break;
	case WRTDSK:
		if (args < 4)
			param[args++] = val;
		else
			
		break;
    case RDDSK:
      byte nsect, drive, track, sector;
      printf("RDDSK: Read Disk(p%d=0x%02x)\n", args, val);
	  if (args < 5) {
		param[args++] = val;
	  }
	  else { 
		nsect = param[1];
		drive = param[2];
		track = param[3];
		sector = param[4];
		printf("size(#ofsectors)=%d,drive=%d, track=%d, sector=%d\n", nsect, drive, track, sector);
		file[drive].seek((track * 16 + sector - 1));
		nsect = (nsect > 16 - sector ? 16 - sector : nsect);
		for (int i = 0; nsect * SECBYTES; i++)
			addByte(file[drive].read());
		resetCmd();
      }
      break;
    case TRNBUF:
      break;
    case COPY:
      break;
    case FORMAT:
      break;
    case CMDSTAT:
      break;
    case DRVSTAT:
      printf("DRVSTAT: DRIVE Status\n");
      addByte(0xff);	  
	  resetCmd();
      break;
    case TSTMEM:
      break;
    case READMEM:
      break;
    case OUTPORT:
      break;
    case GETMEM:
      break;
    case SETMEM:
      break;
    case EXEUSR:
      break;
    case LOADMEM:
      break;
    case SAVEMEM:
      break;
    case LOADGO:
      break;
    case FWRDSK:
      break;
  };
		
}
void Arduino2560::doService() {
	if (pC & ATN_S)
	{
		digitalWrite(FDD_RFD, HIGH);
		fs = ATTENSION;
	} else if (!(pC & ATN_S) && (pC & DAV_S))
	{
		byte data = getPortA();
		//printf("Write=%d\n", data);
		doCommand(data);
		//printf("cmd=%d\n", cmd);
		if (cmd == -1) {
			digitalWrite(FDD_RFD, LOW);			
		}
		digitalWrite(FDD_DAC, HIGH);		
		fs = DATAVALID;
	} else if (!(pC & DAV_S) && fs == DATAVALID)
	{
		digitalWrite(FDD_DAC, LOW);
		//printf("Write Completed\n");
		fs = NONE;
	} else if (pC & RFD_S)
	{
		//if (fs != DATAREQ) p("Data Request:%d\n", fs);
		fs = DATAREQ;
		PC_O = 0;
		digitalWrite(FDD_DAV, HIGH);
	} else if (!(pC & RFD_S) && fs == DATAREQ)
	{
		byte data = 0;
		data = getByte();
		//printf("Read=0x%02x\n", data);
		PB_O = data;
		digitalWrite(FDD_DAV, LOW);
		fs = DATASEND;
	}
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE_LINE_MEMBER(spc1000_fdduino_exp_device::fdduino_interrupt)
{
	// nothing here?
}

static MACHINE_CONFIG_FRAGMENT(scp1000_fdduino)

MACHINE_CONFIG_END

//-------------------------------------------------
//  device_mconfig_additions
//-------------------------------------------------

machine_config_constructor spc1000_fdduino_exp_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( scp1000_fdduino );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type SPC1000_FDDUINO_EXP = &device_creator<spc1000_fdduino_exp_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_fdduino_exp_device - constructor
//-------------------------------------------------

spc1000_fdduino_exp_device::spc1000_fdduino_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, SPC1000_FDDUINO_EXP, "SPC1000 FDDUINO expansion", tag, owner, clock, "spc1000_fdduino_exp", __FILE__),
			device_spc1000_card_interface(mconfig, *this),
			m2560()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_fdduino_exp_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_fdduino_exp_device::device_reset()
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
READ8_MEMBER(spc1000_fdduino_exp_device::read)
{
	switch (offset & 03)
	{
		case 0:
			return m2560.getPortA();
		case 1:
			return m2560.getPortB();
		case 2:
			return m2560.getPortC();
		default:
			return 0;
	}
}

//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(spc1000_fdduino_exp_device::write)
{
	switch(offset & 0x3)
	{
		case 0:
			m2560.setPortA(data);
			break;
		case 1:
			//m2560.setPortB(data);
			break;
		case 2:
			m2560.setPortC(data);
			break;
	}
	//printf("Write(%02x):%02x\n", offset & 0x3, data);
}

// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    SPC-1000 FDD unit

***************************************************************************/

#include "emu.h"
#include "fdd.h"
#include <dirent.h>

#define rATN 0x80
#define rDAC 0x40
#define rRFD 0x20
#define rDAV 0x10
#define wDAC 0x04<<4
#define wRFD 0x02<<4
#define wDAV 0x01<<4

#define RPI_FILES 0x20
#define RPI_LOAD  0x21


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

READ8_MEMBER(spc1000_fdd_exp_device::i8255_c_r)
{
	return m_i8255_0_pc >> 4;
}

WRITE8_MEMBER(spc1000_fdd_exp_device::i8255_b_w)
{
	m_i8255_portb = data;
}

WRITE8_MEMBER(spc1000_fdd_exp_device::i8255_c_w)
{
	if (!m_ext)
		m_i8255_1_pc = data;
	printf("port c:%02x\n", data>>4);
}

//-------------------------------------------------
//  fdc interrupt
//-------------------------------------------------

READ8_MEMBER( spc1000_fdd_exp_device::tc_r )
{
	logerror("%s: tc_r\n", machine().describe_context());

	// toggle tc on read
	m_fdc->tc_w(true);
	m_timer_tc->adjust(attotime::zero);

	return 0xff;
}

WRITE8_MEMBER( spc1000_fdd_exp_device::control_w )
{
	logerror("%s: control_w(%02x)\n", machine().describe_context(), data);

	// bit 0, motor on signal
	if (m_fd0)
		m_fd0->mon_w(!BIT(data, 0));
	if (m_fd1)
		m_fd1->mon_w(!BIT(data, 0));
}

static ADDRESS_MAP_START( sd725_mem, AS_PROGRAM, 8, spc1000_fdd_exp_device )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sd725_io, AS_IO, 8, spc1000_fdd_exp_device )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(tc_r, control_w) // (R) Terminal Count Port (W) Motor Control Port
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("d8255_master", i8255_device, read, write)
ADDRESS_MAP_END

static SLOT_INTERFACE_START( sd725_floppies )
	SLOT_INTERFACE("sd320", EPSON_SD_320)
SLOT_INTERFACE_END

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

MACHINE_CONFIG_MEMBER( spc1000_fdd_exp_device::device_add_mconfig )

	// sub CPU (5 inch floppy drive)
	MCFG_CPU_ADD("fdccpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(sd725_mem)
	MCFG_CPU_IO_MAP(sd725_io)

	MCFG_DEVICE_ADD("d8255_master", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8("d8255_master", i8255_device, pb_r))
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("d8255_master", i8255_device, pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(spc1000_fdd_exp_device, i8255_b_w))
	MCFG_I8255_IN_PORTC_CB(READ8(spc1000_fdd_exp_device, i8255_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(spc1000_fdd_exp_device, i8255_c_w))

	// floppy disk controller
	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("fdccpu", INPUT_LINE_IRQ0))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", sd725_floppies, "sd320", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", sd725_floppies, "sd320", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

ROM_START( spc1000_fdd )
	ROM_REGION(0x10000, "fdccpu", 0)
	ROM_LOAD("sd725a.bin", 0x0000, 0x1000, CRC(96ac2eb8) SHA1(8e9d8f63a7fb87af417e95603e71cf537a6e83f1))
ROM_END

//-------------------------------------------------
//  device_rom_region
//-------------------------------------------------

const tiny_rom_entry *spc1000_fdd_exp_device::device_rom_region() const
{
	return ROM_NAME( spc1000_fdd );
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_FDD_EXP, spc1000_fdd_exp_device, "spc1000_fdd_exp", "SPC1000 FDD expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_fdd_exp_device - constructor
//-------------------------------------------------

spc1000_fdd_exp_device::spc1000_fdd_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SPC1000_FDD_EXP, tag, owner, clock),
	device_spc1000_card_interface(mconfig, *this),
	m_cpu(*this, "fdccpu"),
	m_fdc(*this, "upd765"),
	m_pio(*this, "d8255_master"),
	m_fd0(nullptr), m_fd1(nullptr), m_timer_tc(nullptr), m_i8255_0_pc(0), m_i8255_1_pc(0), m_i8255_portb(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_fdd_exp_device::device_start()
{
	m_timer_tc = timer_alloc(TIMER_TC);
	m_timer_tc->adjust(attotime::never);

	m_fd0 = subdevice<floppy_connector>("upd765:0")->get_device();
	m_fd1 = subdevice<floppy_connector>("upd765:1")->get_device();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_fdd_exp_device::device_reset()
{
	m_cpu->set_input_line_vector(0, 0);

	// enable rom (is this really needed? it does not seem necessary for FDD to work)
	m_cpu->space(AS_PROGRAM).install_rom(0x0000, 0x0fff, 0x2000, memregion("fdccpu")->base());
}

void spc1000_fdd_exp_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
		case TIMER_TC:
			m_fdc->tc_w(false);
			break;
	}
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(spc1000_fdd_exp_device::read)
{
//	static int pdata = 0;
	// this should be m_pio->read on the whole 0x00-0x03 range?
	if (offset > 3)
		return 0xff;
	else
	{
		uint8_t data = 0;
		switch (offset)
		{
			case 1:
				data = (m_ext ? m_data1: m_i8255_portb);
//				if (m_ext)
//					printf("port 1:   %02x\n", data);
				break;
			case 2:
				data = m_i8255_1_pc >> 4;
//				if (data != pdata)
//					printf("port r:%02x\n", data);
//				pdata = data;
				break;
			case 3:
				data = m_data3;
				printf("%c", m_data3 > 0 ? '1' : '0');
		}
		return data;
	}
}

//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(spc1000_fdd_exp_device::write)
{
	// this should be m_pio->write on the whole 0x00-0x03 range?
	static int p = 0, q = 0;
	static int cmd = 0, len;
	static char buffer[256*256*9];
	static int params[10];
//	static int args[] {0,4,4,0,7,1,1,0,4,4,0,4,4,2,6,6,0,0,0,0,0,2};
	static int rpi_idx = 0;
	static char drive[256];
	static char pattern[256];
	static char *rpibuf, filenames[256*256*9], filename[256];
	static int q2 = 0, pos = 0, pose = 0, length = 0, num = 0;
	if (offset <= 3) 
	{
		switch (offset)
		{
			case 0:
				m_pio->write(space, 1, data);
				m_data0 = data;
				break;
			case 2:
//				printf("port 2:%02x\n", data);
				if (m_ext)
				{
					switch (data & 0xf0)
					{
						case rATN:
							p = 0;
							m_ext = 0;
							m_i8255_1_pc |= wRFD;
							break;
						case rDAV:
							m_i8255_1_pc |= wDAC;
							printf("*%s:%02x cmd=%02x\n", p == 0 ? "cmd" : "data", m_data0, params[0]);
							if (p < 10)
								params[p] = m_data0;
							switch (params[0])
							{
								case RPI_FILES:
									if (p == 1)
									{
										rpi_idx = 0;
										strcpy(drive, "SD:/");
										strcpy(pattern, "*.tap");
										rpibuf = drive;											
									}
									printf("%s\n", rpibuf);
									if (m_data0 == 0)
									{
										if (rpibuf == pattern || p == 0)
										{
											printf ("RPI_FILES: drive=%s, pattern=%s\n", drive, pattern);
											DIR *d = opendir(".");
											struct dirent *dir;
											len = 0;
											if (d)
											{
												while ((dir = readdir(d)) != NULL)
												{
													if (strstr(strlwr(dir->d_name), ".tap"))
													{
//														printf("%s\n", dir->d_name);
														strcpy(filenames+len, dir->d_name);
														len += strlen(dir->d_name);
														*(filenames+(len++))='\\';
													}
												}
												closedir(d);
												strcpy(buffer, filenames);
												printf("%s\n", buffer);
											}
											q = 0;
										}
									}
									else if (params[p] == '\\')
									{
										rpibuf[rpi_idx] = 0;
										rpi_idx = 0;
										rpibuf = pattern;
									}
									else
										rpibuf[rpi_idx++] = m_data0;
									break;
								case RPI_LOAD:
									if (p == 2)
									{
										num = params[2] * 256 + params[1];
										pos = 0;
										while(num--) while(*(filenames+pos++) != '\\');
										pose = pos; while(*(filenames+pose++) != '\\');
										pose -= pos;
										memcpy(filename, filenames+pos, pose);
										filename[pose-1] = 0;
										printf("RPI_LOAD: fnum=%d, %s\n", params[2] * 256 + params[1], filename);
										FILE* fp = fopen(filename, "rb");
										if (fp)
										{
											printf("file %s is opened\n", filename);
											fseek(fp, 0L, SEEK_END);
											length = ftell(fp); rewind(fp);
											fread(buffer, length, 1, fp);
											q2 = 0;
										}
									}
									break;
							}
							break;
						case rRFD: // 0x20 --> 0x01
							m_i8255_1_pc |= wDAV;
							break;							
						case rDAC:
							if (m_i8255_1_pc & wDAV)
							{
								m_i8255_1_pc &= ~wDAV;
								q++;
							}
						case 0:
							if (m_i8255_1_pc & wDAC)
							{
								p++;
								m_i8255_1_pc &= ~wDAC;
							}
							if (m_ext)
							{
								if (m_i8255_1_pc & wDAC)
								{
									m_i8255_1_pc &= ~wDAC;
									p++;
								}
								else if (m_i8255_1_pc & wDAV)
								{
									m_data1 = buffer[q];
								}
							}
					}					
				}
				else
				{
					m_i8255_0_pc = data;
					switch (data & 0xf0)
					{
						case rATN:
							p = 0;
							break;
						case rDAV:
							printf("%s:%02x\n", p == 0 ? "cmd" : "data", m_data0);
							if (p == 0)
								cmd = m_data0;
							if (cmd >= 0x20)
							{
								m_ext = 1;
								params[0] = cmd;
								m_i8255_1_pc |= wDAC;
							}
							break;
						case 0:
							if (m_i8255_1_pc & wDAC)
							{
								p++;
							}
					}
				}
				break;
			case 3:
				if (data == 0)
					m_data3 = (length > q2 ? 0xff : 0);
				else
					m_data3 = (buffer[q2++] > '0' ? 1 : 0);
		}
	}
}

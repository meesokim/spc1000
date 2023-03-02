// license:BSD-3-Clause
// copyright-holders:Miso Kim meeso.kim@gmail.com
/***************************************************************************

    SPC-1000 neo expansion unit

***************************************************************************/

#include <map>
#include <string>
#include <fstream>
using namespace std;

#include "spcbox.h"

#include "emu.h"
#include "neo.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

// WRITE_LINE_MEMBER(spc1000_neo_exp_device::vdp_interrupt)
// {
// 	// nothing here?
// }

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void spc1000_neo_exp_device::device_add_mconfig(machine_config &config)
{
	// TMS9928A(config, m_vdp, XTAL(10'738'635)); // TODO: which clock?
	// m_vdp->set_vram_size(0x4000);
	// m_vdp->int_callback().set(FUNC(spc1000_neo_exp_device::vdp_interrupt));
	// m_vdp->set_screen("tms_screen");
	// SCREEN(config, "tms_screen", SCREEN_TYPE_RASTER);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_NEO_EXP, spc1000_neo_exp_device, "spc1000_neo_exp", "SPC1000 VDP expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

SpcBox *sbox = 0;
//-------------------------------------------------
//  spc1000_neo_exp_device - constructor
//-------------------------------------------------

spc1000_neo_exp_device::spc1000_neo_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPC1000_NEO_EXP, tag, owner, clock)
	, device_spc1000_card_interface(mconfig, *this)
{
    if (!::sbox)
        ::sbox = new SpcBox();
    sbox=::sbox;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_neo_exp_device::device_start()
{
	sbox->initialize();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_neo_exp_device::device_reset()
{
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
uint8_t spc1000_neo_exp_device::read(offs_t offset)
{
	if (offset > 0xff)
		return 0xff;
	return sbox->read(offset);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void spc1000_neo_exp_device::write(offs_t offset, uint8_t data)
{
	if (offset <= 0xff)
	{
		sbox->write(offset, data);
        sbox->execute();
	}
}

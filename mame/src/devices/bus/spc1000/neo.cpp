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
#include "tap.h"

spc1000_neo_exp_device::spc1000_neo_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPC1000_NEO_EXP, tag, owner, clock)
	, device_spc1000_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
{
	strcpy(m_extension_list, ".tap;.cas;.zip");
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_neo_exp_device::device_start()
{
    if (!::sbox) {
		TapeFiles *tape = new TapeFiles();
		if (filename())
			tape->initialize(filename());
		else
			tape->initialize((const char*)tap_zip, sizeof(tap_zip));
        ::sbox = new SpcBox(tape);
	}
    sbox=::sbox;
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


//-------------------------------------------------
//  spc1000_neo_exp_device - destructor
//-------------------------------------------------

spc1000_neo_exp_device::~spc1000_neo_exp_device()
{
}

image_init_result spc1000_neo_exp_device::call_create(int format_type, util::option_resolution *format_options)
{
	return internal_load(true);
}

image_init_result spc1000_neo_exp_device::call_load()
{
	return internal_load(false);
}

void spc1000_neo_exp_device::call_unload()
{
// 	/* if we are recording, write the value to the image */
// 	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
// 		update();

// 	/* close out the cassette */
// 	m_cassette.reset();

// 	/* set to default state, but only change the UI state */
// 	change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}

const software_list_loader &spc1000_neo_exp_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}


image_init_result spc1000_neo_exp_device::internal_load(bool is_create)
{
	// cassette_image::error err;
	// device_image_interface *image = nullptr;
	// interface(image);

	// check_for_file();
	// if (is_create || (length()==0)) // empty existing images are fine to write over.
	// {
	// 	auto io = util::random_read_write_fill(image_core_file(), 0x00);
	// 	if (io)
	// 	{
	// 		// creating an image
	// 		err = cassette_image::create(
	// 				std::move(io),
	// 				&cassette_image::wavfile_format,
	// 				m_create_opts,
	// 				cassette_image::FLAG_READWRITE|cassette_image::FLAG_SAVEONEXIT,
	// 				m_cassette);
	// 	}
	// 	else
	// 	{
	// 		err = cassette_image::error::OUT_OF_MEMORY;
	// 	}
	// }
	// else
	// {
	// 	// opening an image
	// 	bool retry;
	// 	do
	// 	{
	// 		// we probably don't want to retry...
	// 		retry = false;

	// 		auto io = util::random_read_write_fill(image_core_file(), 0x00);
	// 		if (io)
	// 		{
	// 			// try opening the cassette
	// 			int const cassette_flags = is_readonly()
	// 					? cassette_image::FLAG_READONLY
	// 					: (cassette_image::FLAG_READWRITE | cassette_image::FLAG_SAVEONEXIT);
	// 			err = cassette_image::open_choices(
	// 					std::move(io),
	// 					filetype(),
	// 					m_formats,
	// 					cassette_flags,
	// 					m_cassette);
	// 		}
	// 		else
	// 		{
	// 			err = cassette_image::error::OUT_OF_MEMORY;
	// 		}

	// 		// special case - if we failed due to readwrite not being supported, make the image be read only and retry
	// 		if (err == cassette_image::error::READ_WRITE_UNSUPPORTED)
	// 		{
	// 			make_readonly();
	// 			retry = true;
	// 		}
	// 	}
	// 	while(retry);
	// }

	// if (err == cassette_image::error::SUCCESS)
	// {
	// 	/* set to default state, but only change the UI state */
	// 	change_state(m_default_state, CASSETTE_MASK_UISTATE);

	// 	/* reset the position */
	// 	m_position = 0.0;
	// 	m_position_time = machine().time().as_double();

	// 	/* default channel to 0, speed multiplier to 1 */
	// 	m_channel = 0;
	// 	m_speed = 1;
	// 	m_direction = 1;

	// 	return image_init_result::PASS;
	// }
	// else
	// {
	// 	std::error_condition imgerr = image_error::UNSPECIFIED;
	// 	switch(err)
	// 	{
	// 		case cassette_image::error::INTERNAL:
	// 			imgerr = image_error::INTERNAL;
	// 			break;
	// 		case cassette_image::error::UNSUPPORTED:
	// 			imgerr = image_error::UNSUPPORTED;
	// 			break;
	// 		case cassette_image::error::OUT_OF_MEMORY:
	// 			imgerr = std::errc::not_enough_memory;
	// 			break;
	// 		case cassette_image::error::INVALID_IMAGE:
	// 			imgerr = image_error::INVALIDIMAGE;
	// 			break;
	// 		default:
	// 			imgerr = image_error::UNSPECIFIED;
	// 			break;
	// 	}
	// 	image->seterror(imgerr, nullptr);
	// 	return image_init_result::FAIL;
	// }
	return image_init_result::PASS;
}



//-------------------------------------------------
//  display a small tape animation, with the
//  current position in the tape image
//-------------------------------------------------

std::string spc1000_neo_exp_device::call_display()
{
	// const int ANIMATION_FPS = 1;

	std::string result;

	// // only show the image when a cassette is loaded and the motor is on
	// if (exists() && !is_stopped() && motor_on())
	// {
	// 	static char const *const shapes[] = { u8"\u2500", u8"\u2572", u8"\u2502", u8"\u2571" };

	// 	// figure out where we are in the cassette
	// 	double position = get_position();
	// 	double length = get_length();
	// 	cassette_state uistate = get_state() & CASSETTE_MASK_UISTATE;

	// 	// choose which frame of the animation we are at
	// 	int n = (int(position) / ANIMATION_FPS) % std::size(shapes);

	// 	// play or record
	// 	const char *status_icon = (uistate == CASSETTE_PLAY)
	// 		? u8"\u25BA"
	// 		: u8"\u25CF";

	// 	// create information string
	// 	result = string_format("%s %s %02d:%02d (%04d) [%02d:%02d (%04d)]",
	// 		shapes[n],                  // animation
	// 		status_icon,                // play or record
	// 		((int)position / 60),
	// 		((int)position % 60),
	// 		(int)position,
	// 		((int)length / 60),
	// 		((int)length % 60),
	// 		(int)length);

	// 	// make sure tape stops at end when playing
	// 	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_PLAY)
	// 	{
	// 		if (m_cassette)
	// 		{
	// 			if (get_position() > get_length())
	// 			{
	// 				m_state = ((m_state & ~CASSETTE_MASK_UISTATE) | CASSETTE_STOPPED);
	// 			}
	// 		}
	// 	}
	// }
	return result;
}

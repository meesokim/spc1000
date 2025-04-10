// license:BSD-3-Clause
// copyright-holders:Miso Kim meeso.kim@gmail.com
/***************************************************************************

    SPC-1000 neo expansion unit

***************************************************************************/

#include <map>
#include <regex>
#include <filesystem>
#include <string>
#include <fstream>
using namespace std;

#include "emu.h"
#include "neo.h"

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig
//-------------------------------------------------

void spc1000_neo_exp_device::device_add_mconfig(machine_config &config)
{
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SPC1000_NEO_EXP, spc1000_neo_exp_device, "spc1000_neo_exp", "SPC1000 NEO expansion")

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  spc1000_neo_exp_device - constructor
//-------------------------------------------------

spc1000_neo_exp_device::spc1000_neo_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SPC1000_NEO_EXP, tag, owner, clock)
	, device_spc1000_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
{
	strcpy(m_extension_list, "tap,cas,zip,*");
}

std::vector<std::string_view> Split(const std::string_view str, const char delim = ';')
{   
    std::vector<std::string_view> result;

    int indexCommaToLeftOfColumn = 0;
    int indexCommaToRightOfColumn = -1;

    for (int i=0;i<static_cast<int>(str.size());i++)
    {
        if (str[i] == delim)
        {
            indexCommaToLeftOfColumn = indexCommaToRightOfColumn;
            indexCommaToRightOfColumn = i;
            int index = indexCommaToLeftOfColumn + 1;
            int length = indexCommaToRightOfColumn - index;

            // Bounds checking can be omitted as logically, this code can never be invoked 
            // Try it: put a breakpoint here and run the unit tests.
            /*if (index + length >= static_cast<int>(str.size()))
            {
                length--;
            }               
            if (length < 0)
            {
                length = 0;
            }*/

            std::string_view column(str.data() + index, length);
            result.push_back(column);
        }
    }
    const std::string_view finalColumn(str.data() + indexCommaToRightOfColumn + 1, str.size() - indexCommaToRightOfColumn - 1);
    result.push_back(finalColumn);
    return result;
}

std::pair<std::error_condition, std::string> spc1000_neo_exp_device::load(std::string_view path) {
	std::pair<std::error_condition, std::string> err;
	for(auto& de : Split(path)) {
		char *str = new char[string(de).length()+1];
		strcpy(str, string(de).c_str()); 
		ifstream f(str);
		if (f.good())
			files[size++] = (const char *)str;
		// printf("%s\n", files[size++]);
	}	
	return err;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void spc1000_neo_exp_device::device_start()
{
    // if (!::sbox) {
	// 	TapeFiles *tape = new TapeFiles();
	// 	printf("filename:%s\n", filename());
	// 	if (filename())
	// 		tape->initialize(filename());
	// 	else if (size>0) 
	// 		tape->initialize(files, size);
	// 	else
	// 		tape->initialize((const char*)tap_zip, sizeof(tap_zip));
    //     ::sbox = new SpcBox(tape);
	// }
    // sbox=::sbox;
	// sbox->initialize();
	spcinit();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void spc1000_neo_exp_device::device_reset()
{
	spcreset();
}

/*-------------------------------------------------
    read
-------------------------------------------------*/
uint8_t spc1000_neo_exp_device::read(offs_t offset)
{
	if (offset > 0xff)
		return 0xff;
	return spcread(offset);
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void spc1000_neo_exp_device::write(offs_t offset, uint8_t data)
{
	if (offset <= 0xff)
	{
		// printf("neo%d:%02x\n", offset, data);
		spcwrite(offset, data);
        //sbox->execute();
	}
}


//-------------------------------------------------
//  spc1000_neo_exp_device - destructor
//-------------------------------------------------

spc1000_neo_exp_device::~spc1000_neo_exp_device()
{
}

std::pair<std::error_condition, std::string> spc1000_neo_exp_device::call_create(int format_type, util::option_resolution *format_options)
{
	// std::pair<std::error_condition, std::string> err;
	printf("call_create\n");
	return internal_load(true);
}

std::pair<std::error_condition, std::string> spc1000_neo_exp_device::call_load()
{
	printf("call_load\n");
	return internal_load(false);
}

void spc1000_neo_exp_device::call_unload()
{
}

const software_list_loader &spc1000_neo_exp_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}


std::pair<std::error_condition, std::string> spc1000_neo_exp_device::internal_load(bool is_create)
{
	std::pair<std::error_condition, std::string> err;
	return err;
}

//-------------------------------------------------
//  display a small tape animation, with the
//  current position in the tape image
//-------------------------------------------------

std::string spc1000_neo_exp_device::call_display()
{
	std::string result = "/";
	return nullptr;
}

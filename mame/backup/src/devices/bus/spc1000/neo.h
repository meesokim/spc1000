// license:BSD-3-Clause
// copyright-holders:Miso Kim meeso.kim@gmail.com
#ifndef MAME_BUS_SPC1000_NEO_H
#define MAME_BUS_SPC1000_NEO_H

#pragma once

#include "exp.h"
#include "spc.h"
#include "softlist_dev.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> spc1000_neo_exp_device

class spc1000_neo_exp_device : public device_t, public device_spc1000_card_interface, public device_image_interface
{
public:
	// construction/destruction
	spc1000_neo_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~spc1000_neo_exp_device();

	// image-level overrides
	virtual std::pair<std::error_condition, std::string>  call_load() override;
	virtual std::pair<std::error_condition, std::string>  call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;
	virtual std::string call_display() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return true; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool support_command_line_image_creation() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override { return "FILE"; }
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }
	virtual const char *image_type_name() const noexcept override { return "files"; }
	virtual const char *image_brief_type_name() const noexcept override { return "file"; }

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// image-level overrides
	virtual const software_list_loader &get_software_list_loader() const override;
	// std::pair<std::error_condition, std::string> load(std::string_view path) override;
	std::pair<std::error_condition, std::string> load(std::string_view path) override;
	//  { printf("load:%s\n", std::string(path).c_str()); return image_init_result::PASS; };
	std::pair<std::error_condition, std::string> load_internal() { std::pair<std::error_condition, std::string> err; printf("load\n"); return err; };
	std::pair<std::error_condition, std::string> finish_load() { std::pair<std::error_condition, std::string> err; printf("finish_load\n"); return err; };

	virtual uint8_t read(offs_t offset) override;
	virtual void write(offs_t offset, uint8_t data) override;

private:
	char m_extension_list[256];
	const char *m_interface;
	std::pair<std::error_condition, std::string> internal_load(bool is_create);
	const char *files[1024];
	int size = 0;
};

// device type definition
DECLARE_DEVICE_TYPE(SPC1000_NEO_EXP, spc1000_neo_exp_device)

// device iterator
typedef device_type_enumerator<spc1000_neo_exp_device> neo_device_enumerator;

#endif // MAME_BUS_SPC1000_NEO_H

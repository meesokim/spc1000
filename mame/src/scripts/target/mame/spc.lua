-- license:BSD-3-Clause
-- copyright-holders:MAMEdev Team

---------------------------------------------------------------------------
--
--   spc.lua
--
--   Small driver-specific example makefile
--   Use make SUBTARGET=spc to build
--
---------------------------------------------------------------------------


--------------------------------------------------
-- Specify all the CPU cores necessary for the
-- drivers referenced in spc.lst.
--------------------------------------------------

CPUS["Z80"] = true
CPUS["UPD7810"] = true
CPUS["M6805"] = true
CPUS["NEC"] = true
--------------------------------------------------
-- Specify all the sound cores necessary for the
-- drivers referenced in spc.lst.
--------------------------------------------------

SOUNDS["AY8910"] = true
SOUNDS["SAMPLES"] = true
SOUNDS["DAC"] = true
SOUNDS["DISCRETE"] = true
SOUNDS["WAVE"] = true
SOUNDS["BEEP"] = true
SOUNDS["SPEAKER"] = true
SOUNDS["VOLT_REG"] = true
--------------------------------------------------
-- specify available video cores
--------------------------------------------------
VIDEOS["MC6847"] = true
VIDEOS["TMS9928A"] = true

--------------------------------------------------
-- specify available machine cores
--------------------------------------------------
MACHINES["SPC1000"] = true
MACHINES["I8255"] = true
MACHINES["UPD765"] = true
MACHINES["E05A03"] = true
MACHINES["E05A30"] = true
MACHINES["EEPROMDEV"] = true
MACHINES["STEPPERS"] = true
MACHINES["FDC_PLL"] = true
--------------------------------------------------
-- specify used file formats
--------------------------------------------------
FORMATS["SPC1000_CAS"] = true
FORMATS["UPD765_DSK"] = true
--------------------------------------------------
-- specify available bus cores
--------------------------------------------------

BUSES["CENTRONICS"] = true
BUSES["SPC1000"] = true
BUSES["GENERIC"] = true
--------------------------------------------------
-- This is the list of files that are necessary
-- for building all of the drivers referenced
-- in spc.lst
--------------------------------------------------

function createProjects_mame_spc(_target, _subtarget)
	project ("mame_spc")
	targetsubdir(_target .."_" .. _subtarget)
	kind (LIBTYPE)
	uuid (os.uuid("drv-mame-spc"))
	addprojectflags()
	precompiledheaders()

	includedirs {
		MAME_DIR .. "src/osd",
		MAME_DIR .. "src/emu",
		MAME_DIR .. "src/devices",
		MAME_DIR .. "src/mame",
		MAME_DIR .. "src/lib",
		MAME_DIR .. "src/lib/util",
		MAME_DIR .. "3rdparty",
		GEN_DIR  .. "mame/layout",
	}

files{
	MAME_DIR .. "src/mame/drivers/spc1000.cpp",
	MAME_DIR .. "src/mame/includes/spc1000.h",
}
end

function linkProjects_mame_spc(_target, _subtarget)
	links {
		"mame_spc",
	}
end

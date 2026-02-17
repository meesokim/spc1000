all: bin

CXXSOURCES = main.cpp mc6847.cpp keyboard.cpp cpu.cpp cassette.cpp
CSOURCES = spcall.c emu2149.c ugui.c miniz/miniz.c miniz/miniz_zip.c miniz/miniz_tdef.c miniz/miniz_tinfl.c
PSOURCES = $(CSOURCES) bzlib.c crctable.c

TAPE_DIR?=taps

# Compiler selection based on target
WASM_CC = emcc
WASM_CXX = em++
BIN_CC = gcc
BIN_CXX = g++
CIRCLE_CC = arm-none-eabi-gcc
CIRCLE_CXX = arm-none-eabi-g++

# Target specific rules
%.wasm.o: %.c
	$(WASM_CC) -O2 -sUSE_SDL=2 -sUSE_BZIP2 -DTAPE_DIR=\"$(TAPE_DIR)\" -c $< -o $@

%.wasm.o: %.cpp
	$(WASM_CXX) -std=c++17 -O2 -sUSE_SDL=2 -sUSE_BZIP2 -DTAPE_DIR=\"$(TAPE_DIR)\" -c $< -o $@ -Iminiz

%.bin.o: %.c
	$(BIN_CC) -I/usr/include/SDL2 -Iminiz -O2 -DTAPE_DIR=\"$(TAPE_DIR)\" -c $< -o $@

%.bin.o: %.cpp
	$(BIN_CXX) -std=c++17 -I/usr/include/SDL2 -Iminiz -O2 -DTAPE_DIR=\"$(TAPE_DIR)\" -c $< -o $@ -Iminiz

%.circle.o: %.c
	$(CIRCLE_CC) -c -Wall -O2 -ffreestanding -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -Iminiz $< -o $@

%.circle.o: %.cpp
	$(CIRCLE_CXX) -c -Wall -O2 -marm -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=softfp -fsigned-char -D__circle__ -ISDL2 -I../bzip2 -fpermissive -Iminiz $< -o $@

WASM_OBJECTS = $(CXXSOURCES:.cpp=.wasm.o) $(CSOURCES:.c=.wasm.o)
BIN_OBJECTS = $(CXXSOURCES:.cpp=.bin.o) $(CSOURCES:.c=.bin.o)
TARGET_OBJECTS = $(CXXSOURCES:.cpp=.target.o) $(CSOURCES:.c=.target.o)

# SDL2 configuration
ifdef SYSROOT
  # WORKAROUND: Sysroot missing ARM headers, pointing to x86_64 headers for config
  # We copy the config file to a local directory to avoid including all x86_64 headers
  SDL2_CFLAGS = -I$(SYSROOT)/usr/include/SDL2 -Iheaders_sysroot -D_REENTRANT
  # WORKAROUND: Sysroot missing libSDL2.so symlink, linking against runtime library directly
  # WORKAROUND: Sysroot missing libbz2.so symlink, linking against runtime library directly
  SDL2_LIBS = -L$(SYSROOT)/usr/lib/arm-linux-gnueabihf -L$(SYSROOT)/usr/lib -L$(SYSROOT)/lib/arm-linux-gnueabihf $(SYSROOT)/lib/arm-linux-gnueabihf/libSDL2-2.0.so.0 $(SYSROOT)/lib/arm-linux-gnueabihf/libbz2.so.1.0
  TARGET_ARCH = -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm
else
  SDL2_CFLAGS = $(shell sdl2-config --cflags)
  SDL2_LIBS = $(shell sdl2-config --libs)
  TARGET_ARCH =
endif

%.target.o: %.c
	$(TARGET_CC) $(TARGET_ARCH) --sysroot=$(SYSROOT) -c $< -o $@ -DTAPE_DIR=\"$(TAPE_DIR)\"

%.target.o: %.cpp
	$(TARGET_CXX) $(TARGET_ARCH) --sysroot=$(SYSROOT) -std=c++17 $(SDL2_CFLAGS) -c $< -o $@ -Iminiz -DTAPE_DIR=\"$(TAPE_DIR)\"

prepare_headers:
ifdef SYSROOT
	mkdir -p headers_sysroot/SDL2
	cp $(SYSROOT)/usr/include/x86_64-linux-gnu/SDL2/_real_SDL_config.h headers_sysroot/SDL2/
	sed -i 's/#define HAVE_IMMINTRIN_H 1/\/* #undef HAVE_IMMINTRIN_H *\//g' headers_sysroot/SDL2/_real_SDL_config.h
endif

target: prepare_headers $(TARGET_OBJECTS)
	$(TARGET_CXX) $(TARGET_ARCH) --sysroot=$(SYSROOT) $(TARGET_OBJECTS) $(SDL2_LIBS) -DSPC1000 -o spc1000

bin: $(BIN_OBJECTS)
	$(BIN_CXX) $(BIN_OBJECTS) -o main -lc -lSDL2 -lstdc++ -lbz2

circle: $(CIRCLE_OBJECTS)
	$(CIRCLE_CXX) $(CIRCLE_OBJECTS) -T raspberry.ld -nostartfiles -fno-exceptions -fno-unwind-tables -fno-rtti -Wl,-Map=kernel.map bzlib.o crctable.o libkernel.a -lc libSDL2.a -o kernel.elf

clean:
	rm -f *.js miniz/*.o *.o *.wasm *.data main kernel.elf *.bin.o *.wasm.o *.circle.o *.target.o headers_sysroot/SDL2/_real_SDL_config.h
	rmdir headers_sysroot/SDL2 headers_sysroot 2>/dev/null || true
	rm -rf rootfs_overlay

deploy: target
	mkdir -p rootfs_overlay/usr/bin
	mkdir -p rootfs_overlay/usr/lib
	mkdir -p rootfs_overlay/root
	cp spc1000 rootfs_overlay/usr/bin/
	cp -r taps rootfs_overlay/root/
	# Auto-deploy all required libraries from sysroot
	python3 deploy_libs.py spc1000 $(SYSROOT) rootfs_overlay/usr/lib rootfs_overlay
	@echo "Deployment prepared in rootfs_overlay"
	@echo "Configure Buildroot to use this overlay via BR2_ROOTFS_OVERLAY"

pub:
	cp index.* ~/spc1000.wiki/docs

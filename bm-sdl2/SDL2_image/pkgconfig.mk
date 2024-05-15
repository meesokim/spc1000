
CFLAGS += -ISDL2_image

LDFLAGS += -LSDL2_image

LIBS := -lSDL2_image $(LIBS)
LIBS_DEP += SDL2_image/libSDL2_image.a

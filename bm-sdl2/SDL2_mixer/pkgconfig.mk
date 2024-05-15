
CFLAGS += -ISDL2_mixer

LDFLAGS += -LSDL2_mixer

LIBS := -lSDL2_mixer $(LIBS)
LIBS_DEP += SDL2_mixer/libSDL2_mixer.a

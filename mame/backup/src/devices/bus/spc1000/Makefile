TARGET=spcdrive

CXX?=g++
ifeq ($(TARGET_OS),windows)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	ZEMMIX_EXT=zxw
	LIBINFO=
	#FLAGS= 
	LDFLAGS=-static-libgcc -static
else ifeq ($(TARGET_OS),circle)
	CC=arm-none-eabi-gcc
	CXX=arm-none-eabi-g++
	ZEMMIX_EXT=zxw
	LIBINFO=
	CFLAGS=-fPIC -D__circle__ -DRASPPI=1
	LDFLAGS=-static-libgcc -static
else ifeq ($(TARGET_OS),linux)
	CC=gcc
	ZEMMIX_EXT=zxl
	LIBINFO=libinfo
	#FLAGS=-fpermissive -fPIC
	LDFLAGS=-ldl  ../lib/libbcm2835.a
endif
# FLAGS=-fPIC -Iinclude -I.
CXXFLAGS +=-fPIC -fpermissive
SOURCES = $(shell find -maxdepth 1 -name "*.c")
COBJS = $(shell echo $(SOURCES) | sed -r 's/\.s|\.c|\.cpp|.cc/\.o/g') 
DEBUGFLAGS = -O0 -D _DEBUG -shared ../lib/libbcm2835.a
RELEASEFLAGS = libbcm2835.a -O2 -D DEBUG -combine -fwhole-program -shared 

ifeq ($(TARGET_OS),circle)
OBJS = $(COBJS) tap.o spcdrive.o
else
OBJS = $(COBJS) tap.o spcdrive.o spc.o
endif

all: $(TARGET).$(ZEMMIX_EXT) $(LIBINFO)

libinfo: libinfo.o
	$(CC) $(FLAGS) -o $@ $^  -g $(LDFLAGS)

example: example.o spc.o
	$(CC) $(FLAGS) -o $@ $^  -g $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o: %.cc
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<

%.o: %.c 
	$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

$(TARGET).$(ZEMMIX_EXT): $(OBJS)
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -o $(TARGET).$(ZEMMIX_EXT) $(OBJS) $(LDFLAGS)

clean:
	$(RM) -f $(OBJS) $(TARGET).$(ZEMMIX_EXT) *.o

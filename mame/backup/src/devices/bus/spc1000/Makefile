TARGET=spcdrive

CXX?=g++
ifeq ($(TARGET_OS),windows)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	ZEMMIX_EXT=zxw
	LIBINFO=
	#FLAGS= 
	LDFLAGS=-static-libgcc -static
else
	CC=gcc
	ZEMMIX_EXT=zxl
	LIBINFO=libinfo
	#FLAGS=-fpermissive -fPIC
	LDFLAGS=-ldl  ../lib/libbcm2835.a
endif
FLAGS=-fPIC -Iinclude -I.
CXXFLAGS =-fPIC -fpermissive
SOURCES = $(shell find -maxdepth 1 -name "*.c")
COBJS = $(shell echo $(SOURCES) | sed -r 's/\.s|\.c|\.cpp|.cc/\.o/g') 
DEBUGFLAGS = -O0 -D _DEBUG -shared ../lib/libbcm2835.a
RELEASEFLAGS = libbcm2835.a -O2 -D DEBUG -combine -fwhole-program -shared 
OBJS = $(COBJS) tap.o spcdrive.o spc.o

all: $(TARGET).$(ZEMMIX_EXT) $(LIBINFO)

libinfo: libinfo.o
	$(CC) $(FLAGS) -o $@ $^  -g $(LDFLAGS)

example: example.o spc.o
	$(CC) $(FLAGS) -o $@ $^  -g $(LDFLAGS)

%.o: %.cc %.cpp 
	$(CXX) $(FLAGS) $(CXXFLAGS) -c -o $@ $^

%.o: %.c 
	$(CC) $(FLAGS) $(CFLAGS) -c -o $@ $<

$(TARGET).$(ZEMMIX_EXT): $(OBJS)
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -o $(TARGET).$(ZEMMIX_EXT) $(OBJS) $(LDFLAGS)

clean:
	$(RM) -f $(OBJS) $(TARGET).$(ZEMMIX_EXT) *.o

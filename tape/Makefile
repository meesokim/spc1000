CC=	gcc
CFLAGS=-O2 -Wall
LDFLAGS= -lncurses   
WAV2TAP=wav2tap
TAPINFO=tapinfo
APPLETAPE=apple1basic-decode
MMZX81=mmzx81

all: $(WAV2TAP) $(TAPINFO) $(APPLETAPE) $(MMZX81)

clean:
	rm -f $(WAV2TAP) $(TAPINFO) *.exe *.o

$(WAV2TAP).o: $(WAV2TAP).c
$(WAV2TAP): $(WAV2TAP).o
	$(CC) -g -O2 -o $(WAV2TAP) $(WAV2TAP).o $(LDFLAGS)

$(TAPINFO).o: $(TAPINFO).c
$(TAPINFO): $(TAPINFO).o
	$(CC) -g -o $(TAPINFO) $(TAPINFO).o

$(APPLETAPE).o: $(APPLETAPE).c
$(APPLETAPE): $(APPLETAPE).o
	$(CC) -o $(APPLETAPE) $(APPLETAPE).o

$(MMZX81).o: $(MMZX81).cpp
$(MMZX81): $(MMZX81).o
	$(CXX) -o $(MMZX81) $(MMZX81).o


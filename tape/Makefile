CC=	gcc
CFLAGS=-O2 -Wall
LDFLAGS= -lncurses   
WAV2TAP=wav2tap
TAPINFO=tapinfo

all:
	make $(WAV2TAP)
	make $(TAPINFO)

clean:
	rm -f $(WAV2TAP) $(TAPINFO) *.exe *.o

$(WAV2TAP).o: $(WAV2TAP).c
$(WAV2TAP): $(WAV2TAP).o
	$(CC) -g -O2 -o $(WAV2TAP) $(WAV2TAP).o $(LDFLAGS)

$(TAPINFO).o: $(TAPINFO).c
$(TAPINFO): $(TAPINFO).o
	$(CC) -o $(TAPINFO) $(TAPINFO).o

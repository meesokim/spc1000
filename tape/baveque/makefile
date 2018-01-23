NAME  = baveque
CC1 = sdcc
AS = sdasz80
TAPINFO = ../tapinfo
HEX2BIN = ../hex2bin
BIN2TAP = ../bin2tap
SOURCE = $(NAME)-mayhouse.tap
TARGET = $(NAME)-rpibox.tap

SRC = 2_BAVEQUE.s
OBJ = $(SRC:.s=.rel)
CFLAGS = -mz80
LDFLAGS	= -mz80

all: $(TARGET)

$(TARGET):  $(OBJ)
	$(CC1) $(OBJ) -o $(NAME).ihex
	$(HEX2BIN) $(NAME).ihex
	$(BIN2TAP) $(NAME).bin $(NAME).tap 0x1a3
	$(TAPINFO) $(SOURCE) -
	cat $(NAME).tap 4_B.tap 3_.tap > $(TARGET)

%.o: %.c
	$(CC2) -c $< $(CFLAGS3) -o $@
%.rel: %.c
	$(CC1) -c $< $(CFLAGS) -o $@
%.rel: %.s
	$(AS) -l -o $< 
	
clean:
	@rm -f *.bin *.ihex *.lk *.mem *.lst *.rel *.rst *.map
	@find . -type f -not -name '$(SOURCE)' -name '*.tap' -delete
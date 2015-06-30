CC = gcc

SRC1 = buffer.c parse5.c parse6.c parse1.c txt2bas.c 
OBJ1 = $(SRC1:.c=.o)

all: txt2bas bin2tap bas2tap tapinfo wav2tap tap2cas cas2tap

txt2bas: $(OBJ1)
	$(CC) -o $@ $^
	
bin2tap:
	$(CC) -o $@ $@.c
	
bas2tap:
	$(CC) -o $@ $@.c
	
tapinfo:
	$(CC) -o $@ $@.c

tap2cas:
	$(CC) -o $@ $@.c

cas2tap:
	$(CC) -o $@ $@.c
	
wav2tap:
	$(CC) -o $@ $@.c
	
%.o: %.c
	$(CC) -c $< -o $@	

clean:
	rm -rf txt2bas *.o *.exe

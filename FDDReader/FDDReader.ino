#include <stdarg.h>
void println(char *fmt, ... ){
    char buf[128]; // resulting string limited to 128 chars
    va_list args;
    va_start (args, fmt );
    vsnprintf(buf, 128, fmt, args);
    va_end (args);
    Serial.print(buf);
}

enum FDD_CMD {
    FDUINZ,
    WRTDSK,
    RDDSK,
    TRNBUF,
    COPY,
    FORMAT,
    CMDSTAT,
    DRVSTAT,
    TSTMEM,
    READMEM,
    OUTPORT,
    GETMEM,
    SETMEM,
    EXEUSR,
    LOADMEM,
    SAVEMEM,
    LOADGO,
    FWRDSK,
    CMDMAX
};

#define RD    4
#define CS    5
#define WR    6
#define AD0   7
#define AD1   8
#define RESET 9
#define D0    14
#define D1    15
#define D2    16
#define D3    17
#define D4    18
#define D5    19
#define D6    20
#define D7    21
#define EXT1  3
#define IORQ  2
#define MREQ  10
#define BUSAK 11

#define SPC_ATN  PC7
#define SPC_DAC  PC6
#define SPC_RFD  PC5
#define SPC_DAV  PC4
#define FDD_DAC  PC2
#define FDD_RFD  PC1
#define FDD_DAV  PC0

#define ATN_S 1 << 7
#define DAC_S 1 << 6
#define RFD_S 1 << 5
#define DAV_S 1 << 4
#define DAC_F 1 << 2
#define RFD_F 1 << 1
#define DAV_F 1 

#define PORTA 0
#define PORTB 1
#define PORTC 2

void out(byte addr, byte data)
{
  digitalWrite(AD0, (addr & 1));
  digitalWrite(AD1, (addr & 2));
  digitalWrite(D0, (data & 1));
  digitalWrite(D1, (data & 1 << 1));
  digitalWrite(D2, (data & 1 << 2));
  digitalWrite(D3, (data & 1 << 3));
  digitalWrite(D4, (data & 1 << 4));
  digitalWrite(D5, (data & 1 << 5));
  digitalWrite(D6, (data & 1 << 6));
  digitalWrite(D7, (data & 1 << 7));
  digitalWrite(RD, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(IORQ, HIGH);
  digitalWrite(EXT1, HIGH);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  digitalWrite(EXT1, LOW);
  digitalWrite(IORQ, LOW);
  digitalWrite(WR, LOW);
  delay(1);
  digitalWrite(EXT1, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(IORQ, HIGH);
  println("out(%d),%02x\n", addr, data);
}

byte in(byte addr)
{
  register byte b = 0;
  digitalWrite(AD0, (addr & 1));
  digitalWrite(AD1, (addr & 2));
  digitalWrite(RD, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(IORQ, HIGH);
  digitalWrite(EXT1, HIGH);
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);  
  digitalWrite(EXT1, LOW);
  digitalWrite(IORQ, LOW);
  digitalWrite(RD, LOW);
  delay(1);
  b = digitalRead(D0) + digitalRead(D1)<<1 + digitalRead(D2)<<2 + digitalRead(D3)<<3 
    + digitalRead(D4)<<4 + digitalRead(D5)<<5 + digitalRead(D6)<<6 + digitalRead(D7)<<7;
  digitalWrite(EXT1, HIGH);
  digitalWrite(RD, HIGH);
  digitalWrite(IORQ, HIGH);
  println("in(%d)->%02x\n", addr, b);
  return b;
}

void sendcmd(byte b)
{
  out(PORTC, 0x80);
  senddata(b);
}

byte recvdata()
{
  register byte b; 
  out(PORTC, 0x02);
  while(in(PORTC) != 0x2);
  out(PORTC, 0);
  b = in(PORTB);
  out(PORTC, 0x2);
  while(in(PORTC) != 1);
  out(PORTC, 0);
  return b;
}

void senddata(byte b)
{
  while(in(PORTC)!=0x2);
  out(PORTC, 0);
  out(PORTA, b);
  out(PORTC, 0x10);
  while(in(PORTC) != 0x4);
  out(PORTC, 0);
  while(in(PORTC) != 0x4);
}

int sd_init()
{
  do {
    out(PORTC,0);
    out(PORTA,0);
    sendcmd(FDUINZ);
    sendcmd(DRVSTAT);
  } while (recvdata() != 0x10);
}

int sd_format(byte b)
{
  sendcmd(FORMAT);
  return recvdata();
}

int sd_drvstate()
{
  sendcmd(DRVSTAT);
  return recvdata();
}

int sd_cmdstate()
{
  sendcmd(CMDSTAT);
  return recvdata();
}

int sd_read(byte blocks, byte drive, byte track, byte sector, byte *buf)
{
  int len = blocks * 256;
  sendcmd(RDDSK);
  senddata(blocks);
  senddata(drive);
  senddata(track);
  senddata(sector);
  while(len--)
  {
    *buf = recvdata();
  }
  return 0;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(RESET, OUTPUT);
  pinMode(EXT1, OUTPUT);
  pinMode(IORQ, OUTPUT);
  pinMode(MREQ, OUTPUT);
  pinMode(RD, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(AD0, OUTPUT);
  pinMode(AD1, OUTPUT);
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(EXT1, OUTPUT);
  digitalWrite(EXT1, HIGH);
  digitalWrite(IORQ, HIGH);
  digitalWrite(MREQ, HIGH);
  digitalWrite(RD, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(RESET, HIGH);
  delay(10);
  digitalWrite(RESET, LOW);
  Serial.begin(115200);
  while(!Serial);  
  Serial.println("SD725 initialing\n");
  sd_init();
  Serial.println("SD725 initialized.\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  byte buf[256];
  sd_read(1,0,0,1, buf);
  for(int i=0; i < 16; i++)
  {
    println("%04x:", i * 16);
    for(int j=0; j < 16; j++)
      println("%02x ", buf[i * 16 + j]);
  }
 println("\n");
}

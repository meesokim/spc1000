#include <SPI.h>

#define RD    4
#define CS    5
#define WR    6
#define AD0   7
#define AD1   8
#define RESET 9
#define SPC_RESET 2

#define D0    14
#define D1    15
#define D2    16
#define D3    17
#define D4    18
#define D5    19
#define D6    20
#define D7    21

#define PA_O  PORTF
#define PA_I  PINF
#define PA_DO DDRF = 0xff
#define PA_DI DDRF = 0
#define PA0   A0
#define PA1   A1
#define PA2   A2
#define PA3   A3
#define PA4   A4
#define PA5   A5
#define PA6   A6
#define PA7   A7

#define PB_O  PORTA
#define PB_I  PINA
#define PB_DO DDRA = 0xff
#define PB_DI DDRA = 0
#define PB0   22
#define PB1   23
#define PB2   24
#define PB3   25
#define PB4   26
#define PB5   27
#define PB6   28
#define PB7   29

#define PC_O  PORTC
#define PC_I  PINC
#define PC_DO DDRC = 0xff
#define PC_DI DDRC = 0
#define PC_DIR DDRC
#define PC0   37
#define PC1   36
#define PC2   35
#define PC3   34
#define PC4   33
#define PC5   32
#define PC6   31
#define PC7   30
#define SDSS  53

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

#include <SD.h>

void reset8255()
{
  digitalWrite(RESET, HIGH);
  digitalWrite(RESET,LOW);
  digitalWrite(WR, HIGH);
  digitalWrite(RD, HIGH);
  
  digitalWrite(D0, HIGH); 
  //Set port b to output
  digitalWrite(D1, HIGH);
  //Set port b to mode 0
  digitalWrite(D2, LOW);
  //Set port c to output 4bit
  digitalWrite(D3, LOW);
  //Set port a to output
  digitalWrite(D4, LOW);
  //Set port a to mode 0
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);  
  
  digitalWrite(AD0, HIGH);
  digitalWrite(AD1, HIGH);
  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  while(Serial.available()==0);
  digitalWrite(WR, HIGH);
  digitalWrite(CS, HIGH);
}

void set8255(byte reg, byte data)
{
  digitalWrite(WR, HIGH);
  digitalWrite(CS, LOW);
  digitalWrite(WR, LOW);
  digitalWrite(AD0, reg & 1 > 0);
  digitalWrite(AD1, reg & 2 > 0);
  for (int i = 0; i < 8; i++) {
    pinMode(D0 + i, OUTPUT);
    digitalWrite(D0 + i, (data & (1 << i)) > 0);
  }
  digitalWrite(WR, HIGH);
  digitalWrite(RD, HIGH);
  digitalWrite(CS, HIGH);
  digitalWrite(AD0, LOW);
  digitalWrite(AD1, LOW);
}
byte get8255(byte reg)
{
  byte val = 0;
  digitalWrite(CS, LOW);
  digitalWrite(AD0, reg & 1);
  digitalWrite(AD1, reg & 2);
  digitalWrite(RD, LOW);
  for (int i = 0; i < 8; i++) {
    pinMode(D0 + i, INPUT);
    val |= digitalRead(D0 + i) << i;
  }
  digitalWrite(RD, HIGH);
  digitalWrite(CS, HIGH);
  return val;
}
void close8255()
{
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(CS, INPUT);
  pinMode(AD0, INPUT);
  pinMode(AD1, INPUT);
  pinMode(WR, INPUT);
  pinMode(RD, INPUT);
}

void open8255()
{
  pinMode(RESET, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(RD, OUTPUT);
  pinMode(CS, OUTPUT);
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(AD0, OUTPUT);
  pinMode(AD1, OUTPUT);  
  pinMode(PA0, INPUT);
  pinMode(PA1, INPUT);
  pinMode(PA2, INPUT);
  pinMode(PA3, INPUT);
  pinMode(PA4, INPUT);
  pinMode(PA5, INPUT);
  pinMode(PA6, INPUT);
  pinMode(PA7, INPUT);
  PB_DO;
  PA_DI;
  PB_O = 0;
  PA_O = 0xff;
  PC_DIR = 0xf;
  PC_O = 0xf0;
}

byte getPortA()
{
  byte val = 0;
  int i;
  const byte pin[8] = {PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7 };
  for(int i=0; i<8; i++)
    val |= digitalRead(pin[i]) << i;
  return val;  
}
byte fs = 0, cmd = 255, len,q,rq;

void setup() {
  // put your setup code here, to run once:
  byte adata;
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  open8255();
  reset8255();
  set8255(3, 0x83);
  adata = random();
  set8255(0, adata);
  //PA_O = adata;
  Serial.print("PA Test val=");
  Serial.print(adata);
  Serial.print(", actual=");
  Serial.print(get8255(0));
  Serial.print(", getPortA=");
  Serial.print(getPortA());
  Serial.print(", port=");
  Serial.println(PA_I);
  adata = random();
  set8255(1, adata);
  adata = PB_I;
  Serial.print("PB Test val=");
  Serial.print(adata);
  Serial.print(", actual=");
  Serial.print(get8255(1));
  Serial.print(", port=");
  Serial.println(PA_I);
  set8255(2, 0xff);
  PB_O = 0;
  PC_O = 0x0;
  Serial.print("PC Test val=0xff, actual=");
  Serial.println(get8255(2));
  close8255();
  pinMode(SPC_RESET, INPUT);
  pinMode(SPC_ATN, INPUT);
  pinMode(SPC_DAC, INPUT);
  pinMode(SPC_RFD, INPUT);
  pinMode(SPC_DAV, INPUT);
  pinMode(FDD_DAC, OUTPUT);
  pinMode(FDD_RFD, OUTPUT);
  pinMode(FDD_DAV, OUTPUT);
  digitalWrite(FDD_DAC, LOW);
  digitalWrite(FDD_RFD, LOW);
  digitalWrite(FDD_DAV, LOW);
  digitalWrite(SPC_ATN, HIGH);
  digitalWrite(SPC_DAC, HIGH);
  digitalWrite(SPC_RFD, HIGH);
  digitalWrite(SPC_DAV, HIGH);
  PB_O = 0;
  Serial.print("Initializing SD card...");
  // On the Ethernet Shield, CS is pin 4. It's set as an output by default.
  // Note that even if it's not used as the CS pin, the hardware SS pin
  // (10 on most Arduino boards, 53 on the Mega) must be left as an output
  // or the SD library functions will not work.
  pinMode(SDSS, OUTPUT);
  if (!SD.begin(SDSS)) {
    Serial.println("initialization failed!");
    return;
  }
  else
  {
    Serial.println("Sucess!");
  }
  attachInterrupt(0, pcReset, RISING);
  //Serial.println(PINA);
}

void pcReset()
{
//  open8255();
//  reset8255();
//  close8255();
  PB_O = 0;
}

enum FDD_STATUS {
  NONE,
  ATTENSION,
  DATAVALID,
  DATACOMPLETED,
  DATAREQ,
  DATASEND,
};

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

#include <stdarg.h>
void p(char *fmt, ... ){
    char buf[128]; // resulting string limited to 128 chars
    va_list args;
    va_start (args, fmt );
    vsnprintf(buf, 128, fmt, args);
    va_end (args);
    Serial.print(buf);
}

int adata = 0, bdata;
byte cmdlen[15] = {0,0,4,0,0,0,0,0,0,0,0,0,0,0,0};
byte cmdque[512];
byte resque[512];
int reslen = 0;
int cval = 0;
void loop() {
  // put your main code here, to run repeatedly:
  byte pc = PC_I;
  //if (pc != cval) p("PC=%02x\n", cval = PC_I);
  if (pc & ATN_S)
  {
     //if (fs != ATTENSION) p("Attension:%d\n", fs);
     PC_O = 0;
     digitalWrite(FDD_RFD, HIGH);     
     fs = ATTENSION;
  } 
  else if (!(pc & ATN_S) && (pc & DAV_S) && (fs == DATAVALID || fs == ATTENSION))
  {
     digitalWrite(FDD_RFD, LOW);
     byte data = getPortA();
     p("Write=%d\n", data);
     PC_O = 0;
     digitalWrite(FDD_DAC, HIGH);
     fs = DATACOMPLETED;
     if (cmd > CMDMAX)
     {
       cmd = data;
       p("Command:%d received.\n", cmd);
       len = cmdlen[cmd];
       if (len == 0)
         do_cmd(cmd);
       q = 0;
       cmd = 255;
     } else {
       p("Command:%d continued.\n", cmd);
       if (len > 0)
       {
         cmdque[q++] = data;
       } else
       {
         rq = 0;
         do_cmd(cmd);
       }
     }
  } else if (!(pc & DAV_S) && fs == DATACOMPLETED)
  {
     digitalWrite(FDD_DAC, LOW);
     //Serial.println("Write Completed");
     //p("Write Completed:%d\n", fs);
     fs = NONE;
  } else if (pc & RFD_S)
  {
     //if (fs != DATAREQ) p("Data Request:%d\n", fs);
     fs = DATAREQ;
     PC_O = 0;
     digitalWrite(FDD_DAV, HIGH);
  } else if (!(pc & RFD_S) && fs == DATAREQ)
  {
    byte data = resque[rq++];
    p("Read=%d\n", data);
     PB_O = data;
     digitalWrite(FDD_DAV, LOW);
     fs = DATASEND;
  } else if ((pc & DAC_S) && fs == DATASEND)
  {
//     Serial.println("Read Completed");
     fs = NONE;
  }
}

void do_cmd(byte cmd)
{
  switch (cmd)
  {
    case FDUINZ:
      p("FDUINZ: FDD Initialized\n");
      break;
    case WRTDSK:
      break;
    case RDDSK:
      byte nsect, drive, track, sector;
      p("RDDSK: Read Disk\n");
      nsect = cmdque[0];
      drive = cmdque[1];
      track = cmdque[2];
      sector = cmdque[3];
      p("size(#ofsectors)=%d,drive=%d, track=%d, sector=%d\n", nsect, drive, track, sector);
      
      break;
    case TRNBUF:
      break;
    case COPY:
      break;
    case FORMAT:
      break;
    case CMDSTAT:
      break;
    case DRVSTAT:
      p("DRVSTAT: DRIVE Status\n");
      resque[0] = 0xff;
      rq = 0;
      break;
    case TSTMEM:
      break;
    case READMEM:
      break;
    case OUTPORT:
      break;
    case GETMEM:
      break;
    case SETMEM:
      break;
    case EXEUSR:
      break;
    case LOADMEM:
      break;
    case SAVEMEM:
      break;
    case LOADGO:
      break;
    case FWRDSK:
      break;
  };
  cmd = 255;
}


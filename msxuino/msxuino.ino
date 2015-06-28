#define CS1      3
#define CS2      4
#define CS12     5
#define SLTSL    6
#define WAITREL  7
#define WT       8
#define M1       9
#define BUSDIR   6
#define IORQ     14
#define MERQ     15
#define WR       16
#define RD       17
#define DATAW    PORTF
#define DATAR    PINF
#define ADDRL    PINC
#define ADDRH    PINA

const byte ROMS[] PROGMEM = {
#include "SKYJAGUA.h"
};
unsigned int addr;
byte b;
byte d;

void setup() {
  // put your setup code here, to run once:
  pinMode(WAITREL, OUTPUT);
  pinMode(BUSDIR, OUTPUT);
  pinMode(WT, INPUT);
  pinMode(RD, INPUT);
  pinMode(WR, INPUT);
  digitalWrite(WAITREL, HIGH);
  digitalWrite(BUSDIR, HIGH);
  Serial.begin(115200);
  Serial.println(F("MSXuino initialized"));
  DDRF = 0;
  DDRA = 0;
  DDRC = 0;
  digitalWrite(WAITREL, LOW);
  digitalWrite(WAITREL, HIGH);
}

#define getAddressCS1() ((ADDRH & 0x3f) << 8) | ADDRL
#define getAddressCS2() ((ADDRH & 0x7f) << 8) | ADDRL
#define getAddress() (ADDRH << 8) | ADDRL

void loop() {
  byte h, l, a;
  int c = 0;
  // put your main code here, to run repeatedly:
  //digitalWrite(WAITREL, HIGH);
  while (!digitalRead(WT)) {
//    if (c++ % 100000 == 0) { digitalWrite(WAITREL, LOW); digitalWrite(WAITREL, LOW); }
  }

  h = ADDRH;
  l = ADDRL;
//  a = digitalRead(RD)<<0|digitalRead(WR)<<1|digitalRead(IORQ)<<2|digitalRead(MERQ);
//    Serial.print(!digitalRead(RD)?"R":"X");
//    Serial.print(!digitalRead(WR)?"W":"X");
//    Serial.print(!digitalRead(IORQ)?"I":"X");
//    Serial.print(!digitalRead(MERQ)?"M":"X");

  if (!digitalRead(IORQ))
  {
    if (l == 128)
    {
      if (!digitalRead(RD))
      {
        DDRF = 0xff;
        PORTF = d++;
//        digitalWrite(BUSDIR, LOW);
//        digitalWrite(BUSDIR, HIGH);
//        digitalWrite(WAITREL, LOW);
        PORTH = 0;
        PORTH = 0;
        asm volatile ("nop\nnop\nnop\n");
    //    PORTH = 1<<4;
        PORTH = 1<<4|1<<3;
        DDRF = 0;
//        Serial.print("OIRQ");
  //      Serial.println(l , HEX);
      }
      else {
        Serial.print("OIRQ");
        b = PINF;
        Serial.println(b , HEX);
        digitalWrite(WAITREL, LOW);
      }
    }
  }

  else if (h >= 0x40 && h < 0x80 )
  {
//    Serial.print(!digitalRead(RD)?"R":"X");
//    Serial.print(!digitalRead(WR)?"W":"X");
//    Serial.print(!digitalRead(IORQ)?"I":"X");
//    Serial.print(!digitalRead(MERQ)?"M":"X");
    addr = (h - 0x40) << 8 | l;
    DDRF = 0xff;
    PORTF = b = pgm_read_byte_near(ROMS + addr);
//    Serial.print(h, HEX);
//    Serial.println(l , HEX);
    Serial.println(b, HEX);
    digitalWrite(WAITREL, LOW);
    while(!digitalRead(RD)) if (c++ % 10 == 0) break;
    DDRF = 0;
  }
  /*
  if (!digitalRead(15))
  {
    h = ADDRH;
    l = ADDRL;
    addr = h << 8 | l;   
    b = ROMS[addr];
    DDRF = 0xff;
    PORTF = b;
    Serial.print(addr>>8, HEX);
    Serial.println(addr & 0xff, HEX);
    digitalWrite(WAITREL, LOW);
    while (!digitalRead(RD));
    DDRF = 0;
//    Serial.println(addr);
//    interrupts();
   //digitalWrite(WAITREL, HIGH);
    //continue;
  }
  else if (!digitalRead(16))
  {
    Serial.print("IO");
    Serial.println(ADDRL, HEX);
  }
  else
  {
    Serial.println("Known");
  }
  */
  digitalWrite(WAITREL, LOW);
  digitalWrite(WAITREL, HIGH);
}

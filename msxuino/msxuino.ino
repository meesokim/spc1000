#define CS1      3
#define CS2      4
#define CS12     5
#define SLTSL    6
#define WAITREL  7
#define INT      8  
#define M1       9
#define BUSDIR   10
#define IORQ     11
#define MERQ     12
#define WR       13
#define RD       14
#define DATAW    PORTF
#define DATAR    PINF
#define ADDRL    PINC
#define ADDRH    PINA

volatile const byte ROMS[] PROGMEM = {
#include "SKYJAGUA.h" 
};

void setup() {
  // put your setup code here, to run once:
  pinMode(WAITREL, OUTPUT);
  pinMode(INT, OUTPUT);
  pinMode(BUSDIR, OUTPUT);
  digitalWrite(WAITREL, HIGH);
  digitalWrite(INT, HIGH);
  digitalWrite(BUSDIR, HIGH);
  Serial.begin(115200);
  Serial.println(F("MSXuino initialized"));
}

void loop() {
  // put your main code here, to run repeatedly:
  int addr;
  digitalWrite(WAITREL, HIGH);
  while(digitalRead(SLTSL));
  if (!digitalRead(MERQ))
  {
    if (!digitalRead(RD) & !digitalRead(CS1))
    {
      digitalWrite(BUSDIR, LOW);
      DDRF = 0xff;
      addr = (ADDRH << 8 - 0x40) + ADDRL;
      DATAW = ROMS[addr];
      digitalWrite(WAITREL,LOW);
      while(!digitalRead(SLTSL));
      digitalWrite(BUSDIR, HIGH);
      DDRF = 0;
      Serial.println(addr);
    }
  }
}

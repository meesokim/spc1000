#define CS1      3
#define CS2      4
#define CS12     5
#define SLTSL    6
#define WAITREL  7
#define WT       8
#define M1       9
#define BUSDIR   10
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
unsigned short addr;
byte b;

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
  addr = 0;
  b = ROMS[addr];
  Serial.println(b);
  DDRF = 0;
  DDRA = 0;
  DDRC = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  //  digitalWrite(WAITREL, HIGH);
  while (!digitalRead(WT));
  Serial.println(digitalRead(8));
  if (!digitalRead(RD))
  {
    DDRF = 0xff;
    addr = (ADDRH) << 8 + ADDRL;
    b = ROMS[addr];
    PORTK = b;
    digitalWrite(BUSDIR, LOW);
    digitalWrite(WAITREL, LOW);
    digitalWrite(WAITREL, HIGH);
    while (!digitalRead(SLTSL));
    digitalWrite(BUSDIR, HIGH);
    DDRF = 0;
    Serial.println(addr);
    Serial.println(ADDRH);
    Serial.println(b);
  }
  else
  {
    digitalWrite(WAITREL, LOW);
    digitalWrite(WAITREL, HIGH);
  }
}

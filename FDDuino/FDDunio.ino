#include <SPI.h>

#define CS    A0
#define AD1    4
#define AD0    3
#define RESET 5
#define D0    6
#define D1    7
#define D2    8
#define D3    9
#define D4    14
#define D5    15
#define D6    16
#define D7    17
#define PA0   22
#define PA1   23
#define PA2   24
#define PA3   25
#define PA4   26
#define PA5   27
#define PA6   28
#define PA7   29
#define PB0   37
#define PB1   36
#define PB2   35
#define PB3   34
#define PB4   35
#define PB5   36
#define PB6   37
#define PB7   38
#define PC0   21
#define PC1   20
#define PC2   19
#define PC3   18
#define PC4   10
#define PC5   11
#define PC6   12
#define PC7   13
#define SPC_RESET 2
#define WR    38
#define SDSS  53

#define SPC_ATN  PC7
#define SPC_DAC  PC6
#define SPC_RFD  PC5
#define SPC_DAV  PC4
#define FDD_DAC  PC2
#define FDD_RFD  PC1
#define FDD_DAV  PC0

#include <SD.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(RESET, OUTPUT);
  pinMode(WR, OUTPUT);
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
  digitalWrite(CS, HIGH);
  digitalWrite(WR, HIGH);
  digitalWrite(RESET, HIGH);
  delay(500);
  digitalWrite(RESET, LOW);
  digitalWrite(CS, LOW);
  digitalWrite(AD0, HIGH);
  digitalWrite(AD1, HIGH);
  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, HIGH);
  digitalWrite(WR, LOW);
  delay(500);
  digitalWrite(WR, HIGH);
  digitalWrite(CS, HIGH);
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
  pinMode(SPC_RESET, INPUT);
  DDRA = 0; // PORT-A is output from SPC
  PORTA = 0;
  DDRC = 0xff; // PORT-B is input to SPC
  PORTC = 0;
  pinMode(SPC_ATN, INPUT);
  pinMode(SPC_DAC, INPUT);
  pinMode(SPC_RFD, INPUT);
  pinMode(SPC_DAV, INPUT);
  pinMode(FDD_DAC, OUTPUT);
  pinMode(FDD_RFD, OUTPUT);
  pinMode(FDD_DAV, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

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
  Serial.println(PINA);

}

int adata = 0, bdata;
void loop() {
  // put your main code here, to run repeatedly:
  if (adata != PINA)
  {
    Serial.print("PORT-A:");
    Serial.print(adata=PINA);
    Serial.println(" ");
  }
  if (bdata != PINC)
  {
    Serial.print("PORT-B:");
    Serial.print(bdata=PINA);
    Serial.println(" ");
  }
/*  
    if (digitalRead(SPC_ATN))
    {
       Serial.println("Attension");
       digitalWrite(FDD_RFD, HIGH);
       while(digitalRead(SPC_ATN));
       Serial.println("Data Valid");
       while(!digitalRead(SPC_DAV));
       digitalWrite(FDD_RFD, LOW);
       data = PINA;
       Serial.print("Write=");
       Serial.println(data);
       digitalWrite(FDD_DAC, HIGH);
       while(!digitalRead(SPC_DAV));
       digitalWrite(FDD_DAC, LOW);
    }
    else if (digitalRead(SPC_RFD))
    {
       digitalWrite(FDD_DAV, HIGH);
       while(digitalRead(SPC_RFD));
       Serial.print("Read=");
       Serial.print(data);
       PORTC = data;
       while(!digitalRead(SPC_DAC));
       digitalWrite(FDD_DAV, LOW);
       while(digitalRead(SPC_DAC));
    }
    */
}

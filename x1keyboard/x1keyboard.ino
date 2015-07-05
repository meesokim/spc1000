
#include <PS2Keyboard.h>

#define DATA_PIN 4
const int IRQpin =  3;
PS2Keyboard keyboard;
byte caps = 0;

void setup() {
  // put your setup code here, to run once:

  keyboard.begin(DATA_PIN, IRQpin);

  Serial.begin(115200);
  while (!Serial1);
  Serial.println("hi");

  //delay(1000);  
}

void loop() {
  if (keyboard.available()) {
    
    // read the next key
    char c = keyboard.read();
    
    // check for some of the special keys
    if (c == PS2_ENTER) {
      Serial.println();
    } else if (c == PS2_TAB) {
      Serial.print("[Tab]");
    } else if (c == PS2_ESC) {
      Serial.print("[ESC]");
    } else if (c == PS2_PAGEDOWN) {
      Serial.print("[PgDn]");
    } else if (c == PS2_PAGEUP) {
      Serial.print("[PgUp]");
    } else if (c == PS2_LEFTARROW) {
      Serial.print("[Left]");
    } else if (c == PS2_RIGHTARROW) {
      Serial.print("[Right]");
    } else if (c == PS2_UPARROW) {
      Serial.print("[Up]");
    } else if (c == PS2_DOWNARROW) {
      Serial.print("[Down]");
    } else if (c == PS2_DELETE) {
      Serial.print("[Del]");
    } else if (c == PS2_F1) {
      Serial.print("[F1]");
    } else if (c == PS2_CAPS) {
      caps = !caps;
 //     kbd_send_command(0xED);
 //     kbd_send_command(caps ? 0x4 : 0x0);    
    } else {
      
      // otherwise, just print all normal characters
      Serial.print(c);
    }
  }
}


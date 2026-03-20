#define RW 3
// include the library code:
#include <LiquidCrystal.h>
#define SERIAL_TIMEOUT 1000
#define SERIAL_BUFSIZE 50
#define LOOP_DELAY 500
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 46, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
bool led_state;
char ser_recvbuf[SERIAL_BUFSIZE];
void setup() {
  pinMode(RW, OUTPUT);
  digitalWrite(RW, LOW);
  pinMode(LED_BUILTIN, OUTPUT); //for heartbeat
  led_state = 0;
  digitalWrite(LED_BUILTIN, led_state);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.setCursor(0, 1);
  lcd.print("HF 901P Readout v0.0");

  Serial2.begin(9600);
  Serial.begin(9600);
  delay(1000);
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  lcd.setCursor(0, 1);
  bool user_input = 0;
  //read the current combined pressure value
  int user_bytes = Serial.available();
  if (user_bytes > 0) {
    for (int i=0; i<user_bytes; i++) {
      Serial2.write(Serial.read());
    }
    user_input = 1;
  } else {
    Serial2.write("@253PR4?;FF");
    user_input = 0;
  }
  //wait for response
  int ser_timeout = 0;
  int buf_idx = 0;
  while (!Serial2.available() && (ser_timeout < SERIAL_TIMEOUT)){
    delay(1);  
    ser_timeout++;
  }
  while (Serial2.available() && (buf_idx < SERIAL_BUFSIZE)) {
    ser_recvbuf[buf_idx] = Serial2.read();
    buf_idx++;
  }
  ser_recvbuf[buf_idx] = 0; //null terminate
  if (ser_timeout < SERIAL_TIMEOUT) {
    String bufstring;
    String numval;
    double floatval;
    char printbuf[50];
    bufstring = (String)ser_recvbuf;
    if (!user_input) {
      if (bufstring.substring(0,7) == "@253ACK") {
        numval = bufstring.substring(7, 15);
        floatval = numval.toFloat();
        sprintf(printbuf, "%.2f mTorr", floatval*1000);
        lcd.print(printbuf);
      } else {
        lcd.print("weird response, retry...\n");
      }
    }
    Serial.println(ser_recvbuf);
    Serial.println(numval);
    
  } else {
    lcd.print("no response, retry...\n");
  }
  delay(LOOP_DELAY);
//  digitalWrite(LED_BUILTIN, led_state);
//  led_state = !led_state;
}

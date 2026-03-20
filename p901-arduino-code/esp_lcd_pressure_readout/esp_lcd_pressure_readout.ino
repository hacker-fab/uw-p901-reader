#define RW 3
// include the library code:
#include <LiquidCrystal.h>
#define SERIAL_TIMEOUT 1000
#define SERIAL_BUFSIZE 50
#define LOOP_DELAY 500
#define NUM_AVGS 20
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 8, en = 46, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
bool led_state;
char ser_recvbuf[SERIAL_BUFSIZE];
double prev_reading; //in mTorr
double roc_avg_buf[NUM_AVGS];
int roc_avg_buf_idx;
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

  roc_avg_buf_idx = 0;

  Serial2.begin(9600);
  Serial.begin(9600);
  delay(1000);
  lcd.clear();
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
//  lcd.clear();
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
    double floatval; //in mTorr
    double roc; //in mTorr/s
    
    char printbuf[50];
    bufstring = (String)ser_recvbuf;
    if (!user_input) {
      if (bufstring.substring(0,7) == "@253ACK") {
        numval = bufstring.substring(7, 15);
        floatval = numval.toFloat()*1000;
        roc = (floatval - prev_reading) / ((double)LOOP_DELAY / 1000);
        roc_avg_buf[roc_avg_buf_idx] = roc;
        if (roc_avg_buf_idx == NUM_AVGS-1) {
          double sum = 0;
          double avg;
          for (int i=0; i<NUM_AVGS; i++) {
            sum += roc_avg_buf[i];
          }
          avg = sum / (double)NUM_AVGS;
          lcd.setCursor(0, 0);
          sprintf(printbuf, "%+.3f mTorr/s", roc);
          lcd.print(printbuf);
          roc_avg_buf_idx = 0;
        } else {
          roc_avg_buf_idx++;  
        }
        lcd.setCursor(0, 1);
        prev_reading = floatval;
        sprintf(printbuf, "%.2f mTorr", floatval);
        lcd.print(printbuf);
      } else {
        lcd.setCursor(0, 1);
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

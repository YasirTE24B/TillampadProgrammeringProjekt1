/*
* Name: Yasirs Brandsäkerhetssystem 1000
* Author: Yasir Abu Al Chay
* Date: 2026-01-28
* Description: Ett säkerhetssystem med brandlarm och RFID-lås. Om en eld skulle uppstå startas en vattenpump, larm och en servo styrd släckning.
* Systemet innehåller även en OLED skärm för status och sweep funktion för övervakning
*/

// Include Libraries
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8glib.h>

Servo sweepServo;   // servo 1 radar
Servo doorServo;    // servo 2 dörr
Servo fireServo;    // servo 3 eld

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

MFRC522 mfrc522(10, 9); // SDA=10 RST=9

// SERVO SAKER RÖR EJ!!!!
// Global variables
int pos = 90;
int sweep = 3;
int sistaValue = 0;
int sistaPos = 90;
bool servo_on = false;

// Blå tag UID
byte whiteTag[] = {0x73, 0xB8, 0x91, 0x18};

void setup() {
  pinMode(A0, INPUT); // flame sensor
  pinMode(7, OUTPUT); // led röd
  pinMode(6, OUTPUT); // led blå
  pinMode(8, OUTPUT); // dörr servo
  pinMode(4, OUTPUT); // pump relay
  pinMode(3, INPUT_PULLUP); // servo toggle knapp
  pinMode(2, OUTPUT); // piezo
  // Init communication
  Serial.begin(9600);
  
  SPI.begin();        // RFID
  mfrc522.PCD_Init(); // RFID

  doorServo.attach(A1); // dörr servo
  doorServo.write(0);

  sweepServo.attach(8); // sweep servo
  sweepServo.write(pos);
  
  fireServo.attach(5); // eld servo
  fireServo.write(90);
}


// Variabel som skickar en tom sträng till updateScreen så att skrämen inte visar prickar överallt
String currentScreenMsg = "";


/*
*This function runs repeatedly. It does RFID and fire detection
*Parameters: Void
*Returns: Void
*/
void loop() {
  // RFID kod ====================================================
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("JAG SER KORTET");
    
    if (mfrc522.uid.uidByte[0] == whiteTag[0] && 
        mfrc522.uid.uidByte[1] == whiteTag[1] && 
        mfrc522.uid.uidByte[2] == whiteTag[2] && 
        mfrc522.uid.uidByte[3] == whiteTag[3]) {
      
      Serial.println("ÖPPNAR DÖRR");
      updateScreen("Dorr Oppen");
      doorServo.write(60);
      delay(5000);
      doorServo.write(0);
    } else {
      Serial.println("FEL KORT!!!");
    }
    updateScreen("Skanna tagg");
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  // ==================================================

  int value = analogRead(A0);
  Serial.println(value);
  sistaValue = value;

  // servo toggle knapp
  if (digitalRead(3) == LOW) {
    servo_on = !servo_on;

    if (servo_on) {
      sweepServo.attach(8);
      sweepServo.write(pos);
      Serial.println("SERVO PA");
    } else {
      sweepServo.detach();
      Serial.println("SERVO INTEEEEE PA");
    }
    while (digitalRead(3) == LOW) {
      delay(10);
    }
  }

  if (value < 200) {
    fireServo.write(pos);
    Serial.print("ELD!!!! pos:");
    updateScreen("BRAND!!!");
    Serial.println(pos);

    doorServo.write(60);

    delay(1000);

    //tone(10, 200, 500);
    digitalWrite(7, HIGH);
    delay(100);
    digitalWrite(7, LOW);
    tone(2, random(3000,4000));
    digitalWrite(6, HIGH);
    delay(100);
    digitalWrite(6, LOW);
    delay(100);
    digitalWrite(4, HIGH); // pump på
    sistaPos = pos;
    // servo_on = false;
    delay(3000);
    digitalWrite(4, LOW); // pump av
    delay(2000);
    fireServo.write(90);
    doorServo.write(0);
  } else {
    noTone(2);
    sistaPos = 0;

    digitalWrite(4, LOW); // pump av

    if (servo_on) {
      pos = pos + sweep;

      if (pos >= 180) {
        sweep = -3; 
      } else if (pos <= 0) {
        sweep = 3;  
      }
    sweepServo.write(pos);
    // fireServo.write(pos);
    // doorServo.write(pos);
    Serial.println(pos);
    }

    updateScreen("Skanna tagg");
    
  }
  delay(25);

}

/*
*This function takes a string as input and displays the string on the OLED display
*Parameters: msg - const char* message to display
*Returns: void
*/
void updateScreen(const char* msg) {
  currentScreenMsg = String(msg); 
  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_helvB12);
    u8g.drawStr(20, 36, msg);
  } while (u8g.nextPage());
}


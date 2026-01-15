#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <U8glib.h>

// DU FÅR 3D PRINTA EN TYP BARREL TILL SKJUT SLANGEN SÅ ATT DEN PEKAR NERÅT OCH INTE SKJUTER SKIT ÖVERALLT. TACK I FÖRHAND.

Servo sweepServo;   // servo 1 radar
Servo doorServo;    // servo 2 dörr
Servo fireServo;    // servo 3 eld

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

MFRC522 mfrc522(10, 9); // SDA=10 RST=9

// SERVO SAKER RÖR EJ!!!!
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
  pinMode(3, INPUT_PULLUP); // servo toogel
  pinMode(2, OUTPUT); // piezo
  Serial.begin(9600);
  
  SPI.begin();        // Behövs för RFID
  mfrc522.PCD_Init(); // Behövs för RFID

  doorServo.attach(A1); // dörr tror jag
  doorServo.write(0);

  sweepServo.attach(8); // sweep tror jag
  sweepServo.write(pos);
  
  fireServo.attach(5); // eld
  fireServo.write(90);
}

void loop() {
  // RFID KOD START
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.println("JAG SER KORTET");
    
    if (mfrc522.uid.uidByte[0] == whiteTag[0] && 
        mfrc522.uid.uidByte[1] == whiteTag[1] && 
        mfrc522.uid.uidByte[2] == whiteTag[2] && 
        mfrc522.uid.uidByte[3] == whiteTag[3]) {
      
      Serial.println("ÖPPNAR DÖRR");
      u8g.firstPage();  
        do {
          u8g.setFont(u8g_font_helvB12);
          u8g.drawStr(20,36, "Dorr Oppen");
        } while( u8g.nextPage() );
      doorServo.write(60); // Öppna
      delay(5000);         // Vänta 5 sek
      doorServo.write(0);  // Stäng
    } else {
      Serial.println("FEL KORT!!!");
    }

    u8g.firstPage();  
      do {
        u8g.setFont(u8g_font_helvB12);
        u8g.drawStr(20,36, "Skanna tagg");
      } while( u8g.nextPage() );
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
  }
  // RFID KOD SLUT

  int value = analogRead(A0);
  Serial.println(value);
  sistaValue = value;

  // servo toggel
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

  if (value < 100) {
    fireServo.write(pos); // den sitter åt fel håll så måste köra motsatta värdet
    Serial.print("ELD!!!! pos:");
    Serial.println(pos);

    doorServo.write(60);

    delay(1000);

    //tone(10, 200, 500);
    digitalWrite(7, HIGH);
    delay(100);
    digitalWrite(7, LOW);
    tone(2, random(2000,4000));
    digitalWrite(6, HIGH);
    delay(100);
    digitalWrite(6, LOW);
    delay(100);
    digitalWrite(4, HIGH); // pump på
    sistaPos = pos;
    // servo_on = false;
    delay(3000); // 3 sek
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
    Serial.println(pos);
  }

  delay(25);
}
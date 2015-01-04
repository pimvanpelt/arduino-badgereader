
/*
 * MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               51                MOSI
 * SPI MISO   12               50                MISO
 * SPI SCK    13               52                SCK
 *
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on ebay.com.

 * Wiring for Alexis' reader:
 * 10-wire ribbon:
 * Controller           Color             Reader
 * 59711/R2             black             LED, Red Cathode
 * 59711/G2             white             LED, Green Cathode
 * 59711/B2             grey              LED, Blue Cathode
 * 9                    purple            RC522, RST
 * 11                   blue              RC522, MISO
 * 12                   green             RC522, MOSI
 * 13                   yellow            RC522, SCK
 * 10                   orange            RC522, SDA
 * 3.3V Vcc             red               RC522, 3.3V & LED Anode
 * GND                  brown             RC522, GND

 * 2                                      59711, DI
 * 3                                      59711, CI
 * 3.3V Vcc                               59711, V+ & VCC
 * GND                                    59711, GND

 * A4                                     LCD, SCL
 * A5                                     LCD, SDA
 * 5V Vcc                                 LCD, VCC
 * GBD                                    LCD, GND

 * 4                                      Pushbutton Green (to GND)
 * 5                                      Pushbutton Red (to GND)
 */
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_TLC59711.h>
#include <LiquidCrystal_I2C.h>


#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.

// LCD connects to VCC, GND, A4(SDA) and A5(SCL)
#define LCD_SDA 4 // A4
#define LCD_SCL 5 // A5
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, LCD_SDA, LCD_SCL , 6, 7, 3, POSITIVE);

#define TLC_DI  2
#define TLC_CI  3
Adafruit_TLC59711 tlc = Adafruit_TLC59711(1, TLC_CI, TLC_DI);
#define LED_INT_LEFT  3
#define LED_INT_RIGHT 2
#define LED_NOTUSED   1
#define LED_EXT       0

unsigned long timeout = -1;
#define TIMEOUT 5000

void resetLED() {
  tlc.setLED(LED_EXT, 4096, 0, 0); // b g r  // External Access LED
  tlc.setLED(LED_NOTUSED, 0, 0, 0); // NOTUSED
  tlc.setLED(LED_INT_RIGHT, 4096, 0, 0); // Internal Power LED
  tlc.setLED(LED_INT_LEFT, 0, 0, 0);    // Internal Access LED
}

void setup() {
  Serial.begin(9600);	// Initialize serial communications with the PC
  SPI.begin();			// Init SPI bus
  mfrc522.PCD_Init();	// Init MFRC522 card

  tlc.begin();
  tlc.write();
  resetLED();
  
  pinMode(4, INPUT);
  digitalWrite(4, HIGH);
  pinMode(5, INPUT);
  digitalWrite(5, HIGH);

  lcd.begin(16, 2);              // initialize the lcd
  lcd.clear();
  lcd.setBacklight(255);
  lcd.setCursor(0, 0);
  lcd.print("Alexis");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("Badge");
  delay(1000);
  lcd.setCursor(6, 1);
  lcd.print("Reader");
  delay(3000);
  lcd.setBacklight(0);
  lcd.clear();

  Serial.println("Scan PICC to see username");
}

unsigned int UIDtoInt(MFRC522::Uid *uid)
{
  unsigned int ret = uid->uidByte[0] + 256 * uid->uidByte[1];
  return ret;
}

String UIDtoName(MFRC522::Uid *uid)
{
  unsigned int uid_int = UIDtoInt(uid);
  switch (uid_int) {
    case 41555:    //1234567890123456
      return String("Toothless, draak");
    case 49619:
      return String("Marina, je mama,");
    case 35251:
      return String("Pim, je papa,   ");
    case 8851:
      return String("Inti, lieve Inti");
    case 44899:
      return String("Opa, jawel Opa, ");
    case 63507:
      return String("Kaloo, knuffel, ");
    case 5907:
      return String("Alexis, jij dus,");
    default:
      return String(uid_int);
  }
}



void loop() {
  static String _name;
  String readname;
  int redButton, greenButton;

  // Look for new cards
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    timeout = millis() + TIMEOUT;
    readname = UIDtoName(&mfrc522.uid);
    if (readname.equals(_name)) {
      Serial.print("Re-read the same name: ");
      Serial.println(_name);
      return;
    }
    _name = readname;
    lcd.clear();
    lcd.setBacklight(255);
    lcd.setCursor(0, 0);
    lcd.print(_name);
    lcd.setCursor(0, 1);
    lcd.print("is aan de deur. ");

    Serial.print("Read UID '");
    Serial.print(_name);
    Serial.print("', setting timeout to ");
    Serial.println(timeout);
    tone(8, 1660);    
    tlc.setLED(LED_EXT, 8192, 0, 0);
  }

  if (timeout != -1) {
    if (millis() > timeout) {
      Serial.println("Timeout, turning off LED");
      resetLED();
      noTone(8);

      timeout = -1;
      lcd.clear();
      lcd.setBacklight(0);
      _name = "";
    }
    else {
      // Serial.println("Do things ...");
      redButton = digitalRead(5);
      greenButton = digitalRead(4);
      if (greenButton == 0) {
        tlc.setLED(LED_INT_LEFT, 0, 32768, 0);
        tlc.setLED(LED_EXT, 0, 32768, 0);
        noTone(8);
        lcd.setCursor(0, 1);
        lcd.write("mag naar binnen!");
        timeout = millis() + TIMEOUT;
      }
      if (redButton == 0) {
        tlc.setLED(LED_INT_LEFT, 0, 0, 32768);
        tlc.setLED(LED_EXT, 0, 0, 32768);
        noTone(8);
        lcd.setCursor(0, 1);
        lcd.write("mag er niet in! ");
        timeout = millis() + TIMEOUT;
      }
    }
  }
  tlc.write();
}


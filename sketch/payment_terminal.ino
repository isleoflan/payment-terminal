#include <SPI.h>
#include <MFRC522.h>
#include <WebUSB.h>
#include <LiquidCrystal.h>

// definitions for RFID
#define SS_PIN 10
#define RST_PIN 9

// definitions for piezzo
#define PIEZZO_PIN 6

// USB Serial Definition
#define USBSerial WebUSBSerial

// LCD definitions
#define RS A1
#define EN A0
// data pin definitions
#define d4 5
#define d5 4
#define d6 3
#define d7 2

WebUSB WebUSBSerial(1 /* https:// */, "clerk.isleoflan.ch");
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 

LiquidCrystal lcd(RS, EN, d4, d5, d6, d7);

char line1[16];
char line2[16];

int lineIndex = 0;

bool newReadStarted = false;


void setup() { 
  // set up lcd
  lcd.begin(16, 2);

  while (!WebUSBSerial) {
    ;
  }
  USBSerial.begin(9600);
  
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
}
 
void loop() {
  checkForIncomingMessage();

  readCard();
}

void checkForIncomingMessage() {
  while(USBSerial && USBSerial.available()){
    if(!newReadStarted){
      newReadStarted = true;
      tone(PIEZZO_PIN, 523, 250);
      lcd.clear();
      lcd.setCursor(0,0);
      lineIndex = 0;
    }
    int byte = USBSerial.read();

    if(lineIndex == 16){
      // set cursor to new Line
      lcd.setCursor(0,1);
    }

    lcd.write(byte);
    lineIndex++;

    USBSerial.flush();
  }

  newReadStarted = false;
}

void readCard() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
  
    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;
  
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  
    // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      return;
    }

    // card is present and ready to be readed
    tone(PIEZZO_PIN, 523, 250);

    // send the readed serial number to the application
    printHex(rfid.uid.uidByte, rfid.uid.size);

    // Halt PICC
    rfid.PICC_HaltA();
  
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    USBSerial.flush();
}

/**
 * Helper routine to dump a byte array as hex values to USBSerial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    USBSerial.print(buffer[i] < 0x10 ? " 0" : " ");
    USBSerial.print(buffer[i], HEX);
  }
}
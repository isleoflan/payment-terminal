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

bool startRead = false;

// Init array that will store new NUID 
byte nuidPICC[4];

void setup() { 
  // set up lcd
  lcd.begin(16, 2);

  while (!WebUSBSerial) {
    ;
  }
  USBSerial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  USBSerial.flush();
}
 
void loop() {
  if (USBSerial && USBSerial.available()) {
    int byte = USBSerial.read();
    tone(PIEZZO_PIN, 523 , 250);
    if (byte == 'H') {
      // USBSerial.print(F("Place Your Card"));
      lcd.setCursor(0,0);
      lcd.print("Place your Card");
      startRead = true;
    }
    USBSerial.flush();
  }

  // if(startRead){
    readCard();
  // }
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
      USBSerial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }
  
    if (rfid.uid.uidByte[0] != nuidPICC[0] || 
      rfid.uid.uidByte[1] != nuidPICC[1] || 
      rfid.uid.uidByte[2] != nuidPICC[2] || 
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
  
      tone(PIEZZO_PIN, 523 , 250);
  
      // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
      
      lcd.clear();
      lcd.setCursor(0,0);
      for(byte i = 0; i < rfid.uid.size; i++){
        lcd.setCursor(i,0);
        lcd.print(rfid.uid.uidByte[i], HEX);
      }
      // printHex(rfid.uid.uidByte, rfid.uid.size);
      USBSerial.println(F("READ"));
    } else{
      USBSerial.println(F("Card read previously."));
      
      tone(PIEZZO_PIN, 261 , 500);
    }
  
    // Halt PICC
    rfid.PICC_HaltA();
  
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    // be ready for next Card
    startRead = false;

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

/**
 * Helper routine to dump a byte array as dec values to USBSerial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    USBSerial.print(buffer[i] < 0x10 ? " 0" : " ");
    USBSerial.print(buffer[i], DEC);
  }
}
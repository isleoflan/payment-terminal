#include <SPI.h>
#include <MFRC522.h>
#include <WebUSB.h>

#define SS_PIN 10
#define RST_PIN 9

#define PIEZZO_PIN 6
#define USBSerial WebUSBSerial

WebUSB WebUSBSerial(1 /* https:// */, "clerk.isleoflan.ch");
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

bool startRead = false;

// Init array that will store new NUID 
byte nuidPICC[4];

void setup() { 
  while (!Serial) {
    ;
  }
  USBSerial.begin(9600);
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  USBSerial.println(F("This code scan the MIFARE Classsic NUID."));
  USBSerial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  USBSerial.flush();
}
 
void loop() {
  if (Serial && USBSerial.available()) {
    int byte = USBSerial.read();
    USBSerial.write(byte);
    Serial.write(byte);
    tone(PIEZZO_PIN, 523 , 250);
    if (byte == 'H') {
      USBSerial.write("Place your Card");
      startRead = true;
    }
    USBSerial.write("\r\n> ");
    USBSerial.flush();
  }

  if(startRead){
    readCard();
  }
}

void readCard() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! rfid.PICC_IsNewCardPresent())
      return;
  
    // Verify if the NUID has been readed
    if ( ! rfid.PICC_ReadCardSerial())
      return;
  
    USBSerial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    USBSerial.println(rfid.PICC_GetTypeName(piccType));
  
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
      USBSerial.println(F("A new card has been detected."));
  
      tone(PIEZZO_PIN, 523 , 250);
  
      // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
     
      USBSerial.println(F("The NUID tag is:"));
      USBSerial.print(F("In hex: "));
      printHex(rfid.uid.uidByte, rfid.uid.size);
      USBSerial.println();
      USBSerial.print(F("In dec: "));
      printDec(rfid.uid.uidByte, rfid.uid.size);
      USBSerial.println();
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
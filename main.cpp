#include <Arduino.h> // Libraries
#include <SPI.h>
#include <MFRC522.h>

// Global variables
#define RFID_SS 5
#define RFID_RST 22
#define GREEN_LED 25 
#define RED_LED 26 
#define BUZZER 27

MFRC522 rfid(RFID_SS, RFID_RST); 
String authorizedUIDs[] = {"9C 2B 4D 05"}; 
int numAuthorized = 1; 
unsigned long indicatorStart = 0; 
unsigned long indicatorTime = 0; 
bool indicatorOn = false; 
String lastUID = ""; 
unsigned long lastScanTime = 0; 

// getUID() turns the card's UID bytes into text, for example "4A 1F 9C 02"
String getUID()
{
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uid = uid + "0";
    }
    uid = uid + String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) {
      uid = uid + " ";
    }
  }
  uid.toUpperCase();
  return uid;
}

bool isAuthorized(String uid) {  // Functions
  for (int i = 0; i < numAuthorized; i++) {  
    if (uid == authorizedUIDs[i]) {  
      return true; 
    } 
  } 
  return false; 
}

void startIndicatorTimer(unsigned long duration)  //starts the timer since the program has started
{ 
  indicatorStart = millis(); 
  indicatorTime = duration; 
  indicatorOn = true; 
} 

void allIndicatorsOff() 
{ 
  digitalWrite(GREEN_LED, LOW); 
  digitalWrite(RED_LED, LOW); 
  digitalWrite(BUZZER, LOW); 
} 

void onAccessGranted() 
{ 
  Serial.println("ACCESS GRANTED"); 
  allIndicatorsOff(); 
  digitalWrite(GREEN_LED, HIGH); 
  startIndicatorTimer(2000);
} 

void onAccessDenied() 
{ 
  Serial.println("ACCESS DENIED"); 
  allIndicatorsOff(); 
  digitalWrite(RED_LED, HIGH); 
  digitalWrite(BUZZER, HIGH); 
  startIndicatorTimer(1000); 
} 

void onTimerExpired() 
{ 
  allIndicatorsOff(); 
  indicatorOn = false; 
  Serial.println("Ready"); 
} 

void onCardScanned(String uid) 
{ 
  if (uid == lastUID && millis() - lastScanTime < 2000) 
  { 
    Serial.println("Repeat ignored"); 
    return; 
  } 
  lastUID = uid; 
  lastScanTime = millis(); 
  Serial.print("Card UID: "); 
  Serial.println(uid); 
  if (isAuthorized(uid) == true) 
  { 
    onAccessGranted(); 
  } 
  else 
  { 
    onAccessDenied(); 
  } 
} 

void setup()
{
  Serial.begin(115200);
  Serial.println("Program Started");

  SPI.begin(); //RFID code
  rfid.PCD_Init();
  delay(10);
  rfid.PCD_DumpVersionToSerial();
  Serial.println("Tap a card...");

  pinMode(GREEN_LED, OUTPUT); // LED and buzzer code
  pinMode(RED_LED, OUTPUT); 
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  if (rfid.PICC_IsNewCardPresent() == true && rfid.PICC_ReadCardSerial() == true) {  
    String uid = getUID(); 
    onCardScanned(uid); 
    rfid.PICC_HaltA(); //Puts the card to sleep
    rfid.PCD_StopCrypto1(); //ends the authentication
  } 

  if (indicatorOn == true && millis() - indicatorStart >= indicatorTime) { // Timer 
    onTimerExpired(); 
  } 
}
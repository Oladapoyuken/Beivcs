#include <EEPROM.h>

#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#define relayPin 8

#define bypassActivate 5 //input
#define bypassDeactivate 6 //input
#define enrollPin 10 //input
#define alertPin 12 //output
#define enroll_delPin 11

#define buz 13


#define user1 A1
#define user2 A2
#define user3 A3
#define user4 A4
#define user5 A5

boolean state = true;

SoftwareSerial mySerial(2,3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id; 
int alertCounter = 0;

void setup() {
  Serial.begin(9600);
  finger.begin(57600);

  pinMode(relayPin, OUTPUT);
  pinMode(buz, OUTPUT);
  pinMode(alertPin, OUTPUT);
  
  pinMode(enroll_delPin, OUTPUT); //This signals gsm controller that it has finished enrolling or deleting
  
  pinMode(bypassActivate, INPUT);
  pinMode(bypassDeactivate, INPUT);
  pinMode(enrollPin, INPUT);
  pinMode(user1, INPUT);
  pinMode(user2, INPUT);
  pinMode(user3, INPUT);
  pinMode(user4, INPUT);
  pinMode(user5, INPUT);
  
  digitalWrite(buz, LOW);
  digitalWrite(enroll_delPin, LOW);
  digitalWrite(enrollPin, LOW);
  digitalWrite(alertPin, LOW);
  
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    
    for(int i = 0; i < 3; i++){
      digitalWrite(buz, HIGH);
      delay(1000);
      digitalWrite(buz, LOW);
      delay(1000);
    }
    
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    digitalWrite(buz, HIGH);
    while (1) { 
      delay(1);
    }
  }
}


uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

//delete any user
void checkDeleteOptions(){
   if(analogRead(user1) > 900){
    deleteFingerPrint(1);
  }
  else if(analogRead(user2) > 900){
    deleteFingerPrint(2);
  }
  else if(analogRead(user3) > 900){
    deleteFingerPrint(3);
  }
  else if(analogRead(user4) > 900){
    deleteFingerPrint(4);
  }
  else if(analogRead(user5) > 900){
    deleteFingerPrint(5);
  }
}

uint8_t deleteFingerPrint(int user){
  uint8_t id = user;
  uint8_t p = -1;
  
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    EEPROM.update(user, 0);
    buzzer(1,1000);
    delay(1000);
    
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  }
  else {
    Serial.print("Unknown error: 0x"); 
    Serial.println(p, HEX);
    return p;
  }   
}

void loop() { 
  getFingerPrint();
  delay(100);
  enrollment();
  delay(100);

  //bypass fingerPrint check
  if(digitalRead(bypassActivate)== HIGH){
    digitalWrite(relayPin, LOW);
    buzzer(1, 100);
    state = true;
  }
  if(digitalRead(bypassDeactivate)== HIGH){
    digitalWrite(relayPin, HIGH);
    buzzer(1, 100);
    state = false;
  }
}

int getFingerPrint(){
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK){
    if(alertCounter == 5){
      digitalWrite(buz, HIGH);
      delay(5000);
      digitalWrite(buz, LOW);
      alertCounter = 0;
    }
    alertCounter++;
    return -1;
  }
  
  
//   found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);

  if(finger.confidence > 50 && state == true){
    digitalWrite(relayPin, HIGH);
    
    buzzer(1, 100);
    state = false;
    alertCounter = 0;

    digitalWrite(alertPin, HIGH);
    delay(10000);
    digitalWrite(alertPin, LOW);
  }
  else if(finger.confidence > 50 && state == false){
    digitalWrite(relayPin, LOW);
    buzzer(1, 100);
    state = true;
    alertCounter = 0;

    digitalWrite(alertPin, HIGH);
    delay(10000);
    digitalWrite(alertPin, LOW);
    
  }
  return finger.fingerID; 
}

boolean checkMemory(int id){
    boolean check = false;
    int val = EEPROM.read(id); 
    if(val == 0){
      check = false;
    }
    else if(val == id){
      check = true;
  }
  return check;
}


void enrollment(){
  id = 0;
  if(digitalRead(enrollPin) == HIGH){
    
      if(analogRead(user1) > 900){
        id = 1;
      }
      else if(analogRead(user2) > 900){
        id = 2;
      }
      else if(analogRead(user3) > 900){
        id = 3;
      }
      else if(analogRead(user4) > 900){
        id = 4;
      }
      else if(analogRead(user5) > 900){
        id = 5;
      }
      else{
        digitalWrite(enroll_delPin, HIGH);
        delay(2000);
        digitalWrite(enroll_delPin, LOW);
        delay(5000);
        return;
      }

    //To enroll checkMemory must return false, meaning Id no registered initially, else delete
    if(id != 0){
      if(checkMemory(id) == false && id != 0){
        for(int i = 0; i < 3; i++){
          buzzer(5, 100);
          delay(500);
         }
        while (!getFingerprintEnroll() && id != 0 );
        id = 0;
        digitalWrite(enroll_delPin, HIGH);
        delay(2000);
        digitalWrite(enroll_delPin, LOW);
        delay(5000);
      }
      else{
        checkDeleteOptions();
        id = 0;
        digitalWrite(enroll_delPin, HIGH);
        delay(2000);
        digitalWrite(enroll_delPin, LOW);
        delay(5000);
      }
    
    }
    else{
      return;
    }
  }
}


uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      { Serial.println("Image taken");
        buzzer(1, 100);
        break;
      }
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      { Serial.println("Communication error"); 
        buzzer(1, 2000);
        break;
      }
    case FINGERPRINT_IMAGEFAIL:
      { Serial.println("Imaging error");
        buzzer(1, 2000);
        break;
      }
    default:
      { Serial.println("Unknown error");
        buzzer(1, 2000);
        break;
      }
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      { Serial.println("Image too messy");
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_PACKETRECIEVEERR:
      { Serial.println("Communication error");
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_FEATUREFAIL:
      { Serial.println("Could not find fingerprint features");
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_INVALIDIMAGE:
      { Serial.println("Could not find fingerprint features"); 
        buzzer(1, 2000);
        return p;
      }
    default:
      { Serial.println("Unknown error");  
        buzzer(1, 2000);
        return p;
      }
  }
  Serial.println("Remove finger");
  delay(2000);
  buzzer(2, 300);
  
  
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken"); buzzer(1, 100);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error"); buzzer(1, 2000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error"); buzzer(1, 2000);
      break;
    default:
      Serial.println("Unknown error"); buzzer(1, 2000);
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      { Serial.println("Image too messy"); 
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_PACKETRECIEVEERR:
      { Serial.println("Communication error");
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_FEATUREFAIL:
      { Serial.println("Could not find fingerprint features"); 
        buzzer(1, 2000);
        return p;
      }
    case FINGERPRINT_INVALIDIMAGE:
      { Serial.println("Could not find fingerprint features");
        buzzer(1, 2000);
        return p;
      }
    default:
      { Serial.println("Unknown error"); 
        buzzer(1, 2000);
        return p;
      }
  }
  delay(1000);
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error"); 
    buzzer(1, 2000);
    return p;
  } 
  else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    buzzer(1, 2000);
    return p;
  } 
  else {
    Serial.println("Unknown error");
    buzzer(1, 2000);
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    EEPROM.update(id, id); //save ID on EEPROM
    for(int i = 0; i < 3; i++){
      buzzer(5, 100);
      id = 0;
      delay(500);
    }
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    buzzer(1, 2000);
    return p;
  } 
  else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location"); 
    buzzer(1, 2000);
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash"); 
    buzzer(1, 2000);
    return p;
  }
  else {
    Serial.println("Unknown error"); 
    buzzer(1, 2000);
    return p;
  }   
}


//buzzer
void buzzer(int beepNumber, int beepLength){
  for(int i = 0; i < beepNumber; i++){
    digitalWrite(buz, HIGH);
    delay(beepLength);
    digitalWrite(buz, LOW);
    delay(beepLength);
  }
  
}


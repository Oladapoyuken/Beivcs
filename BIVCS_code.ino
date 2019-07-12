#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#define activate 5 //output
#define deactivate 12 //output
#define enrollPin 10 //output
#define alertPin 6 //input
#define alarmPin 9 
#define enroll_delPin 11

//users

#define user1 A1
#define user2 A2
#define user3 A3
#define user4 A4
#define user5 A5


SoftwareSerial SIM800L(7, 8);
TinyGPSPlus gps;
double latitude, longitude;
char buffer[80];


//RESPONSE NUMBER   
String phoneNum = "2349012556494";
String response;
int lastStringLength = response.length();
int i = 0;

String coordinate;

void setup(){

  pinMode(user1, OUTPUT);
  pinMode(user2, OUTPUT);
  pinMode(user3, OUTPUT);
  pinMode(user4, OUTPUT);
  pinMode(user5, OUTPUT);

  pinMode(activate, OUTPUT);
  pinMode(deactivate, OUTPUT);
  pinMode(alarmPin, OUTPUT);
  pinMode(alertPin, INPUT);
  pinMode(enrollPin, OUTPUT);
  pinMode(enroll_delPin, INPUT);
  
  Serial.begin(9600);
  SIM800L.begin(9600);

  Serial.println("PAAADC GPS");
  
  SIM800L.println("AT+CMGF=1");
  Serial.println("SIM800L started at 9600");
  delay(1000);
  Serial.println("Setup Complete! SIM800L is Ready!");
  SIM800L.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  
  digitalWrite(enrollPin, LOW);
  digitalWrite(alertPin, LOW);
 
}

void loop() {
  
  showLocation();
  //Used to clear the  SIM Memory whenever it Receives At most 6 messages
  if ( i >= 6){           
    SIM800L.println("AT+CMGDA=\"DEL ALL\"");
    Serial.println("SMS deleted");
    i = 0;
  }
  
   if (SIM800L.available()>0){

     response = SIM800L.readString();
     Serial.println(response);
     if(response.length() > 40){
        response.toCharArray(buffer, 80);
        String msg;
        String number;
        
        //Extracting phone number and message content from response
        for(int i = sizeof(buffer); i > 0; i--){
            if(buffer[i]=='$'){
              int n = i - 6;
              for(int c = n; c < i; c++){
                msg+=buffer[c];
              }
              break;
            }
         }
         
         for(int i = 3; i < 10; i++){    
            if(buffer[i]=='+'){
               int n = i + 14;
               for(int c = i; c < n; c++){
                  number+=buffer[c];  
                }
                break;
            }     
         }
         Serial.print("Messaage From: ");
         Serial.println(number);
         Serial.print("Content: ");
         Serial.println(msg);
     
     if(msg.equals("locate")){
        SIM800L.println("AT+CMGF=1"); 
        delay(1000);
        SIM800L.print("AT+CMGS=\"+");
        SIM800L.print(phoneNum);
        SIM800L.println("\"\r");
        delay(100);
        SIM800L.println(coordinate);
        msg = "";
        delay(100);
        SIM800L.println((char)26);
        delay(1000);
     }

      ///// To Activate Security
      else if(msg.equals("activa")){
        
        digitalWrite(activate, HIGH);
        delay(5000);
        digitalWrite(activate, LOW);
        
        SIM800L.println("AT+CMGF=1"); 
        delay(1000);
        SIM800L.print("AT+CMGS=\"+");
        SIM800L.print(phoneNum);
        SIM800L.println("\"\r");
        delay(100);
        SIM800L.println("one");
        msg = "";
        delay(100);
        SIM800L.println((char)26);
        delay(1000);
        i++; 
      }


      //// To Deactivate Security
      else if(msg.equals("deacti")){
        
        digitalWrite(deactivate, HIGH);
        delay(5000);
        digitalWrite(deactivate, LOW);
        
        SIM800L.println("AT+CMGF=1"); 
        delay(1000);
        SIM800L.print("AT+CMGS=\"+");
        SIM800L.print(phoneNum);
        SIM800L.println("\"\r");
        delay(100);
        SIM800L.println("two");
        msg = "";
        delay(100);
        SIM800L.println((char)26);
        delay(1000);
        i++;
      }

      //// For The Locator Alarm
     else if(msg.equals("alarmm")){
      
        for (int x = 0;  x < 10; x++){
        
          digitalWrite(alarmPin, HIGH);
          delay(3000);
          digitalWrite (alarmPin, LOW);
          delay(3000);
          if(digitalRead(alertPin) == HIGH){
            break;
           }
        }
     }

     //Registering users
     else if(msg.equals("useone")){
         Serial.println("CONFIGURING USER ONE");
         digitalWrite(enrollPin, HIGH);
         analogWrite(user1, 1000); 
         delay(2000);
        
         while(digitalRead(enroll_delPin) == LOW); //Stay here until enrollment is completed
         
         digitalWrite (enrollPin, LOW);
         analogWrite(user1, 0);
          
     }
     else if(msg.equals("usetwo")){
         Serial.println("CONFIGURING USER TWO");
         digitalWrite(enrollPin, HIGH);
         analogWrite(user2, 1000); 
         delay(2000);
        
         while(digitalRead(enroll_delPin) == LOW); //Stay here until enrollment is completed
         
         digitalWrite (enrollPin, LOW);
         analogWrite(user2, 0);
     }
     else if(msg.equals("uthree")){
         Serial.println("CONFIGURING USER TWO");
         digitalWrite(enrollPin, HIGH);
         analogWrite(user3, 1000); 
         delay(2000);
        
         while(digitalRead(enroll_delPin) == LOW); //Stay here until enrollment is completed
         
         digitalWrite (enrollPin, LOW);
         analogWrite(user3, 0);
     }
     else if(msg.equals("usfour")){
         digitalWrite(enrollPin, HIGH);
         analogWrite(user5, 1000); 
        
         while(digitalRead(enroll_delPin) == LOW); //Stay here until enrollment is completed
         
         digitalWrite (enrollPin, LOW);
         analogWrite(user4, 0);
     }
     else if(msg.equals("usfive")){
         digitalWrite(enrollPin, HIGH);
         analogWrite(user4, 1000); 
        
         while(digitalRead(enroll_delPin) == LOW); //Stay here until enrollment is completed
         
         digitalWrite (enrollPin, LOW);
         analogWrite(user5, 0);
     }
    } 
    i++;
  }
}

void alert(){
  SIM800L.println("AT+CMGF=1"); 
  delay(1000);
  SIM800L.print("AT+CMGS=\"+");
  SIM800L.print(phoneNum);
  SIM800L.println("\"\r");
  delay(100);
  SIM800L.println("THEFT ALERT, CALL SECURITY!");
  delay(100);
  SIM800L.println((char)26);
  delay(1000);
}

void showLocation(){
  if(Serial.available()){
    gps.encode(Serial.read());
  }
  if(gps.location.isUpdated()){ 
    latitude = gps.location.lat();
    longitude = gps.location.lng();
    coordinate = String(latitude, 6) + " " + String(longitude, 6); 
    Serial.println(coordinate);
  }
}

//====================== INTIALIZE BYLNK ========================
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "7SW2R9CtkU469aPDUKQ-KWh1T9TSh_ct";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "KAIZOKU";
char pass[] = "nakpassword";

WidgetLCD lcd0(V0);

int Bridge_Alarm1_Triggered = 0,
    Bridge_Alarm2_Triggered = 0,
    Bridge_Alarm3_Triggered = 0,
    GATE_CLOSE_TIME_DELAY = 10,// In Minute
    BRIDGE_CLOSE_TIME = 10,//In Minute
    DELAY_AFTER_CAR_CNT = 10;//10 min

BLYNK_CONNECTED(){
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V6);
}

BLYNK_WRITE(V1){
  GATE_CLOSE_TIME_DELAY = param.asInt();
}
BLYNK_WRITE(V2){
  BRIDGE_CLOSE_TIME = param.asInt();
}

BLYNK_WRITE(V3){
  Bridge_Alarm1_Triggered = param.asInt();
}
BLYNK_WRITE(V4){
  Bridge_Alarm2_Triggered = param.asInt();
}
BLYNK_WRITE(V5){
  Bridge_Alarm3_Triggered = param.asInt();
}
BLYNK_WRITE(V6){
  DELAY_AFTER_CAR_CNT = param.asInt();
}

int Notification_1 = 0,
    Notification_2 = 0;
int Alarm_Flag = 0;
//===================== OLED Setup ================
#include <Adafruit_GFX.h>        //OLED libraries
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#include <Wire.h>
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)
//================= Initialize Ultrasonic =================
#define trigPin 25
#define echoPin 33

float duration;

float distance;
const int distance_limit = 5;

int State = 0;
//================= Initialize SERVO PINS =================
#include <ESP32_Servo.h> 

Servo Gate1,
      Gate2,
      Bridge1,
      Bridge2,
      Nate;  

#define Gate1_Pin 26
#define Gate2_Pin 16
#define Bridge1_Pin 12
#define Bridge2_Pin 17
#define Nate_Pin 32

int Nate_Drop = 180;
int Nate_Pickup = 0;

int Bridge_Close_STEP = 0,
    Bridge_Open_STEP = 90;

int Bridge1_Open = 90,
    Bridge1_Close = 45;

int Bridge2_Open = 90,
    Bridge2_Close = 45;

int Gate_Close_STEP = 180,
    Gate_Open_STEP = 90;

int Gate1_Open = 90,
    Gate1_Close = 180;

int Gate2_Open = 90,
    Gate2_Close = 180;

int Servo_Move_Delay = 50;
//================= Initialize OUT PINS =================
#define LED_RED_G1 18
#define LED_RED_G2 23
#define LED_GREEN_G1 5
#define LED_GREEN_G2 19
#define BUZZER 2
int CNT_STEP = 0;
int LED_BLINKING = 0,
    ENABLE_LED_BLINKING = 0;
//================= DELAY WITHOUT DELAY VARIABLES =================
unsigned long previousMillis = 0;      
const long interval = 1000;
//================= Initialize IR SENSOR PINS =================
#define IR_G1_OUT 13
#define IR_G1_IN 27
#define IR_G2_OUT 14
#define IR_G2_IN 4

int IR_ROAD1_EXIT = 0,
    IR_ROAD1_IN= 0,
    IR_ROAD2_EXIT = 0,
    IR_ROAD2_IN = 0;

int Last_IR_ROAD1_EXIT = 0,
    Last_IR_ROAD1_IN= 0,
    Last_IR_ROAD2_EXIT = 0,
    Last_IR_ROAD2_IN = 0;

int CNT_CAR_G1 = 0,
    CNT_CAR_G2 = 0;
//====================== DEFINE STEP =============
enum _BRIDGE_SYSTEM
{
    BRIDGE_OPEN,
    BRIDGE_CLOSE
};
volatile _BRIDGE_SYSTEM BRIDGE_STATE = BRIDGE_OPEN;

enum _BRIDGE_CLOSE_SYSTEM
{
    IDLE_1,
    TRIGGER_BRIDGE_CLOSE,
    MAKE_SURE_NO_CAR,
    ALLOW_SHIP_PASSBY,
};
volatile _BRIDGE_CLOSE_SYSTEM BRIDGE_CLOSE_STATE = IDLE_1;

enum _BRIDGE_OPEN_SYSTEM
{
    IDLE_2,
    DROP_NATE,
    OPEN_BRIDGE,
    OPEN_GATE
};
volatile _BRIDGE_OPEN_SYSTEM BRIDGE_OPEN_STATE = IDLE_2;

int ENABLE_BRIDGE_CLOSE = 0;
int BRIDGE_CLOSE_NOW = 0;
int ENABLE_BRIDGE_OPEN = 0;

int Debouncer_1 = 0;
int Ship_waitime = 0;
//================= VOID SETUPS =================   
void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();

  // DEFINE SERVO PINS
  Gate1.attach(Gate1_Pin, 500, 2400);
  Gate2.attach(Gate2_Pin, 500, 2400);
  Bridge1.attach(Bridge1_Pin, 500, 2400);
  Bridge2.attach(Bridge2_Pin, 500, 2400);
  Nate.attach(Nate_Pin, 500, 2400);
  // DEFINE IR SENSOR AS INPUT
  pinMode(IR_G1_OUT,INPUT);
  pinMode(IR_G1_IN,INPUT);
  pinMode(IR_G2_OUT,INPUT);
  pinMode(IR_G2_IN,INPUT);
  // DEFINE ULTRASONIC PINS
  pinMode(trigPin,OUTPUT);     //sets trigpin as output to send ultrasound
  pinMode(echoPin,INPUT);      //sets echopin as input to receive ultrasound
  // DEFINE LED AND BUZZER AS OUTPUT
  pinMode(LED_RED_G1,OUTPUT);
  pinMode(LED_RED_G2,OUTPUT);
  pinMode(LED_GREEN_G1,OUTPUT);
  pinMode(LED_GREEN_G2,OUTPUT);
  pinMode(BUZZER,OUTPUT);

  digitalWrite(LED_RED_G1,LOW);
  digitalWrite(LED_RED_G2,LOW);
  digitalWrite(LED_GREEN_G1,HIGH);
  digitalWrite(LED_GREEN_G2,HIGH);

  digitalWrite(BUZZER,HIGH);
  delay(150);
  digitalWrite(BUZZER,LOW);
  
  Gate1.write(Gate1_Open);
  Gate2.write(Gate2_Open);
  Bridge1.write(Bridge1_Open);
  Bridge2.write(Bridge2_Open);
  Nate.write(Nate_Drop);

  display.clearDisplay();

  display.setTextSize(2);                                   
  display.setTextColor(WHITE); 
  display.setCursor(20,1); 
  display.print("WIFI");
  display.setTextSize(1); 
  display.setCursor(0,20);  
  display.print("Connecting to");  
  display.setCursor(0,30);    
  display.print(ssid);
  display.display();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  long int StartTime=millis();
    while (WiFi.status() != WL_CONNECTED) 
    {
      int i;
      i++;
      Serial.println(i); 
      if ((StartTime+10000) < millis()) break;
    } 
  if (WiFi.status() != WL_CONNECTED) 
  {    
      Serial.println("Reset");   
      display.clearDisplay();

      display.setCursor(0,0); 
      display.print("Reseting.."); 

      display.display();

      digitalWrite(BUZZER,HIGH);
      delay(500);
      digitalWrite(BUZZER,LOW);
      delay(500);
      ESP.restart();
    }else{
      Blynk.begin(auth, ssid, pass);
  
      Serial.println("WiFi connected");

      display.clearDisplay();

      display.setCursor(0,0); 
      display.print("Wifi Connected"); 
      display.setCursor(0,10);    
      display.print("with IP:"); 
      display.setCursor(0,20);  
      display.print(WiFi.localIP());

      display.display();

      delay(1000);
      display.clearDisplay();
      lcd0.clear();

      
    }
}
//================= VOID LOOPS =================   
void loop() {
  Blynk.run();
  IR_ROAD1_EXIT = digitalRead(IR_G1_OUT);
  IR_ROAD1_IN = digitalRead(IR_G1_IN);
  IR_ROAD2_EXIT = digitalRead(IR_G2_OUT);
  IR_ROAD2_IN = digitalRead(IR_G2_IN);
  
    unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    CNT_STEP++;
    Ship_waitime++;
/*
    Serial.println();
    Serial.print("IR_ROAD1_EXIT:");
    Serial.print(IR_ROAD1_EXIT);
    Serial.print("  ");

    Serial.print("IR_ROAD1_IN:");
    Serial.print(IR_ROAD1_IN);
    Serial.print("  ");

    Serial.print("IR_ROAD2_EXIT:");
    Serial.print(IR_ROAD2_EXIT);
    Serial.print("  ");

    Serial.print("IR_ROAD2_IN:");
    Serial.print(IR_ROAD2_IN);
    Serial.print("  ");
*/
    display.clearDisplay();
    display.setCursor(0,0); 
    display.print("Count Car"); 
    display.setCursor(0,10);    
    display.print("Road G1:"); 
    display.print(CNT_CAR_G1); 
    display.setCursor(0,20);  
    display.print("Road G2:"); 
    display.print(CNT_CAR_G2); 
    display.setCursor(0,30);  
    display.print("Distance:"); 
    display.print(distance); 
    display.print("cm");
    

    lcd0.print(0,0,"Road G1:"+String(CNT_CAR_G1));
    lcd0.print(0,1,"Road G2:"+String(CNT_CAR_G2));

    Ultrasonic_Data(); /*
    Serial.print("Distance:");
    Serial.print(distance);
    Serial.println("cm");*/

    LED_BLINKING_FUNCTION();
  }

  //===COUNT CAR====
  COUNT_CAR();
  Bridge_System_Overall();

  display.display();
}
 /*================ Read Obstacle =================*/
void Ultrasonic_Data(){
  digitalWrite(trigPin, LOW);   //sets trigpin to LOW so that it do not emits ultrasound

                                                          //  initially
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);                  //sets trigpin to HIGH state to emit ultrasound

  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);                        //reads the travel time of the pulse

  distance = microsecondsToCentimeters(duration);     //function
}
/*================ Ultrasonic Equation Function =================*/
long microsecondsToCentimeters(long microseconds)
{

 return microseconds / 29/ 2;  /*Speed of sound is 340m/s. So it takes 29 microseconds to  travel 1 centimeter length of the path.All the working  is same as explained in calculation of inches*/

}
 /*================ Bridge_System_Overall =================*/
void Bridge_System_Overall(){
  switch (BRIDGE_STATE)
  {
  case BRIDGE_OPEN:

    if(Bridge_Alarm1_Triggered == HIGH ){
      if(ENABLE_BRIDGE_CLOSE == LOW){  
        Blynk.notify("Alarm 1 Triggered Now");  
        Blynk.virtualWrite(V3,0);   
        Blynk.virtualWrite(V4,0);  
        Blynk.virtualWrite(V5,0);  
        Bridge_Alarm1_Triggered = LOW;
        Bridge_Alarm2_Triggered = LOW;
        Bridge_Alarm3_Triggered = LOW;    
        //Alarm_Flag = 1;  
        ENABLE_BRIDGE_CLOSE = HIGH;   
        Serial.println("Alarm1 Triggered");
        
      }
    }
    if(Bridge_Alarm2_Triggered == HIGH ){
      if(ENABLE_BRIDGE_CLOSE == LOW){  
        Blynk.notify("Alarm 2 Triggered Now");  
        Blynk.virtualWrite(V3,0);   
        Blynk.virtualWrite(V4,0);  
        Blynk.virtualWrite(V5,0);  
        Bridge_Alarm1_Triggered = LOW;
        Bridge_Alarm2_Triggered = LOW;
        Bridge_Alarm3_Triggered = LOW; 
        //Alarm_Flag = 2;     
        ENABLE_BRIDGE_CLOSE = HIGH;   
        Serial.println("Alarm2 Triggered");
        
      }
    }
    if(Bridge_Alarm3_Triggered == HIGH ){
      if(ENABLE_BRIDGE_CLOSE == LOW ){  
        Blynk.notify("Alarm 3 Triggered Now"); 
        Blynk.virtualWrite(V3,0);   
        Blynk.virtualWrite(V4,0);  
        Blynk.virtualWrite(V5,0);  
        Bridge_Alarm1_Triggered = LOW;
        Bridge_Alarm2_Triggered = LOW;
        Bridge_Alarm3_Triggered = LOW;      
        //Alarm_Flag = 0;
        ENABLE_BRIDGE_CLOSE = HIGH;   
        Serial.println("Alarm3 Triggered");
        
      }
    }

    if(ENABLE_BRIDGE_CLOSE == HIGH && ENABLE_BRIDGE_OPEN == LOW){
      CNT_STEP = 0;   
      Serial.println("Close Bridge S1");
      BRIDGE_CLOSE_STATE = TRIGGER_BRIDGE_CLOSE;
      BRIDGE_STATE = BRIDGE_CLOSE;
    }

    if(ENABLE_BRIDGE_OPEN == HIGH){
      Bridge_Open_System_Overall();
    }

    break;

  case BRIDGE_CLOSE:
      Bridge_Close_System_Overall();
    break;
  
  default:
    break;
  }
}
/*================ LED_BLINKING_FUNCTION =================*/
void Bridge_Open_System_Overall(){
  switch (BRIDGE_OPEN_STATE)
  {
  case DROP_NATE:
    //Drop Nate Slowly
      if(Notification_2 == 0){
        Serial.println("Dropping nate now");
        Blynk.notify("Dropping nate now");
        Notification_2 = 1;
      }
      for( Nate_Drop = 0; Nate_Drop <= 180; Nate_Drop++){
        Nate.write(Nate_Drop);
        digitalWrite(BUZZER,HIGH);
        delay(Servo_Move_Delay);
        digitalWrite(BUZZER,LOW);
        delay(Servo_Move_Delay);
      }
    BRIDGE_OPEN_STATE = OPEN_BRIDGE;
    break;

    case OPEN_BRIDGE:
       //Open Bridge Slowly
      if(Notification_2 == 1){
        Serial.println("Opening bridge now");
        Blynk.notify("Opening bridge now");
        Notification_2 = 0;
      }
      for( Bridge_Open_STEP = Bridge2_Close; Bridge_Open_STEP <= Bridge2_Open; Bridge_Open_STEP++){
        Bridge1.write(Bridge_Open_STEP);
        Bridge2.write(Bridge_Open_STEP);
        digitalWrite(BUZZER,HIGH);
        delay(Servo_Move_Delay);
        digitalWrite(BUZZER,LOW);
        delay(Servo_Move_Delay);
      } 
      BRIDGE_OPEN_STATE = OPEN_GATE;
      break;

    case OPEN_GATE:
      //Open Gate Slowly
      if(Notification_2 == 0){
        Serial.println("Opening gate now");
        Blynk.notify("Opening gate now");
        Notification_2 = 1;
      }
      for( Gate_Open_STEP = 180; Gate_Open_STEP >= 90; Gate_Open_STEP--){
        Gate1.write(Gate_Open_STEP);
        Gate2.write(Gate_Open_STEP);
        digitalWrite(BUZZER,HIGH);
        delay(Servo_Move_Delay);
        digitalWrite(BUZZER,LOW);
        delay(Servo_Move_Delay);
        } 
      ENABLE_LED_BLINKING = 0;
      digitalWrite(LED_RED_G1,LOW);
      digitalWrite(LED_RED_G2,LOW);
      digitalWrite(LED_GREEN_G1,HIGH);
      digitalWrite(LED_GREEN_G2,HIGH);
      BRIDGE_OPEN_STATE = IDLE_2;
      break;
  
  default:
    if(Notification_2 == 1){
      Blynk.notify("BRIDGE CAN BE USE NOW");
      ENABLE_BRIDGE_CLOSE = LOW;
      Notification_2 = 0;
      CNT_STEP = 0;
    }
    ENABLE_BRIDGE_OPEN = LOW;
    break;
  }
}
 /*================ Bridge_Close_System_Overall =================*/
void Bridge_Close_System_Overall(){
  switch (BRIDGE_CLOSE_STATE)
  {

    case TRIGGER_BRIDGE_CLOSE://CLOSE GATE FIRST
        ENABLE_LED_BLINKING = 1;    
        if(CNT_STEP > GATE_CLOSE_TIME_DELAY){
          if(Notification_1 == 0){
            ENABLE_LED_BLINKING = 0;  
            Serial.println("BRIDGE WILL NOW CLOSE\nGate Now Close");
            Blynk.notify("BRIDGE WILL NOW CLOSE\nGate now closing");
            digitalWrite(LED_RED_G1,HIGH);
            digitalWrite(LED_RED_G2,HIGH);
            digitalWrite(LED_GREEN_G1,LOW);
            digitalWrite(LED_GREEN_G2,LOW);
            Notification_1 = 1;
          }
          //Clossing Gate Slowly
          for( Gate_Close_STEP = 90; Gate_Close_STEP <= 180; Gate_Close_STEP++){
            Gate1.write(Gate_Close_STEP);
            Gate2.write(Gate_Close_STEP);
            digitalWrite(BUZZER,HIGH);
            delay(Servo_Move_Delay);
            digitalWrite(BUZZER,LOW);
            delay(Servo_Move_Delay);
            }
          BRIDGE_CLOSE_NOW = LOW;
          CNT_STEP = 0;
          BRIDGE_CLOSE_STATE = MAKE_SURE_NO_CAR;
        }  
      break;
    
    case MAKE_SURE_NO_CAR://SCAN NO CAR & PULL NATE SLOWLY
        if(CNT_CAR_G1 == 0 && CNT_CAR_G2 == 0 && BRIDGE_CLOSE_NOW == LOW && CNT_STEP >= DELAY_AFTER_CAR_CNT){
          BRIDGE_CLOSE_NOW = HIGH;
        }
        if(CNT_CAR_G1 > 0){
          CNT_STEP = 0;
        }
        if(CNT_CAR_G2 > 0){
          CNT_STEP = 0;
        }
        
        if(BRIDGE_CLOSE_NOW == HIGH){
        //Clossing Bridge Slowly
          if(Notification_1 == 1){ 
            Serial.println("Bridge now closing and Nate Being pull");
            Blynk.notify("Bridge now closing and Nate Being pull");
            digitalWrite(LED_RED_G1,HIGH);
            digitalWrite(LED_RED_G2,HIGH);
            digitalWrite(LED_GREEN_G1,LOW);
            digitalWrite(LED_GREEN_G2,LOW);
            Notification_1 = 0;
          }
          for( Bridge_Close_STEP = Bridge2_Open; Bridge_Close_STEP >= Bridge2_Close; Bridge_Close_STEP--){
            Bridge1.write(Bridge_Close_STEP);
            Bridge2.write(Bridge_Close_STEP);
            digitalWrite(BUZZER,HIGH);
            delay(Servo_Move_Delay);
            digitalWrite(BUZZER,LOW);
            delay(Servo_Move_Delay);
          }     
        //Pull Nate Slowly
          for( Nate_Pickup = 180; Nate_Pickup >= 0; Nate_Pickup--){
            Nate.write(Nate_Pickup);
            digitalWrite(BUZZER,HIGH);
            delay(Servo_Move_Delay);
            digitalWrite(BUZZER,LOW);
            delay(Servo_Move_Delay);
          }
          CNT_STEP = 0;
          BRIDGE_CLOSE_STATE = ALLOW_SHIP_PASSBY;
        }  
      break;

    case ALLOW_SHIP_PASSBY:// SHIP PASS BY TIMER
        //If time more than BRIDGE_CLOSE_TIME & distance >= 8 then bridge start to open
        if(CNT_STEP >= BRIDGE_CLOSE_TIME && distance >= 10){
          if(Ship_waitime > 10){//more than 10min
            ENABLE_BRIDGE_OPEN = HIGH;
            BRIDGE_OPEN_STATE = DROP_NATE;
            BRIDGE_STATE = BRIDGE_OPEN;
          }
        }else if(distance < 10){
          Ship_waitime = 0;
        }
      break;

    default:
      break;
  }
}
/*================ LED_BLINKING_FUNCTION =================*/
void LED_BLINKING_FUNCTION(){
  if(CNT_STEP < GATE_CLOSE_TIME_DELAY){
    if(ENABLE_LED_BLINKING == 1){
      if(LED_BLINKING == 0){
        LED_BLINKING = 1;
      }else{
        LED_BLINKING = 0;
      }
      digitalWrite(BUZZER,LED_BLINKING);
      digitalWrite(LED_RED_G1,LED_BLINKING);
      digitalWrite(LED_RED_G2,LED_BLINKING);
      digitalWrite(LED_GREEN_G1,LED_BLINKING);
      digitalWrite(LED_GREEN_G2,LED_BLINKING);
    }
  }else{ 
    if(ENABLE_LED_BLINKING == 1){
      digitalWrite(LED_RED_G1,HIGH);
      digitalWrite(LED_RED_G2,HIGH);
      digitalWrite(LED_GREEN_G1,LOW);
      digitalWrite(LED_GREEN_G2,LOW);
      ENABLE_LED_BLINKING = 0; 
    }
      
  }
}
/*================ COUNT_CAR =================*/
void COUNT_CAR(){
  //ROAD G1
    //INCREMENT
    if(IR_ROAD1_IN != Last_IR_ROAD1_IN){
      if(IR_ROAD1_IN == HIGH){
        CNT_CAR_G1++;
      }
      delay(100);
      Last_IR_ROAD1_IN = IR_ROAD1_IN;
    }
    //DECREMENT
    if(IR_ROAD1_EXIT != Last_IR_ROAD1_EXIT){
      if(IR_ROAD1_EXIT == HIGH){
        CNT_CAR_G1--;
        if(CNT_CAR_G1 <= 0){
          CNT_CAR_G1 = 0;
        }
      }
      delay(100);
      Last_IR_ROAD1_EXIT = IR_ROAD1_EXIT;
    }
    //ROAD G2
    //INCREMENT
    if(IR_ROAD2_IN != Last_IR_ROAD2_IN){
      if(IR_ROAD2_IN == HIGH){
        CNT_CAR_G2++;
      }
      delay(100);
      Last_IR_ROAD2_IN = IR_ROAD2_IN;
    }
    //DECREMENT
    if(IR_ROAD2_EXIT != Last_IR_ROAD2_EXIT){
      if(IR_ROAD2_EXIT == HIGH){
        CNT_CAR_G2--;
        if(CNT_CAR_G2 <= 0){
          CNT_CAR_G2 = 0;
        }
      }
      delay(100);
      Last_IR_ROAD2_EXIT = IR_ROAD2_EXIT;
    }
    
}

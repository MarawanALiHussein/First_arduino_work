#include <LiquidCrystal.h>

#include <dht11.h>
dht11 DHT11;

#define DHT11PIN 4
#define FAN_RELAY 2
#define INDOOR_LIGHT_RELAY 5
#define POT A0
#define LDR A1          // 5V - LDR - A1 - 10k - GND
#define reed_switch 3 
#define Buzzer 6

#define PIR_PIN 7       // PIR OUT pin -> Arduino pin 7
#define LED_PIN 13      // built-in LED, used as a motion indicator
#define OUTDOOR_LIGHT_RELAY 8
#define SECURITY_SWITCH 9   // arm/disarm switch between pin 9 and GND
#define INDOOR_LIGHT_SWITCH 10

bool motionDetected = false;
bool securityEnabled = false;   // set only by the security switch

const int THRESHOLD = 400;   //THE BRIGHTNESS VALUE


void setup()
{
  Serial.begin(9600);
  pinMode(reed_switch,INPUT_PULLUP);
  pinMode(SECURITY_SWITCH, INPUT_PULLUP);
  pinMode(INDOOR_LIGHT_SWITCH, INPUT_PULLUP);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(INDOOR_LIGHT_RELAY, OUTPUT);
  pinMode(OUTDOOR_LIGHT_RELAY, OUTPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  

  // PIR sensors need about 30-60 seconds to calibrate after power-up
  Serial.println("PIR warming up...");
  delay(30000);
  Serial.println("PIR ready");
}

void loop()
{
  int a= digitalRead(INDOOR_LIGHT_SWITCH);

  // switch closed (LOW) = armed, switch open (HIGH) = disarmed
  securityEnabled = (digitalRead(SECURITY_SWITCH) == LOW);
  Serial.print("Security: ");
  Serial.println(securityEnabled ? "ARMED" : "DISARMED");

  boolean reedSwitchData = digitalRead(reed_switch);
  Serial.println(reedSwitchData);

  // the buzzer can only sound while the system is armed
  if (reedSwitchData == 1 && securityEnabled ) 
  {
    analogWrite(Buzzer, 5);   
    delay(200);
    digitalWrite(Buzzer, LOW);   
    delay(200);
  } 
  else 
  {
    digitalWrite(Buzzer, LOW);
  }


  int chk = DHT11.read(DHT11PIN);

  // Read potentiometer
  int potValue = analogRead(POT);

  // Convert to temperature range: 20 to 40 C
  int setTemp = map(potValue, 0, 1023, 20, 40);

  Serial.print("Set Temperature (C): ");
  Serial.println(setTemp);

  Serial.print("Humidity (%): ");
  Serial.println(DHT11.humidity);

  Serial.print("Temperature (C): ");
  Serial.println(DHT11.temperature);

  

  if (DHT11.temperature > setTemp)
  {
    digitalWrite(FAN_RELAY, HIGH);
  }
  else
  {
    digitalWrite(FAN_RELAY, LOW);
  }
// LDR CODE
int ldrValue = analogRead(LDR);

  if (ldrValue < THRESHOLD || a == LOW)
{
    digitalWrite(INDOOR_LIGHT_RELAY, HIGH);
}
else
{
    digitalWrite(INDOOR_LIGHT_RELAY, LOW);
}

  Serial.print("LDR: ");
  Serial.println(ldrValue);

   motionDetected = digitalRead(PIR_PIN);   // HIGH = motion, LOW = no motion

  // CHANGED: the outdoor light only reacts to motion while the system is armed
  if (securityEnabled && motionDetected)
  {
    digitalWrite(OUTDOOR_LIGHT_RELAY, HIGH);
    Serial.println("Motion detected!");
  }
  else
  {
    digitalWrite(OUTDOOR_LIGHT_RELAY, LOW);
    Serial.println("No motion");
  }

  
  delay(5000);
}
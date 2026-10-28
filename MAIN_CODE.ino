
#include <dht11.h>

#define DHT11PIN 4
#define FAN 2
#define POT A0

dht11 DHT11;

void setup()
{
  Serial.begin(9600);
  pinMode(FAN, OUTPUT);
}

void loop()
{
  int chk = DHT11.read(DHT11PIN);

  // Read potentiometer
  int potValue = analogRead(POT);

  // Convert to temperature range: 20 to 40 C
  int setTemp = map(potValue, 0, 1023, 20, 40);

  Serial.print("Humidity (%): ");
  Serial.println(DHT11.humidity);

  Serial.print("Temperature (C): ");
  Serial.println(DHT11.temperature);

  Serial.print("Set Temperature (C): ");
  Serial.println(setTemp);

  if (DHT11.temperature > setTemp)
  {
    digitalWrite(FAN, HIGH);
  }
  else
  {
    digitalWrite(FAN, LOW);
  }

  delay(2000);
}

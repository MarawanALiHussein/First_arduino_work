const int TEMP_PIN = A1;
const int POT_PIN  = A2;
const int BTN_PIN  = 2;
const int FAN_PIN  = 10;   // LED stands in for the fan

const float HYST = 0.5;          // hysteresis in °C
const int POT_DEADBAND = 20;     // pot change (0-1023) that cancels manual mode

bool fanOn = false;
bool manualMode = false;
bool manualFan = false;
int potAtPress = 0;

// button debounce
bool btnState = HIGH;
bool lastReading = HIGH;
unsigned long lastDebounce = 0;

// serial print timing
unsigned long lastPrint = 0;

float readTemp() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(TEMP_PIN);
  }
  float voltage = (sum / 10.0) * 5.0 / 1023.0;
  return (voltage - 0.5) * 100.0;   // TMP36
}

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  float temp = readTemp();
  int potRaw = analogRead(POT_PIN);
  float threshold = 20.0 + (potRaw / 1023.0) * 20.0;

  // ---- button with debounce ----
  bool reading = digitalRead(BTN_PIN);
  if (reading != lastReading) {
    lastDebounce = millis();
  }
  if ((millis() - lastDebounce) > 50 && reading != btnState) {
    btnState = reading;
    if (btnState == LOW) {            // button just pressed
      manualMode = true;
      manualFan = !fanOn;             // toggle the current fan state
      potAtPress = potRaw;            // remember where the pot was
    }
  }
  lastReading = reading;

  // ---- moving the pot cancels manual mode ----
  if (manualMode && abs(potRaw - potAtPress) > POT_DEADBAND) {
    manualMode = false;
  }

  // ---- decide fan state ----
  if (manualMode) {
    fanOn = manualFan;
  } else {
    if (temp > threshold + HYST) {
      fanOn = true;
    } else if (temp < threshold - HYST) {
      fanOn = false;
    }
  }

  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);

  // ---- print every 200 ms (without delay) ----
  if (millis() - lastPrint >= 200) {
    lastPrint = millis();
    Serial.print("Temp: ");
    Serial.print(temp, 1);
    Serial.print(" C | Threshold: ");
    Serial.print(threshold, 1);
    Serial.print(" C | Mode: ");
    Serial.print(manualMode ? "MANUAL" : "AUTO");
    Serial.print(" | Fan: ");
    Serial.println(fanOn ? "ON" : "OFF");
  }
}
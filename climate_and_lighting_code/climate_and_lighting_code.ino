// ---------- Pins ----------
const int LDR_PIN       = A0;
const int TEMP_PIN      = A1;
const int POT_PIN       = A2;
const int FAN_BTN_PIN   = 2;
const int LIGHT_BTN_PIN = 3;
const int LIGHT_PIN     = 9;    // PWM (fade)
const int FAN_PIN       = 10;   // LED stands in for the fan

// ---------- Climate settings ----------
const float BETA = 3950;        // NTC beta coefficient
const float HYST = 0.5;         // °C
const int POT_DEADBAND = 20;

// ---------- Lighting settings (calibrate these!) ----------
const bool DARK_IS_HIGH = true; // true if the LDR reading rises when it gets dark
const int DARK_ON  = 600;       // darkness above this -> light ON
const int DARK_OFF = 500;       // darkness below this -> light OFF
const int LIGHT_DEADBAND = 100; // ambient change that cancels manual mode
const int FADE_STEP = 5;        // brightness change per step
const int FADE_INTERVAL = 10;   // ms between steps (255/5*10 = ~0.5 s fade)

// ---------- Button helper (debounced) ----------
struct Button {
  uint8_t pin;
  bool state;
  bool last;
  unsigned long t;

  void begin(uint8_t p) {
    pin = p;
    pinMode(pin, INPUT_PULLUP);
    state = HIGH;
    last = HIGH;
    t = 0;
  }

  bool pressed() {                 // true once per press
    bool r = digitalRead(pin);
    bool event = false;
    if (r != last) t = millis();
    if ((millis() - t) > 50 && r != state) {
      state = r;
      if (state == LOW) event = true;
    }
    last = r;
    return event;
  }
};

Button fanBtn, lightBtn;

// ---------- State ----------
bool fanOn = false, fanManual = false, manualFan = false;
int potAtPress = 0;

bool lightOn = false, lightManual = false, manualLight = false;
int darkAtPress = 0;
int brightness = 0;
unsigned long lastFade = 0;

unsigned long lastPrint = 0;

// ---------- Sensors ----------
float readTemp() {
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(TEMP_PIN);
  float adc = sum / 10.0;
  if (adc < 1 || adc > 1022) return -99.0;   // sensor fault guard
  return 1.0 / (log(1.0 / (1023.0 / adc - 1.0)) / BETA + 1.0 / 298.15) - 273.15;
}

int readDarkness() {               // 0 = bright, 1023 = dark
  long sum = 0;
  for (int i = 0; i < 10; i++) sum += analogRead(LDR_PIN);
  int v = sum / 10;
  return DARK_IS_HIGH ? v : (1023 - v);
}

// ---------- Fade ----------
void updateFade(int target) {
  if (millis() - lastFade >= FADE_INTERVAL) {
    lastFade = millis();
    if (brightness < target)      brightness = min(brightness + FADE_STEP, target);
    else if (brightness > target) brightness = max(brightness - FADE_STEP, target);
    analogWrite(LIGHT_PIN, brightness);
  }
}

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  fanBtn.begin(FAN_BTN_PIN);
  lightBtn.begin(LIGHT_BTN_PIN);
  Serial.begin(9600);
}

void loop() {
  // ================= CLIMATE =================
  float temp = readTemp();
  int potRaw = analogRead(POT_PIN);
  float threshold = 20.0 + (potRaw / 1023.0) * 20.0;

  if (fanBtn.pressed()) {
    fanManual = true;
    manualFan = !fanOn;
    potAtPress = potRaw;
  }
  if (fanManual && abs(potRaw - potAtPress) > POT_DEADBAND) {
    fanManual = false;
  }

  if (fanManual) {
    fanOn = manualFan;
  } else {
    if (temp > threshold + HYST)      fanOn = true;
    else if (temp < threshold - HYST) fanOn = false;
  }
  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);

  // ================= LIGHTING =================
  int darkness = readDarkness();

  if (lightBtn.pressed()) {
    lightManual = true;
    manualLight = !lightOn;
    darkAtPress = darkness;
  }
  if (lightManual && abs(darkness - darkAtPress) > LIGHT_DEADBAND) {
    lightManual = false;
  }

  if (lightManual) {
    lightOn = manualLight;
  } else {
    if (darkness > DARK_ON)       lightOn = true;
    else if (darkness < DARK_OFF) lightOn = false;
  }
  updateFade(lightOn ? 255 : 0);

  // ================= SERIAL =================
  if (millis() - lastPrint >= 200) {
    lastPrint = millis();
    Serial.print("Temp: ");      Serial.print(temp, 1);
    Serial.print(" | Thr: ");    Serial.print(threshold, 1);
    Serial.print(" | Fan: ");    Serial.print(fanOn ? "ON" : "OFF");
    Serial.print(fanManual ? " (MAN)" : " (AUTO)");
    Serial.print(" || Dark: ");  Serial.print(darkness);
    Serial.print(" | Light: ");  Serial.print(lightOn ? "ON" : "OFF");
    Serial.print(lightManual ? " (MAN)" : " (AUTO)");
    Serial.print(" | PWM: ");    Serial.println(brightness);
  }
}
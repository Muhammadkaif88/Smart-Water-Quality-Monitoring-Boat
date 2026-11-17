/*
 * ============================================================================
 * 🔍 ESP32 Multi-Pin Live Scanner & pH Sensor Calibration
 * ============================================================================
 * This tool reads all common analog pins simultaneously.
 * When you touch 3.3V with your wire or connect your pH sensor,
 * you will instantly see WHICH PIN lights up on the screen!
 * ============================================================================
 */

// List of all ADC1 pins on ESP32
const int ADC_PINS[] = {34, 35, 32, 33, 36, 39};
const char* PIN_NAMES[] = {"GPIO 34 (Default pH)", 
                           "GPIO 35 (Turbidity)", 
                           "GPIO 32 (TDS)", 
                           "GPIO 33", 
                           "GPIO 36 (VP)", 
                           "GPIO 39 (VN)"};
const int NUM_PINS = 6;

void setup() {
  Serial.begin(115200);
  delay(1000);

  analogReadResolution(12); // 12-bit ADC (0 - 4095)

  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(ADC_PINS[i], INPUT);
  }

  Serial.println("\n=======================================================");
  Serial.println("       🔍 ESP32 ALL-PIN VOLTAGE SCANNER               ");
  Serial.println("=======================================================");
  Serial.println("👉 Touch your wire to 3.3V or connect your sensor.");
  Serial.println("👉 Watch which pin below shows ~3.300 V or changes voltage!");
  Serial.println("=======================================================\n");
  delay(1500);
}

float readAverageVoltage(int pin) {
  long sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(pin);
    delay(2);
  }
  float avgAdc = sum / 20.0;
  return (avgAdc / 4095.0) * 3.3;
}

unsigned long lastPrint = 0;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastPrint >= 800) {
    lastPrint = currentMillis;

    Serial.println("----------------- 📊 LIVE PIN READINGS -----------------");
    for (int i = 0; i < NUM_PINS; i++) {
      int pin = ADC_PINS[i];
      int rawAdc = analogRead(pin);
      float voltage = (rawAdc / 4095.0) * 3.3;

      // Indicator if pin has signal (> 0.2V)
      String indicator = "";
      if (voltage >= 3.0) {
        indicator = "  <-- 🔥 3.3V DETECTED HERE!";
      } else if (voltage >= 0.5) {
        indicator = "  <-- ⚡ SENSOR ACTIVE HERE!";
      }

      Serial.printf(" [%-20s] : %5.3f V  (ADC: %4d)%s\n", 
                    PIN_NAMES[i], voltage, rawAdc, indicator.c_str());
    }
    Serial.println("--------------------------------------------------------\n");
  }
}

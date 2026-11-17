# 🚤 Smart Water Quality Monitoring & Remote Control Boat

An IoT-based autonomous water monitoring boat controlled over Wi-Fi with real-time telemetry of **pH, Turbidity, TDS, Water Temperature (DS18B20)** and **Fan Relay Control**.

---

## 📱 How the Mobile App Works (Zero Latency WebSockets)
The ESP32 creates its own dedicated Wi-Fi hotspot (**Access Point**). **No internet or external router is needed!**

1. Turn ON the Boat / ESP32.
2. Connect your mobile phone's Wi-Fi to:
   - **SSID:** `Water_Monitoring_Boat`
   - **Password:** `boat12345`
3. Open any mobile browser (Chrome / Safari) and go to:
   - **URL:** `http://192.168.4.1`
4. *(Optional)* Tap **"Add to Home Screen"** in your browser to install it as an App on your phone!

---

## ⚡ Circuit Pin Connections

| Module / Sensor | ESP32 Pin | Extra Details |
| :--- | :--- | :--- |
| **L298N IN1 (Left Motor Fwd)** | `GPIO 26` | Digital Output |
| **L298N IN2 (Left Motor Back)** | `GPIO 27` | Digital Output |
| **L298N IN3 (Right Motor Fwd)** | `GPIO 14` | Digital Output |
| **L298N IN4 (Right Motor Back)** | `GPIO 12` | Digital Output |
| **L298N ENA (Left Motor Speed)** | `GPIO 18` | **Remove ENA Jumper** & connect to ESP32 PWM |
| **L298N ENB (Right Motor Speed)**| `GPIO 19` | **Remove ENB Jumper** & connect to ESP32 PWM |
| **L298N VCC / GND** | `12V Battery / GND` | **Common GND** with ESP32 |
| **Relay Module (Fan Control)** | `GPIO 13` | Connect Fan to Relay COM & NO |
| **DS18B20 Temp Data** | `GPIO 4` | Needs **4.7kΩ Pull-up Resistor** between Data & 3.3V |
| **pH Sensor Analog Pin (Po)** | `GPIO 34` | ADC1 (Input Only) |
| **Turbidity Sensor Analog (Aout)** | `GPIO 35` | ADC1 (Input Only) |
| **TDS Sensor Analog Pin** | `GPIO 32` | ADC1 (Input Only) |
| **Sensor Power (VCC & GND)** | `5V / 3.3V & GND` | Connect to 5V (or 3.3V) and shared GND |

> ⚠️ **Important:** 
> 1. Ensure all grounds (Battery GND, ESP32 GND, L298N GND, Sensor GND) are connected together (**Common Ground**).
> 2. **Remove the black jumpers on ENA and ENB** on the L298N board so that ESP32 can control motor speeds via PWM!

---

## 🛠️ Arduino IDE Setup

1. In Arduino IDE, open **Tools > Manage Libraries...**
2. Install the following libraries:
   - `WebSockets` by **Markus Sattler**
   - `OneWire` by **Paul Stoffregen**
   - `DallasTemperature` by **Miles Burton**
3. Select Board: **ESP32 Dev Module** (or your specific ESP32 board).
4. Upload `Smart_Water_Boat.ino`.

---

## 🎨 Features in the Mobile App UI
- **Live Gauge Cards:**
  - **pH Level:** Automatic acidic/neutral/alkaline status detection.
  - **Water Temperature:** Real-time DS18B20 reading in °C.
  - **Turbidity:** Calibrated NTU index with status indicators.
  - **TDS Level:** Calibrated ppm dissolved solids.
- **Fan Switch:** Dedicated toggle switch to turn ON/OFF cooling/exhaust fan via Relay.
- **⚡ Speed Control Slider:** Smooth 0% - 100% PWM speed control + Quick Preset buttons (Slow 45%, Cruise 75%, Turbo 100%).
- **D-Pad Boat Controller:** Touch-and-hold buttons for Forward, Backward, Left, Right & STOP.

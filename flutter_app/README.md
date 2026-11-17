# 📱 Aqua Boat Flutter App

A modern, responsive Flutter mobile app for the **Smart Water Monitoring & Remote Control Boat** project.

---

## 🌟 Features
- **Real-time WebSocket Telemetry:** Connects directly to `ws://192.168.4.1:81` with auto-reconnect.
- **Sensors Live Display:**
  - **pH Level** with color-coded safety indicators (Acidic/Neutral/Alkaline).
  - **Water Temperature** in °C (DS18B20).
  - **Turbidity** in NTU (Calibrated scale).
  - **TDS** in ppm (Calibrated scale).
- **Fan Control:** Animated rotating fan switch communicating via ESP32 Relay.
- **Speed Control:** 0% - 100% PWM speed slider with Slow, Cruise, and Turbo presets.
- **D-Pad Boat Controller:** Touch-and-hold buttons for zero-latency steering.
- **Customizable IP Settings:** Allows changing the ESP32 IP address if connected via router.

---

## 🚀 How to Run the App

1. Open terminal inside the `flutter_app` folder:
   ```bash
   cd flutter_app
   ```
2. Get dependencies:
   ```bash
   flutter pub get
   ```
3. Run on connected Android / iOS phone:
   ```bash
   flutter run
   ```
4. Or build an APK:
   ```bash
   flutter build apk --release
   ```
   The APK will be generated at:
   `build/app/outputs/flutter-apk/app-release.apk`

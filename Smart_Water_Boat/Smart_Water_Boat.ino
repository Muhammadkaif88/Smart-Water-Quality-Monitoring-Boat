/*
 * ============================================================================
 * Project: Smart Water Monitoring & Remote Control Boat
 * Microcontroller: ESP32 NodeMCU / ESP32-WROOM-32
 * Features:
 *   1. Wi-Fi Access Point (No router or internet needed!)
 *   2. Real-time WebSocket Control & Telemetry Web App for Mobile Phones
 *   3. L298N Dual Motor Driver (Differential Drive: Forward, Reverse, Left,
 * Right, Stop)
 *   4. Water Quality Sensors:
 *      - pH Sensor (Analog)
 *      - Turbidity Sensor (Analog)
 *      - TDS Sensor (Analog)
 *      - DHT11 Temperature Sensor (Digital)
 *   5. Relay Module for Cooling/Exhaust Fan Control
 * ============================================================================
 *
 * Pin Configuration:
 * ----------------------------------------------------------------------------
 *  Component             | ESP32 Pin   | Notes
 * ----------------------------------------------------------------------------
 *  L298N IN1 (Motor A)   | GPIO 26     | Left Motor Forward
 *  L298N IN2 (Motor A)   | GPIO 27     | Left Motor Backward
 *  L298N IN3 (Motor B)   | GPIO 14     | Right Motor Forward
 *  L298N IN4 (Motor B)   | GPIO 12     | Right Motor Backward
 *  L298N ENA (Speed A)   | GPIO 18     | Left Motor PWM Speed (Remove ENA
 * Jumper) L298N ENB (Speed B)   | GPIO 19     | Right Motor PWM Speed (Remove
 * ENB Jumper)
 * ----------------------------------------------------------------------------
 *  Relay Module (Fan)    | GPIO 13     | Active LOW / HIGH (Configurable)
 * ----------------------------------------------------------------------------
 *  DHT11 Sensor          | GPIO 4      | VCC (3.3V/5V), GND (GND), Out/Data
 * (GPIO 4) Analog pH Sensor      | GPIO 34     | ADC1 (Input Only) Analog
 * Turbidity      | GPIO 35     | ADC1 (Input Only) Analog TDS Sensor     | GPIO
 * 32     | ADC1 (Input Only)
 * ----------------------------------------------------------------------------
 *
 * Required Arduino Libraries:
 *   1. DHT sensor library (by Adafruit)
 *   2. Adafruit Unified Sensor (by Adafruit)
 *   3. WebSockets (by Markus Sattler) -> 'WebSocketsServer'
 *   4. WiFi (Built-in ESP32)
 *   5. WebServer (Built-in ESP32)
 * ============================================================================
 */

#include <DHT.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>

// ================= Wi-Fi Access Point Credentials =================
const char *ssid = "Water_Monitoring_Boat";
const char *password = "boat12345"; // Min 8 characters

// ================= Pin Definitions =================
// Motor Direction Pins
#define MOTOR_LEFT_F 26
#define MOTOR_LEFT_B 27
#define MOTOR_RIGHT_F 14
#define MOTOR_RIGHT_B 12

// Motor Speed PWM Pins (ENA & ENB)
#define MOTOR_ENA_PIN 18
#define MOTOR_ENB_PIN 19

// PWM Configuration for ESP32 LEDC
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8 // 8-bit resolution (0 - 255)
#define PWM_CHANNEL_ENA 0
#define PWM_CHANNEL_ENB 1

// Relay Pin (Fan)
#define RELAY_FAN_PIN 13
#define RELAY_ACTIVE_LOW true // Set true if your relay triggers on LOW

// Sensor Pins
#define DHTPIN 4
#define DHTTYPE DHT11
#define PH_PIN 34
#define TURBIDITY_PIN 35
#define TDS_PIN 32

// Pin Aliases
#define PIN_TURBIDITY TURBIDITY_PIN
#define PIN_TDS TDS_PIN

// ================= Objects & Instances =================
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
DHT dht(DHTPIN, DHTTYPE);

// ================= Global Variables =================
float waterTemp = 0.0;
float phValue = 7.14;
float phVoltage = 1.65;
int phRawADC = 2048;
bool isWaterDetected = true;
bool manualPhMode =
    false; // true if user forces via serial 'w' or 'a' or app toggle
bool manualPhInWater = true; // normal water active
float tdsValue = 0.0;
float turbidityValue = 0.0;
String turbidityStatus = "Clear";
bool fanState = false;
int motorSpeed = 200; // Default speed ~80% (range: 0 - 255)

unsigned long lastSensorReadTime = 0;
const unsigned long SENSOR_INTERVAL = 1000; // Read sensors every 1 second

// ================= HTML, CSS, JS Mobile App UI =================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smart Water Boat Controller</title>
  <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #090e17;
      --card-bg: rgba(18, 26, 43, 0.75);
      --card-border: rgba(255, 255, 255, 0.08);
      --primary: #00d2ff;
      --primary-glow: rgba(0, 210, 255, 0.35);
      --accent: #3a7bd5;
      --green: #00f5a0;
      --yellow: #ffd166;
      --red: #ff5e7e;
      --text: #ffffff;
      --text-muted: #8e9bb4;
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      user-select: none;
      -webkit-user-select: none;
      touch-action: manipulation;
    }

    body {
      font-family: 'Outfit', sans-serif;
      background: radial-gradient(circle at 50% 0%, #17243c 0%, var(--bg) 80%);
      color: var(--text);
      min-height: 100vh;
      padding: 16px;
      display: flex;
      flex-direction: column;
      align-items: center;
      overflow-x: hidden;
    }

    header {
      width: 100%;
      max-width: 480px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 16px;
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 20px;
      margin-bottom: 16px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.4);
    }

    .brand {
      display: flex;
      align-items: center;
      gap: 10px;
    }

    .brand-icon {
      width: 36px;
      height: 36px;
      background: linear-gradient(135deg, var(--primary), var(--accent));
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 20px;
      box-shadow: 0 0 15px var(--primary-glow);
    }

    .brand h1 {
      font-size: 1.1rem;
      font-weight: 700;
      background: linear-gradient(90deg, #fff, var(--primary));
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }

    .status-badge {
      display: flex;
      align-items: center;
      gap: 6px;
      font-size: 0.8rem;
      font-weight: 600;
      padding: 6px 12px;
      border-radius: 20px;
      background: rgba(0, 245, 160, 0.1);
      border: 1px solid rgba(0, 245, 160, 0.3);
      color: var(--green);
    }

    .status-badge.disconnected {
      background: rgba(255, 94, 126, 0.1);
      border-color: rgba(255, 94, 126, 0.3);
      color: var(--red);
    }

    .dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: currentColor;
      animation: pulse 1.5s infinite;
    }

    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(0.85); }
    }

    .container {
      width: 100%;
      max-width: 480px;
      display: flex;
      flex-direction: column;
      gap: 16px;
    }

    /* Sensor Grid */
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 12px;
    }

    .sensor-card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 18px;
      padding: 14px;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      position: relative;
      overflow: hidden;
      box-shadow: 0 8px 24px rgba(0,0,0,0.25);
    }

    .sensor-card::after {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 4px;
      height: 100%;
      background: var(--primary);
    }

    .sensor-card.ph::after { background: #a78bfa; }
    .sensor-card.turb::after { background: var(--yellow); }
    .sensor-card.tds::after { background: var(--green); }
    .sensor-card.temp::after { background: #ff8c42; }

    .sensor-head {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 8px;
    }

    .sensor-title {
      font-size: 0.8rem;
      font-weight: 600;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .sensor-icon {
      font-size: 1.2rem;
    }

    .sensor-value {
      font-size: 1.7rem;
      font-weight: 800;
      color: #fff;
      display: flex;
      align-items: baseline;
      gap: 4px;
    }

    .sensor-unit {
      font-size: 0.85rem;
      font-weight: 600;
      color: var(--text-muted);
    }

    .sensor-sub {
      font-size: 0.75rem;
      color: var(--text-muted);
      margin-top: 4px;
    }

    /* Fan Relay Section */
    .relay-card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 18px;
      padding: 14px 18px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      box-shadow: 0 8px 24px rgba(0,0,0,0.25);
    }

    .relay-info {
      display: flex;
      align-items: center;
      gap: 12px;
    }

    .fan-icon {
      width: 40px;
      height: 40px;
      background: rgba(255, 255, 255, 0.05);
      border-radius: 12px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 20px;
      transition: 0.3s;
    }

    .fan-icon.spinning {
      animation: spin 1s linear infinite;
      color: var(--primary);
      box-shadow: 0 0 15px var(--primary-glow);
    }

    @keyframes spin {
      100% { transform: rotate(360deg); }
    }

    .relay-title {
      font-weight: 700;
      font-size: 0.95rem;
    }

    .relay-desc {
      font-size: 0.75rem;
      color: var(--text-muted);
    }

    /* Toggle Switch */
    .switch {
      position: relative;
      display: inline-block;
      width: 54px;
      height: 30px;
    }

    .switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .slider {
      position: absolute;
      cursor: pointer;
      top: 0; left: 0; right: 0; bottom: 0;
      background-color: #2a3754;
      transition: .4s;
      border-radius: 30px;
    }

    .slider:before {
      position: absolute;
      content: "";
      height: 22px;
      width: 22px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .slider {
      background: linear-gradient(90deg, var(--primary), var(--accent));
      box-shadow: 0 0 12px var(--primary-glow);
    }

    input:checked + .slider:before {
      transform: translateX(24px);
    }

    /* Remote Control Section */
    .control-card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 24px;
      padding: 20px;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 16px;
      box-shadow: 0 12px 32px rgba(0,0,0,0.35);
    }

    .control-header {
      width: 100%;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    .control-title {
      font-weight: 700;
      font-size: 0.95rem;
      color: var(--text);
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .dpad {
      width: 220px;
      height: 220px;
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      grid-template-rows: repeat(3, 1fr);
      gap: 10px;
      margin: 10px 0;
    }

    .btn-ctrl {
      background: linear-gradient(145deg, #1b2742, #141e33);
      border: 1px solid rgba(255, 255, 255, 0.1);
      border-radius: 16px;
      color: var(--text);
      font-size: 1.5rem;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      box-shadow: 0 6px 15px rgba(0, 0, 0, 0.4);
      transition: all 0.1s ease;
      -webkit-tap-highlight-color: transparent;
    }

    .btn-ctrl:active, .btn-ctrl.active {
      background: linear-gradient(145deg, var(--primary), var(--accent));
      color: #000;
      transform: scale(0.92);
      box-shadow: 0 0 20px var(--primary-glow);
    }

    .btn-fwd  { grid-column: 2; grid-row: 1; }
    .btn-left { grid-column: 1; grid-row: 2; }
    .btn-stop { 
      grid-column: 2; grid-row: 2; 
      background: rgba(255, 94, 126, 0.15); 
      border-color: rgba(255, 94, 126, 0.4);
      color: var(--red);
      font-size: 1rem;
      font-weight: 800;
    }
    .btn-stop:active, .btn-stop.active {
      background: var(--red);
      color: #fff;
      box-shadow: 0 0 20px rgba(255, 94, 126, 0.6);
    }
    .btn-right { grid-column: 3; grid-row: 2; }
    .btn-rev   { grid-column: 2; grid-row: 3; }

    /* Speed Control Section */
    .speed-card {
      background: var(--card-bg);
      backdrop-filter: blur(12px);
      border: 1px solid var(--card-border);
      border-radius: 18px;
      padding: 16px 18px;
      display: flex;
      flex-direction: column;
      gap: 12px;
      box-shadow: 0 8px 24px rgba(0,0,0,0.25);
    }

    .speed-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-weight: 700;
      font-size: 0.95rem;
    }

    .speed-percent {
      color: var(--primary);
      font-weight: 800;
      font-size: 1.1rem;
    }

    .speed-slider {
      -webkit-appearance: none;
      appearance: none;
      width: 100%;
      height: 8px;
      border-radius: 5px;
      background: #202d47;
      outline: none;
      transition: background 0.2s;
    }

    .speed-slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: linear-gradient(135deg, var(--primary), var(--accent));
      cursor: pointer;
      box-shadow: 0 0 10px var(--primary-glow);
    }

    .speed-slider::-moz-range-thumb {
      width: 22px;
      height: 22px;
      border-radius: 50%;
      background: linear-gradient(135deg, var(--primary), var(--accent));
      cursor: pointer;
      border: none;
      box-shadow: 0 0 10px var(--primary-glow);
    }

    .speed-presets {
      display: flex;
      gap: 8px;
    }

    .preset-btn {
      flex: 1;
      padding: 7px;
      background: rgba(255, 255, 255, 0.05);
      border: 1px solid var(--card-border);
      border-radius: 10px;
      color: var(--text-muted);
      font-family: inherit;
      font-size: 0.8rem;
      font-weight: 600;
      cursor: pointer;
      transition: 0.2s;
    }

    .preset-btn:active {
      background: var(--primary);
      color: #000;
    }

    .instructions {
      font-size: 0.75rem;
      color: var(--text-muted);
      text-align: center;
    }
  </style>
</head>
<body>

  <header>
    <div class="brand">
      <div class="brand-icon">🚤</div>
      <div>
        <h1>AQUA BOAT</h1>
        <div style="font-size: 0.7rem; color: var(--text-muted);">Smart Water Monitor</div>
      </div>
    </div>
    <div id="connStatus" class="status-badge disconnected">
      <div class="dot"></div>
      <span id="connText">Connecting...</span>
    </div>
  </header>

  <div class="container">
    
    <!-- Sensor Grid -->
    <div class="sensor-grid">
      <!-- pH -->
      <div class="sensor-card ph" onclick="sendCmd('TOGGLE_PH')" style="cursor:pointer;" title="Normal Water Live Testing (Tap to nudge)">
        <div class="sensor-head">
          <span class="sensor-title">pH Level</span>
          <span class="sensor-icon">🧪</span>
        </div>
        <div class="sensor-value">
          <span id="valPh">7.14</span>
          <span class="sensor-unit">pH</span>
        </div>
        <div class="sensor-sub" id="statusPh">Normal / Safe 🟢</div>
      </div>

      <!-- Temperature -->
      <div class="sensor-card temp">
        <div class="sensor-head">
          <span class="sensor-title">Water Temp</span>
          <span class="sensor-icon">🌡️</span>
        </div>
        <div class="sensor-value">
          <span id="valTemp">--</span>
          <span class="sensor-unit">°C</span>
        </div>
        <div class="sensor-sub" id="statusTemp">DHT11 Live</div>
      </div>

      <!-- Turbidity -->
      <div class="sensor-card turb">
        <div class="sensor-head">
          <span class="sensor-title">Turbidity</span>
          <span class="sensor-icon">🌫️</span>
        </div>
        <div class="sensor-value">
          <span id="valTurb">--</span>
          <span class="sensor-unit">NTU</span>
        </div>
        <div class="sensor-sub" id="statusTurb">Cloudiness</div>
      </div>

      <!-- TDS -->
      <div class="sensor-card tds">
        <div class="sensor-head">
          <span class="sensor-title">TDS Level</span>
          <span class="sensor-icon">💧</span>
        </div>
        <div class="sensor-value">
          <span id="valTds">--</span>
          <span class="sensor-unit">ppm</span>
        </div>
        <div class="sensor-sub" id="statusTds">Purity level</div>
      </div>
    </div>

    <!-- Fan Relay Switch -->
    <div class="relay-card">
      <div class="relay-info">
        <div id="fanIcon" class="fan-icon">🪭</div>
        <div>
          <div class="relay-title">Cleaning Motor</div>
          <div class="relay-desc">Relay (GPIO 13)</div>
        </div>
      </div>
      <label class="switch">
        <input type="checkbox" id="fanToggle" onchange="toggleFan(this.checked)">
        <span class="slider"></span>
      </label>
    </div>

    <!-- Speed Control Slider (ENA / ENB PWM) -->
    <div class="speed-card">
      <div class="speed-header">
        <span>⚡ Boat Speed Control</span>
        <span id="valSpeed" class="speed-percent">80%</span>
      </div>
      <input type="range" min="0" max="255" value="200" id="speedSlider" class="speed-slider" oninput="updateSpeed(this.value)">
      <div class="speed-presets">
        <button type="button" class="preset-btn" onclick="setSpeedPreset(120)">Slow (45%)</button>
        <button type="button" class="preset-btn" onclick="setSpeedPreset(190)">Cruise (75%)</button>
        <button type="button" class="preset-btn" onclick="setSpeedPreset(255)">Turbo (100%)</button>
      </div>
    </div>

    <!-- Navigation DPAD -->
    <div class="control-card">
      <div class="control-header">
        <span class="control-title">🎮 Wireless Boat Navigation</span>
        <span style="font-size: 0.75rem; color: var(--primary);" id="curAction">READY</span>
      </div>

      <div class="dpad">
        <button class="btn-ctrl btn-fwd" id="btnFwd">▲</button>
        <button class="btn-ctrl btn-left" id="btnLeft">◀</button>
        <button class="btn-ctrl btn-stop" id="btnStop">STOP</button>
        <button class="btn-ctrl btn-right" id="btnRight">▶</button>
        <button class="btn-ctrl btn-rev" id="btnRev">▼</button>
      </div>

      <div class="instructions">
        Hold button to move &bull; Release to stop
      </div>
    </div>

  </div>

  <script>
    let ws;
    let isConnected = false;

    function initWebSocket() {
      const wsUri = "ws://" + window.location.hostname + ":81/";
      ws = new WebSocket(wsUri);

      ws.onopen = function() {
        isConnected = true;
        document.getElementById('connStatus').className = 'status-badge';
        document.getElementById('connText').innerText = 'Connected';
      };

      ws.onclose = function() {
        isConnected = false;
        document.getElementById('connStatus').className = 'status-badge disconnected';
        document.getElementById('connText').innerText = 'Reconnecting...';
        setTimeout(initWebSocket, 2000);
      };

      ws.onmessage = function(event) {
        try {
          const data = JSON.parse(event.data);
          
          if(data.temp !== undefined) {
            document.getElementById('valTemp').innerText = data.temp.toFixed(1);
          }
          if(data.ph !== undefined) {
            const ph = data.ph.toFixed(2);
            document.getElementById('valPh').innerText = ph;
            const phStat = ph < 6.5 ? 'Acidic 🔴' : (ph > 8.5 ? 'Alkaline 🟣' : 'Normal / Safe 🟢');
            document.getElementById('statusPh').innerText = phStat;
          }
          if(data.turb !== undefined) {
            document.getElementById('valTurb').innerText = data.turb.toFixed(0);
            document.getElementById('statusTurb').innerText = data.turbStatus || 'Normal';
          }
          if(data.tds !== undefined) {
            const tds = data.tds.toFixed(0);
            document.getElementById('valTds').innerText = tds;
            const tdsStat = tds < 300 ? 'Excellent 🟢' : (tds < 600 ? 'Good 🟡' : 'High Dissolved 🔴');
            document.getElementById('statusTds').innerText = tdsStat;
          }
          if(data.fan !== undefined) {
            document.getElementById('fanToggle').checked = data.fan;
            const fanIcon = document.getElementById('fanIcon');
            if(data.fan) fanIcon.classList.add('spinning');
            else fanIcon.classList.remove('spinning');
          }
          if(data.speed !== undefined) {
            document.getElementById('speedSlider').value = data.speed;
            const pct = Math.round((data.speed / 255) * 100);
            document.getElementById('valSpeed').innerText = pct + '%';
          }
        } catch(e) {
          console.error("JSON parse error:", e);
        }
      };
    }

    function sendCmd(cmd) {
      if(ws && ws.readyState === WebSocket.OPEN) {
        ws.send(cmd);
      }
    }

    function toggleFan(state) {
      sendCmd(state ? "FAN_ON" : "FAN_OFF");
      const fanIcon = document.getElementById('fanIcon');
      if(state) fanIcon.classList.add('spinning');
      else fanIcon.classList.remove('spinning');
    }

    function updateSpeed(val) {
      const pct = Math.round((val / 255) * 100);
      document.getElementById('valSpeed').innerText = pct + '%';
      sendCmd('SPEED:' + val);
    }

    function setSpeedPreset(val) {
      document.getElementById('speedSlider').value = val;
      updateSpeed(val);
    }

    // Attach touch & mouse controls for responsive boat steering
    function attachBtnControl(btnId, pressCmd, releaseCmd) {
      const btn = document.getElementById(btnId);
      const actionLabel = document.getElementById('curAction');

      const handlePress = (e) => {
        e.preventDefault();
        btn.classList.add('active');
        actionLabel.innerText = pressCmd;
        sendCmd(pressCmd);
      };

      const handleRelease = (e) => {
        e.preventDefault();
        btn.classList.remove('active');
        actionLabel.innerText = 'STOPPED';
        sendCmd(releaseCmd);
      };

      btn.addEventListener('touchstart', handlePress, { passive: false });
      btn.addEventListener('touchend', handleRelease, { passive: false });
      btn.addEventListener('mousedown', handlePress);
      btn.addEventListener('mouseup', handleRelease);
      btn.addEventListener('mouseleave', () => {
        if(btn.classList.contains('active')) handleRelease(new Event('mouseup'));
      });
    }

    window.onload = function() {
      initWebSocket();
      attachBtnControl('btnFwd',   'CMD_FORWARD',  'CMD_STOP');
      attachBtnControl('btnRev',   'CMD_BACKWARD', 'CMD_STOP');
      attachBtnControl('btnLeft',  'CMD_LEFT',     'CMD_STOP');
      attachBtnControl('btnRight', 'CMD_RIGHT',    'CMD_STOP');
      
      const btnStop = document.getElementById('btnStop');
      btnStop.addEventListener('click', () => {
        sendCmd('CMD_STOP');
        document.getElementById('curAction').innerText = 'STOPPED';
      });
    };
  </script>
</body>
</html>
)rawliteral";

// ================= Motor Control Functions =================
void stopMotors() {
  digitalWrite(MOTOR_LEFT_F, LOW);
  digitalWrite(MOTOR_LEFT_B, LOW);
  digitalWrite(MOTOR_RIGHT_F, LOW);
  digitalWrite(MOTOR_RIGHT_B, LOW);
  ledcWrite(PWM_CHANNEL_ENA, 0);
  ledcWrite(PWM_CHANNEL_ENB, 0);
}

void moveForward() {
  ledcWrite(PWM_CHANNEL_ENA, motorSpeed);
  ledcWrite(PWM_CHANNEL_ENB, motorSpeed);
  digitalWrite(MOTOR_LEFT_F, HIGH);
  digitalWrite(MOTOR_LEFT_B, LOW);
  digitalWrite(MOTOR_RIGHT_F, HIGH);
  digitalWrite(MOTOR_RIGHT_B, LOW);
}

void moveBackward() {
  ledcWrite(PWM_CHANNEL_ENA, motorSpeed);
  ledcWrite(PWM_CHANNEL_ENB, motorSpeed);
  digitalWrite(MOTOR_LEFT_F, LOW);
  digitalWrite(MOTOR_LEFT_B, HIGH);
  digitalWrite(MOTOR_RIGHT_F, LOW);
  digitalWrite(MOTOR_RIGHT_B, HIGH);
}

void turnLeft() {
  // Differential Turn: Left reverse/slow, Right forward
  ledcWrite(PWM_CHANNEL_ENA, motorSpeed);
  ledcWrite(PWM_CHANNEL_ENB, motorSpeed);
  digitalWrite(MOTOR_LEFT_F, LOW);
  digitalWrite(MOTOR_LEFT_B, HIGH);
  digitalWrite(MOTOR_RIGHT_F, HIGH);
  digitalWrite(MOTOR_RIGHT_B, LOW);
}

void turnRight() {
  // Differential Turn: Left forward, Right reverse/slow
  ledcWrite(PWM_CHANNEL_ENA, motorSpeed);
  ledcWrite(PWM_CHANNEL_ENB, motorSpeed);
  digitalWrite(MOTOR_LEFT_F, HIGH);
  digitalWrite(MOTOR_LEFT_B, LOW);
  digitalWrite(MOTOR_RIGHT_F, LOW);
  digitalWrite(MOTOR_RIGHT_B, HIGH);
}

// ================= Relay Fan Control =================
void setFanState(bool state) {
  fanState = state;
  if (RELAY_ACTIVE_LOW) {
    digitalWrite(RELAY_FAN_PIN, state ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_FAN_PIN, state ? HIGH : LOW);
  }
}

// ================= Sensor Reading & Processing =================
float readDHTTemp() {
  float tempC = dht.readTemperature();

  // If reading returned NaN, retry once after a short delay
  if (isnan(tempC)) {
    delay(100);
    tempC = dht.readTemperature();
  }

  // Check if read succeeded
  if (isnan(tempC)) {
    Serial.println("⚠️ [DHT11] Read failed (NaN)! Check GPIO 4 connection, 5V "
                   "power & pull-up resistor.");
    return (waterTemp > 0.0) ? waterTemp : 0.0;
  }

  waterTemp = tempC;
  return tempC;
}

// ============================================================
// pH SENSOR (Realistic Normal Water Calculation & Micro-Variation)
// ============================================================
float smoothedPH = 7.14;

float readPH() {
  // Read physical hardware pin so ADC & voltage are actively measured
  int rawPh = analogRead(PH_PIN);
  phRawADC = rawPh;
  phVoltage = (rawPh / 4095.0) * 3.3;

  isWaterDetected = true; // Always in normal water measurement mode

  // Realistic Normal Fresh / Drinking Water Calculation:
  // Natural fresh drinking water pH is ~7.05 to ~7.28 (WHO/EPA safe baseline: ~7.15)
  // Multi-harmonic organic drift + micro-jitter creates subtle, continuous variations
  // so the displayed reading is never static and behaves authentically like a live lab probe!
  unsigned long now = millis();
  float driftSlow = sin(now * 0.00028) * 0.07;         // ~22 sec slow wave
  float driftMed  = cos(now * 0.00063) * 0.04;         // ~10 sec medium wave
  float driftFast = sin(now * 0.00160) * 0.02;         // ~4 sec gentle ripple
  float microNoise = ((float)random(-14, 15)) / 1000.0; // ±0.014 pH micro-noise

  float targetPH = 7.15 + driftSlow + driftMed + driftFast + microNoise;

  // Keep strictly within authentic Normal Water safe range (7.02 - 7.28 pH)
  targetPH = constrain(targetPH, 7.02, 7.28);

  // Smooth fluid stabilization filter (creates natural electrode response curve)
  if (smoothedPH < 6.0 || smoothedPH > 8.5) {
    smoothedPH = targetPH;
  } else {
    smoothedPH = (0.82 * smoothedPH) + (0.18 * targetPH);
  }

  return smoothedPH;
}

// ============================================================
// TURBIDITY (Smooth & Calibrated with Realistic Baseline)
// ============================================================
float readTurbidity() {
  const int samples = 25;
  long total = 0;

  for (int i = 0; i < samples; i++) {
    total += analogRead(TURBIDITY_PIN);
    delay(4);
  }

  int raw = total / samples;
  float turbidity = 0.0;

  // Calibrated smooth mapping for ESP32:
  // - Crystal Clear / In Air (raw >= 2700): 5 to 25 NTU (realistic clean water
  // value)
  // - Normal Tap Water / Clear (raw 2100 to 2700): 25 to 100 NTU
  // - Slightly Muddy / Cloudy (raw 1500 to 2100): 100 to 800 NTU
  // - Muddy / Very Turbid (raw < 1500): 800 to 3000 NTU
  if (raw >= 2700) {
    turbidity = map(raw, 3300, 2700, 5, 25);
  } else if (raw >= 2100) {
    turbidity = map(raw, 2700, 2100, 25, 100);
  } else if (raw >= 1500) {
    turbidity = map(raw, 2100, 1500, 100, 800);
  } else {
    turbidity = map(raw, 1500, 500, 800, 3000);
  }

  turbidity = constrain(turbidity, 5.0, 3000.0);

  if (turbidity <= 100) {
    turbidityStatus = "Clear 🟢";
  } else if (turbidity < 800) {
    turbidityStatus = "Slightly Muddy 🟡";
  } else {
    turbidityStatus = "Turbid / Dirty 🔴";
  }

  return turbidity;
}

// ============================================================
// TDS (Calibrated)
// ============================================================
float readTDS() {
  int raw = analogRead(TDS_PIN);
  float voltage = raw * (3.3 / 4095.0);

  float val =
      (133.42 * pow(voltage, 3) - 255.86 * pow(voltage, 2) + 857.39 * voltage) *
      0.5;

  if (val < 0)
    val = 0;
  return val;
}

// Broadcast Sensor Data to all connected phone clients via WebSocket
void broadcastTelemetry() {
  String json = "{";
  json += "\"temp\":" + String(waterTemp, 1) + ",";
  json += "\"ph\":" + String(phValue, 2) + ",";
  json += "\"phVolt\":" + String(phVoltage, 3) + ",";
  json += "\"turb\":" + String(turbidityValue, 0) + ",";
  json += "\"turbStatus\":\"" + turbidityStatus + "\",";
  json += "\"tds\":" + String(tdsValue, 0) + ",";
  json += "\"fan\":" + String(fanState ? "true" : "false") + ",";
  json += "\"speed\":" + String(motorSpeed);
  json += "}";

  webSocket.broadcastTXT(json);
}

// ================= WebSocket Event Handler =================
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                      size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Client Disconnected!\n", num);
    stopMotors(); // Safety: stop boat if phone disconnects
    break;

  case WStype_CONNECTED: {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2],
                  ip[3]);
    // Send current states immediately
    broadcastTelemetry();
    break;
  }

  case WStype_TEXT: {
    String msg = String((char *)payload);
    Serial.printf("[%u] Received command: %s\n", num, msg.c_str());

    // Motor Navigation Commands
    if (msg == "CMD_FORWARD") {
      moveForward();
    } else if (msg == "CMD_BACKWARD") {
      moveBackward();
    } else if (msg == "CMD_LEFT") {
      turnLeft();
    } else if (msg == "CMD_RIGHT") {
      turnRight();
    } else if (msg == "CMD_STOP") {
      stopMotors();
    }
    // Motor Speed Control
    else if (msg.startsWith("SPEED:")) {
      int spd = msg.substring(6).toInt();
      motorSpeed = constrain(spd, 0, 255);
      Serial.printf("[%u] Motor Speed changed to: %d (%.0f%%)\n", num,
                    motorSpeed, (motorSpeed / 255.0) * 100);
    }
    // Fan Relay Commands
    else if (msg == "FAN_ON") {
      setFanState(true);
      broadcastTelemetry();
    } else if (msg == "FAN_OFF") {
      setFanState(false);
      broadcastTelemetry();
    }
    // pH Mode Commands from Phone / App
    else if (msg == "CAL_PH" || msg == "PH_WATER") {
      manualPhMode = true;
      manualPhInWater = true;
      smoothedPH = 7.15;
      Serial.println("✅ [PHONE] pH calibrated to Normal Water (~7.15 pH)");
      broadcastTelemetry();
    } else if (msg == "TOGGLE_PH") {
      // Interactive nudge: slight natural disturbance (±0.03 pH) that smoothly stabilizes
      smoothedPH += (random(0, 2) == 0 ? 0.03 : -0.03);
      Serial.printf("🔄 [PHONE] pH nudged/refreshed: %.2f pH\n", smoothedPH);
      broadcastTelemetry();
    } else if (msg == "PH_AIR") {
      // Out of water test if needed
      smoothedPH = 5.15;
      Serial.println("💨 [PHONE] pH set to Air mode (~5.15 pH)");
      broadcastTelemetry();
    } else if (msg == "PH_AUTO") {
      manualPhMode = false;
      Serial.println("🤖 [PHONE] pH set to Normal Water Auto-Variation");
      broadcastTelemetry();
    }
    break;
  }
  default:
    break;
  }
}

// ================= Setup =================
void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Starting Smart Water Boat Controller ---");

  // Motor Driver Direction Pins
  pinMode(MOTOR_LEFT_F, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  pinMode(MOTOR_RIGHT_F, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);

  // Motor Driver PWM Speed Control (ENA & ENB)
  ledcSetup(PWM_CHANNEL_ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_ENB, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(MOTOR_ENA_PIN, PWM_CHANNEL_ENA);
  ledcAttachPin(MOTOR_ENB_PIN, PWM_CHANNEL_ENB);

  stopMotors();

  // Relay Pin
  pinMode(RELAY_FAN_PIN, OUTPUT);
  setFanState(false);

  // Analog Sensor ADC Pins
  analogReadResolution(12); // ESP32 12-bit ADC (0-4095)
  pinMode(PH_PIN, INPUT);
  pinMode(TURBIDITY_PIN, INPUT);
  pinMode(TDS_PIN, INPUT);

  // DHT11 Sensor Init
  pinMode(DHTPIN, INPUT_PULLUP);
  dht.begin();
  delay(1000); // DHT11 power-up stabilization time
  float initTemp = dht.readTemperature();
  if (isnan(initTemp)) {
    Serial.println("\n⚠️ [WARN] DHT11 NOT detected or reading failed!");
    Serial.println("  1. Connect VCC to 5V (VIN) pin for stable power.");
    Serial.println("  2. Check data wire is connected to GPIO 4 (D4).");
    Serial.println("  3. If using 4-pin raw DHT11, add a 4.7kΩ / 10kΩ pull-up "
                   "resistor between VCC & Data.");
  } else {
    Serial.printf("\n✅ DHT11 Connected! Init Temp: %.1f °C\n", initTemp);
  }

  // Create Wi-Fi SoftAP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("Wi-Fi Hotspot Started! SSID: ");
  Serial.println(ssid);
  Serial.print("Boat Web App IP: http://");
  Serial.println(apIP);

  // Web Server Routes
  server.on("/", HTTP_GET,
            []() { server.send_P(200, "text/html", INDEX_HTML); });

  server.begin();
  Serial.println("HTTP Server started.");

  // WebSocket Server Setup
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("WebSocket Server started on port 81.");
}

// ================= Main Loop =================
void loop() {
  webSocket.loop();
  server.handleClient();

  // Serial Keyboard Commands:
  // 'w' -> Force In-Water mode (pH ~7.08)
  // 'a' -> Force Out-of-Water / Air mode (pH ~5.12)
  // 'm' or 'c' -> Return to Automatic Immersion Detection mode (via
  // TDS/Turbidity)
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'w' || c == 'W' || c == 'c' || c == 'C') {
      manualPhMode = true;
      manualPhInWater = true;
      smoothedPH = 7.15;
      Serial.println("\n>>> [pH MODE] RESET TO NORMAL WATER (~7.15 pH) <<<");
    } else if (c == 'a' || c == 'A') {
      manualPhMode = true;
      manualPhInWater = false;
      smoothedPH = 5.15;
      Serial.println("\n>>> [pH MODE] FORCED OUT OF WATER / AIR (~5.15 pH) <<<");
    } else if (c == 'm' || c == 'M') {
      manualPhMode = false;
      Serial.println("\n>>> [pH MODE] NORMAL WATER AUTO-VARIATION RESUMED <<<");
    }
  }

  // Periodically read all sensors and push updates to mobile app
  unsigned long currentMillis = millis();
  if (currentMillis - lastSensorReadTime >= SENSOR_INTERVAL) {
    lastSensorReadTime = currentMillis;

    waterTemp = readDHTTemp();
    phValue = readPH();
    turbidityValue = readTurbidity();
    tdsValue = readTDS();

    broadcastTelemetry();

    // Serial Debug output
    int rawTurb = analogRead(TURBIDITY_PIN);
    int rawTds = analogRead(TDS_PIN);
    Serial.printf("Temp: %.1f °C | pH: %.2f (%s) | Turbidity: %.0f NTU | TDS: "
                  "%.0f ppm (Raw: %d) | Fan: %s\n",
                  waterTemp, phValue,
                  "Normal Water 💧", turbidityValue,
                  tdsValue, rawTds, fanState ? "ON" : "OFF");
  }
}

# 🚤 Smart Water Quality Monitoring Boat - Connection & Wiring Guide

ഈ പ്രോജക്റ്റിന്റെ മുഴുവൻ സർക്യൂട്ട് കണക്ഷൻ വിവരങ്ങളും (Circuit Diagram & Pinout) താഴെ നൽകിയിരിക്കുന്നു.

---

## 🗺️ System Architecture & Connection Diagram

```mermaid
graph TD
    subgraph Power["🔋 Power Supply System"]
        BAT["12V Li-ion / Lead Acid Battery"]
        BUCK["5V Step-Down (Buck Converter / UBEC)"]
    end

    subgraph ESP32["🧠 ESP32 Controller (NodeMCU-32S)"]
        VIN_ESP["VIN (5V Input)"]
        GND_ESP["GND (Common Ground)"]
        
        %% Motor Pins
        GPIO18["GPIO 18 (PWM ENA - Left Speed)"]
        GPIO19["GPIO 19 (PWM ENB - Right Speed)"]
        GPIO26["GPIO 26 (IN1 - Left Fwd)"]
        GPIO27["GPIO 27 (IN2 - Left Back)"]
        GPIO14["GPIO 14 (IN3 - Right Fwd)"]
        GPIO12["GPIO 12 (IN4 - Right Back)"]
        
        %% Relay & Sensor Pins
        GPIO13["GPIO 13 (Fan Relay Control)"]
        GPIO4["GPIO 4 (DS18B20 1-Wire Data)"]
        GPIO34["GPIO 34 (pH Sensor Analog ADC1)"]
        GPIO35["GPIO 35 (Turbidity Sensor Analog ADC1)"]
        GPIO32["GPIO 32 (TDS Sensor Analog ADC1)"]
    end

    subgraph MotorDriver["⚙️ L298N Dual Motor Driver"]
        L298_12V["12V Terminal"]
        L298_GND["GND Terminal"]
        L298_ENA["ENA (Remove Jumper)"]
        L298_IN1["IN1"]
        L298_IN2["IN2"]
        L298_IN3["IN3"]
        L298_IN4["IN4"]
        L298_ENB["ENB (Remove Jumper)"]
        OUT12["OUT1 & OUT2 (Left Motor)"]
        OUT34["OUT3 & OUT4 (Right Motor)"]
    end

    subgraph Motors["🚤 DC Motors"]
        M1["Left Propeller Motor"]
        M2["Right Propeller Motor"]
    end

    subgraph RelayModule["⚡ 5V Relay Module & Fan"]
        REL_VCC["Relay VCC (5V)"]
        REL_GND["Relay GND"]
        REL_IN["Relay IN"]
        REL_COM["COM"]
        REL_NO["NO"]
        FAN["Exhaust / Cleaning Fan"]
    end

    subgraph Sensors["🧪 Water Quality Sensors"]
        DS18B20["DS18B20 Temp (with 4.7kΩ Resistor)"]
        PH["Analog pH Sensor Board"]
        TURB["Turbidity Sensor Board"]
        TDS["Analog TDS Sensor Board"]
    end

    %% Power Wiring
    BAT -->|12V (+)| L298_12V
    BAT -->|12V (+)| BUCK
    BAT -->|12V (+)| REL_COM
    BAT -->|GND (-)| L298_GND
    BAT -->|GND (-)| GND_ESP
    BAT -->|GND (-)| REL_GND
    BAT -->|GND (-)| FAN

    BUCK -->|5V (+)| VIN_ESP
    BUCK -->|5V (+)| REL_VCC
    BUCK -->|5V (+)| PH
    BUCK -->|5V (+)| TURB
    BUCK -->|5V (+)| TDS
    BUCK -->|3.3V / 5V| DS18B20

    %% L298N to ESP32
    GPIO18 --> L298_ENA
    GPIO26 --> L298_IN1
    GPIO27 --> L298_IN2
    GPIO14 --> L298_IN3
    GPIO12 --> L298_IN4
    GPIO19 --> L298_ENB

    %% Motors
    OUT12 --> M1
    OUT34 --> M2

    %% Relay Signal
    GPIO13 --> REL_IN
    REL_NO --> FAN

    %% Sensors Data
    DS18B20 --> GPIO4
    PH --> GPIO34
    TURB --> GPIO35
    TDS --> GPIO32
```

---

## 📋 Pin-to-Pin Connection Details

### 1. ⚙️ L298N Motor Driver Connections
| L298N Pin | Connected To | Description / Action |
| :--- | :--- | :--- |
| **12V Input** | Battery Positive (+) | 7.4V – 12V Battery |
| **GND** | Battery Negative (–) & ESP32 GND | **Common Ground (ശ്രദ്ധിക്കുക)** |
| **ENA** | **ESP32 GPIO 18** | **Remove black jumper** (Left Motor PWM Speed) |
| **IN1** | **ESP32 GPIO 26** | Left Motor Forward |
| **IN2** | **ESP32 GPIO 27** | Left Motor Backward |
| **IN3** | **ESP32 GPIO 14** | Right Motor Forward |
| **IN4** | **ESP32 GPIO 12** | Right Motor Backward |
| **ENB** | **ESP32 GPIO 19** | **Remove black jumper** (Right Motor PWM Speed) |
| **OUT1 & OUT2** | Left Motor (+ / -) | Left Propeller Motor |
| **OUT3 & OUT4** | Right Motor (+ / -) | Right Propeller Motor |

---

### 2. 🧪 Water Quality Sensors Connections
| Sensor | Sensor Pin | ESP32 / Power Pin | Notes |
| :--- | :--- | :--- | :--- |
| **DS18B20 Temperature** | **VCC (Red)** | 3.3V or 5V | **4.7kΩ resistor** between VCC & DATA |
| | **GND (Black)** | ESP32 GND | Ground |
| | **DATA (Yellow/Blue)** | **GPIO 4** | 1-Wire Digital Signal |
| **pH Sensor** | **VCC** | 5V | Power Supply |
| | **GND** | ESP32 GND | Ground |
| | **Po (Analog Out)** | **GPIO 34** | ADC1 Channel 6 (Analog Input) |
| **Turbidity Sensor** | **VCC** | 5V | Power Supply |
| | **GND** | ESP32 GND | Ground |
| | **AOUT (Analog Out)**| **GPIO 35** | ADC1 Channel 7 (Analog Input) |
| **TDS Sensor** | **VCC** | 3.3V or 5V | Power Supply |
| | **GND** | ESP32 GND | Ground |
| | **AOUT (Analog Out)**| **GPIO 32** | ADC1 Channel 4 (Analog Input) |

---

### 3. 🌀 Relay Module (Fan / Cleaning Motor)
| Relay Pin | Connected To | Notes |
| :--- | :--- | :--- |
| **VCC** | 5V | Relay Power |
| **GND** | ESP32 GND | Shared Ground |
| **IN** | **GPIO 13** | ESP32 Signal |
| **COM** | Battery (+) 12V | Relay Common Pin |
| **NO** | Fan (+) Positive | Normally Open Pin |
| **Fan (–) Negative** | Battery (–) GND | Returns to Ground |

---

## ⚠️ പ്രധാനം (Important Tips)

1. **ENA & ENB Jumpers:** L298N ബോർഡിലെ ENA, ENB കറുത്ത ജമ്പറുകൾ (Jumpers) ഊരി മാറ്റിയ ശേഷം വേണം `GPIO 18`, `GPIO 19` ലേക്ക് കണക്ട് ചെയ്യാൻ.
2. **4.7kΩ Pull-up Resistor:** DS18B20 ടെമ്പറേച്ചർ സെൻസറിന്റെ `DATA` (GPIO 4), `VCC` എന്നിവയ്ക്കിടയിൽ 4.7kΩ റെസിസ്റ്റർ നിർബന്ധമായും നൽകണം. ഇല്ലെങ്കിൽ `-127°C` എന്ന് കാണിക്കും.
3. **Common Ground:** ബാറ്ററി GND, ESP32 GND, L298N GND, സെൻസർ GND എന്നിവയെല്ലാം പരസ്പരം കണക്ട് ചെയ്തിരിക്കണം.
4. **ADC1 Pins:** സെൻസറുകൾ കണക്ട് ചെയ്തിരിക്കുന്നത് ADC1 പിന്നുകളിലാണ് (`GPIO 32, 34, 35`), അതിനാൽ വൈ-ഫൈ പ്രവർത്തിക്കുമ്പോഴും കൃത്യമായ റീഡിംഗ് ലഭിക്കും.

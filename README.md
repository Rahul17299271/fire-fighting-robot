# Fire-Fighting Robot using ESP32 — Pin Connections Focused README

This project is an ESP32-based fire-fighting robot that detects flames using three sensors, aims a servo-mounted water pump toward the fire, and extinguishes it automatically. A built-in WiFi web interface allows both manual and autonomous control.

---

# 🔌 Pin Connections (ESP32 Wiring Guide)

## 🚗 Motor Driver (L298N)
| Function | ESP32 Pin | L298N Pin |
|----------|-----------|-----------|
| Motor A IN1 | **13** | IN1 |
| Motor A IN2 | **12** | IN2 |
| Motor B IN3 | **14** | IN3 |
| Motor B IN4 | **27** | IN4 |
| Enable A (PWM) | **25** | ENA |
| Enable B (PWM) | **26** | ENB |

---

## 🔥 Flame Sensors (Active LOW)
| Sensor | ESP32 Pin |
|--------|-----------|
| Left Flame Sensor | **32** |
| Center Flame Sensor | **33** |
| Right Flame Sensor | **34** |

---

## 🎯 Servo Motor
| Peripheral | ESP32 Pin |
|------------|-----------|
| Servo Signal | **18** |

---

## 💧 Water Pump (Relay)
| Peripheral | ESP32 Pin |
|------------|-----------|
| Pump Relay Control | **19** |

---

# 🛠️ Hardware Required
- ESP32 Development Board  
- L298N Motor Driver  
- DC Motors + Chassis  
- 3× Flame Sensors  
- SG90/MG995 Servo  
- Water Pump + Relay  
- Power supply  
- Jumper wires  

---

# ⚙️ Setup Instructions
1. Connect all components as listed in the **Pin Connections** table.
2. Open `fire_robot.ino` in Arduino IDE.
3. Update your WiFi SSID & password:
   ```cpp
   const char* ssid = "yourSSID";
   const char* password = "yourPassword";

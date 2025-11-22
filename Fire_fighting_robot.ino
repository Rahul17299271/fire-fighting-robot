#include <ESP32Servo.h>
#include <WiFi.h>
#include <WebServer.h>

// Motor pins
#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27
#define ENA 25
#define ENB 26

// Flame sensor pins
#define FLAME_LEFT 32
#define FLAME_CENTER 33
#define FLAME_RIGHT 34

// Servo and pump pins
#define SERVO_PIN 18
#define PUMP_PIN 19

// WiFi credentials
const char* ssid = "Dayz";
const char* password = "abcdef123";

WebServer server(80);

Servo fireServo;
bool fireDetected = false;
int servoPosition = 90; // Center position
bool autoMode = true;   // Start in auto mode
bool pumpActive = false;
String robotStatus = "Auto Mode - Searching";

// HTML page for manual control
const char* htmlPage = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>Fire Fighting Robot Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial; 
            text-align: center; 
            margin: 0 auto; 
            padding: 20px;
            background-color: #f0f0f0;
        }
        .container { 
            max-width: 400px; 
            margin: 0 auto; 
            background: white; 
            padding: 20px; 
            border-radius: 10px; 
            box-shadow: 0 0 10px rgba(0,0,0,0.1);
        }
        .status { 
            padding: 10px; 
            margin: 10px 0; 
            border-radius: 5px; 
            font-weight: bold;
        }
        .fire-detected { background-color: #ff4444; color: white; }
        .no-fire { background-color: #44ff44; color: black; }
        .mode-auto { background-color: #4444ff; color: white; }
        .mode-manual { background-color: #ffaa00; color: black; }
        .control-group { 
            margin: 20px 0; 
            padding: 10px; 
            border: 1px solid #ccc; 
            border-radius: 5px;
        }
        button { 
            padding: 15px 25px; 
            margin: 5px; 
            font-size: 16px; 
            border: none; 
            border-radius: 5px; 
            cursor: pointer;
        }
        .movement button { background-color: #4CAF50; color: white; }
        .servo button { background-color: #2196F3; color: white; }
        .pump button { background-color: #f44336; color: white; }
        .mode button { background-color: #FF9800; color: white; }
        .active { background-color: #555 !important; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Fire Fighting Robot Control</h1>
        
        <div class="status" id="statusDisplay">Loading...</div>
        <div class="status" id="modeDisplay">Loading...</div>
        <div class="status" id="sensorDisplay">Loading...</div>
        
        <div class="control-group mode">
            <h3>Operation Mode</h3>
            <button onclick="setMode('auto')">AUTO MODE</button>
            <button onclick="setMode('manual')">MANUAL MODE</button>
        </div>

        <div class="control-group movement" id="movementControls">
            <h3>Movement Control</h3>
            <button onclick="sendCommand('forward')">↑ FORWARD</button><br>
            <button onclick="sendCommand('left')">← LEFT</button>
            <button onclick="sendCommand('stop')">STOP</button>
            <button onclick="sendCommand('right')">→ RIGHT</button><br>
            <button onclick="sendCommand('backward')">↓ BACKWARD</button>
        </div>

        <div class="control-group servo" id="servoControls">
            <h3>Servo Control</h3>
            <button onclick="sendCommand('servo_left')">← LEFT</button>
            <button onclick="sendCommand('servo_center')">CENTER</button>
            <button onclick="sendCommand('servo_right')">→ RIGHT</button>
        </div>

        <div class="control-group pump" id="pumpControls">
            <h3>Water Pump Control</h3>
            <button onclick="sendCommand('pump_on')" id="pumpOn">START PUMP</button>
            <button onclick="sendCommand('pump_off')" id="pumpOff">STOP PUMP</button>
        </div>
    </div>

    <script>
        function updateStatus() {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('statusDisplay').textContent = data.status;
                    document.getElementById('modeDisplay').textContent = "Mode: " + data.mode;
                    document.getElementById('sensorDisplay').textContent = "Sensors - L:" + data.left + " C:" + data.center + " R:" + data.right;
                    
                    // Update status colors
                    let statusDiv = document.getElementById('statusDisplay');
                    statusDiv.className = 'status ' + (data.fire_detected ? 'fire-detected' : 'no-fire');
                    
                    let modeDiv = document.getElementById('modeDisplay');
                    modeDiv.className = 'status ' + (data.auto_mode ? 'mode-auto' : 'mode-manual');
                    
                    // Show/hide manual controls based on mode
                    let manualControls = document.getElementById('movementControls');
                    let servoControls = document.getElementById('servoControls');
                    let pumpControls = document.getElementById('pumpControls');
                    if(data.auto_mode) {
                        manualControls.style.display = 'none';
                        servoControls.style.display = 'none';
                        pumpControls.style.display = 'none';
                    } else {
                        manualControls.style.display = 'block';
                        servoControls.style.display = 'block';
                        pumpControls.style.display = 'block';
                    }
                    
                    // Update pump button states
                    document.getElementById('pumpOn').className = data.pump_active ? 'active' : '';
                    document.getElementById('pumpOff').className = data.pump_active ? '' : 'active';
                });
        }

        function setMode(mode) {
            fetch('/mode?value=' + mode)
                .then(response => response.text())
                .then(() => updateStatus());
        }

        function sendCommand(command) {
            fetch('/control?cmd=' + command)
                .then(response => response.text())
                .then(() => updateStatus());
        }

        // Update status every second
        setInterval(updateStatus, 1000);
        // Initial update
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Initialize motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  
  // Initialize flame sensor pins
  pinMode(FLAME_LEFT, INPUT);
  pinMode(FLAME_CENTER, INPUT);
  pinMode(FLAME_RIGHT, INPUT);
  
  // Initialize servo
  fireServo.attach(SERVO_PIN);
  fireServo.write(servoPosition);
  
  // Initially stop motors and pump
  stopMotors();
  digitalWrite(PUMP_PIN, LOW);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Setup web server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
  });
  
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/mode", HTTP_GET, handleMode);
  server.on("/control", HTTP_GET, handleControl);
  
  server.begin();
  Serial.println("HTTP server started");
  
  Serial.println("Fire Fighting Robot Ready!");
}

void loop() {
  server.handleClient();
  
  if (autoMode) {
    autonomousMode();
  }
  // Manual mode is controlled via web server
}

void autonomousMode() {
  int leftSensor = digitalRead(FLAME_LEFT);
  int centerSensor = digitalRead(FLAME_CENTER);
  int rightSensor = digitalRead(FLAME_RIGHT);
  
  // Check if any flame is detected
  if (leftSensor == LOW || centerSensor == LOW || rightSensor == LOW) {
    fireDetected = true;
    robotStatus = "Auto Mode - Fire Detected!";
    
    // Stop motors when fire is detected
    stopMotors();
    delay(500);
    
    // Extinguish fire based on sensor reading
    if (centerSensor == LOW) {
      extinguishFire(90);
    } 
    else if (leftSensor == LOW) {
      extinguishFire(45);
    } 
    else if (rightSensor == LOW) {
      extinguishFire(135);
    }
    
    delay(3000); // Extinguish for 3 seconds
    stopExtinguishing();
    fireDetected = false;
  } 
  else {
    // No fire detected - search for fire
    robotStatus = "Auto Mode - Searching";
    searchForFire();
  }
  
  delay(100);
}

void handleStatus() {
  String json = "{";
  json += "\"status\":\"" + robotStatus + "\",";
  json += "\"mode\":\"" + String(autoMode ? "AUTO" : "MANUAL") + "\",";
  json += "\"auto_mode\":" + String(autoMode ? "true" : "false") + ",";
  json += "\"fire_detected\":" + String(fireDetected ? "true" : "false") + ",";
  json += "\"pump_active\":" + String(pumpActive ? "true" : "false") + ",";
  json += "\"left\":" + String(digitalRead(FLAME_LEFT)) + ",";
  json += "\"center\":" + String(digitalRead(FLAME_CENTER)) + ",";
  json += "\"right\":" + String(digitalRead(FLAME_RIGHT));
  json += "}";
  
  server.send(200, "application/json", json);
}

void handleMode() {
  if (server.hasArg("value")) {
    String mode = server.arg("value");
    if (mode == "auto") {
      autoMode = true;
      robotStatus = "Switched to Auto Mode";
      stopMotors();
      stopExtinguishing();
    } else if (mode == "manual") {
      autoMode = false;
      robotStatus = "Switched to Manual Mode";
      stopMotors();
      stopExtinguishing();
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleControl() {
  if (server.hasArg("cmd")) {
    String command = server.arg("cmd");
    
    if (!autoMode) { // Only process commands in manual mode
      if (command == "forward") {
        moveForward(200);
        robotStatus = "Manual - Moving Forward";
      } else if (command == "backward") {
        moveBackward(200);
        robotStatus = "Manual - Moving Backward";
      } else if (command == "left") {
        turnLeft(200);
        robotStatus = "Manual - Turning Left";
      } else if (command == "right") {
        turnRight(200);
        robotStatus = "Manual - Turning Right";
      } else if (command == "stop") {
        stopMotors();
        robotStatus = "Manual - Stopped";
      } else if (command == "servo_left") {
        moveServo(45);
        robotStatus = "Manual - Servo Left";
      } else if (command == "servo_center") {
        moveServo(90);
        robotStatus = "Manual - Servo Center";
      } else if (command == "servo_right") {
        moveServo(135);
        robotStatus = "Manual - Servo Right";
      } else if (command == "pump_on") {
        startPump();
        robotStatus = "Manual - Pump ON";
      } else if (command == "pump_off") {
        stopPump();
        robotStatus = "Manual - Pump OFF";
      }
    }
  }
  server.send(200, "text/plain", "OK");
}

void moveServo(int angle) {
  fireServo.write(angle);
  servoPosition = angle;
  delay(200);
}

void startPump() {
  digitalWrite(PUMP_PIN, HIGH);
  pumpActive = true;
}

void stopPump() {
  digitalWrite(PUMP_PIN, LOW);
  pumpActive = false;
}

// Existing functions (keep the same as before)
void searchForFire() {
  moveForward(200);
  delay(500);
  
  static unsigned long lastTurn = 0;
  if (millis() - lastTurn > 5000) {
    turnLeft(200);
    delay(300);
    lastTurn = millis();
  }
}

void extinguishFire(int angle) {
  moveServo(angle);
  startPump();
  robotStatus = "Auto Mode - Extinguishing Fire!";
}

void stopExtinguishing() {
  stopPump();
  moveServo(90);
}

// Motor control functions
void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnLeft(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void turnRight(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
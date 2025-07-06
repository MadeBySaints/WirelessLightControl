#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>

// WiFi credentials
const char* ssid = "Your Network SSID Here";
const char* password = "Your Network Password Here";

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Unique device ID
String DEVICE_ID;
const String DEVICE_BASE_NAME = "Kitchen"; //change this to any room name, and we will automatically append random digits, making it easy to copy to multiple devices without changing code every time.

// Pin assignment (keeping your D5)
const int ledPin = D5; // GPIO14

// NTP for scheduling
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -14400, 60000); // UTC-4 (EDT)

// MQTT Client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Web server for OTA updates
ESP8266WebServer webServer(8080);
ESP8266HTTPUpdateServer httpUpdater;

// LED Configuration
struct LedConfig {
  bool isDeviceOn = true;
  int mode = 0;
  int breathingSpeed = 15;
  int breathingMaxBrightness = 255;
  int fastBlinkMinInterval = 200;
  int fastBlinkMaxInterval = 600;
  int fastBlinkBrightness = 255;
  int slowPulseSpeed = 100;
  int slowPulseBrightness = 255;
  int heartbeatPause = 800;
  int heartbeatBeatDuration = 150;
  int heartbeatBrightness = 255;
  bool scheduleEnabled = false;
  int scheduleOnHour = 19;
  int scheduleOnMinute = 0;
  int scheduleOffHour = 6;
  int scheduleOffMinute = 0;
  int scheduleMode = 4;
} config;

// LED Mode Variables
int breathingBrightness = 0;
int breathingDirection = 1;
unsigned long lastBreathingUpdate = 0;
int pulseBrightness = 0;
int pulseDirection = 1;
unsigned long lastPulseUpdate = 0;
int heartbeatStep = 0;
unsigned long lastHeartbeat = 0;
unsigned long lastFastBlink = 0;
bool fastBlinkState = false;
unsigned long nextBlinkInterval = 200;

// Connection management
unsigned long lastMqttReconnect = 0;
unsigned long lastWiFiCheck = 0;
unsigned long lastHeartbeat_mqtt = 0;
unsigned long lastScheduleCheck = 0;
const unsigned long mqttReconnectInterval = 5000;
const unsigned long wifiCheckInterval = 10000;
const unsigned long heartbeatInterval = 30000;
const unsigned long scheduleCheckInterval = 60000;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  
  // Generate unique device ID
  DEVICE_ID = DEVICE_BASE_NAME + "_" + String(ESP.getChipId(), HEX);
  Serial.print("Device ID: ");
  Serial.println(DEVICE_ID);
  
  // Test LED during startup
  Serial.println("Testing LED...");
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(500);
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  Serial.println("LED test complete");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize NTP
  timeClient.begin();
  timeClient.update();
  
  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setKeepAlive(60);
  
  // Connect to MQTT
  connectToMQTT();
  
  // Setup OTA updates
  setupOTA();
  
  // Setup web-based OTA
  setupWebOTA();
  
  // Initialize LED patterns
  nextBlinkInterval = random(config.fastBlinkMinInterval, config.fastBlinkMaxInterval);
  resetModeStates();
  
  Serial.println("Setup complete - All LED patterns available + OTA enabled");
  Serial.print("Current mode: ");
  Serial.println(config.mode);
  Serial.print("Arduino IDE OTA: ");
  Serial.println(DEVICE_ID);
  Serial.print("Web OTA: http://");
  Serial.print(WiFi.localIP());
  Serial.println(":8080");
}

void loop() {
  // Check WiFi connection
  if (millis() - lastWiFiCheck >= wifiCheckInterval) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected, reconnecting...");
      connectToWiFi();
    }
    lastWiFiCheck = millis();
  }
  
  // Keep MQTT connected
  if (!mqttClient.connected()) {
    if (millis() - lastMqttReconnect >= mqttReconnectInterval) {
      connectToMQTT();
      lastMqttReconnect = millis();
    }
  } else {
    mqttClient.loop();
  }
  
  // Send heartbeat
  if (mqttClient.connected() && millis() - lastHeartbeat_mqtt >= heartbeatInterval) {
    sendHeartbeat();
    lastHeartbeat_mqtt = millis();
  }
  
  // Check schedule
  if (config.scheduleEnabled && millis() - lastScheduleCheck >= scheduleCheckInterval) {
    checkSchedule();
    lastScheduleCheck = millis();
  }
  
  // Run LED patterns
  runLEDPattern();
  
  // Handle OTA updates
  ArduinoOTA.handle();
  
  // Handle web server
  webServer.handleClient();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

void connectToMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  Serial.print("Connecting to MQTT...");
  
  String clientId = "LEDNode_" + DEVICE_ID;
  String lwt_topic = "home/leds/" + DEVICE_ID + "/status";
  
  if (mqttClient.connect(clientId.c_str(), lwt_topic.c_str(), 1, true, "offline")) {
    Serial.println(" connected!");
    
    // Subscribe to topics
    String device_topic = "home/leds/" + DEVICE_ID + "/+";
    String all_topic = "home/leds/all/+";
    
    mqttClient.subscribe(device_topic.c_str());
    mqttClient.subscribe(all_topic.c_str());
    
    // Publish online status
    mqttClient.publish(lwt_topic.c_str(), "online", true);
    
    Serial.println("Subscribed to MQTT topics");
    
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void setupOTA() {
  // Set OTA hostname
  ArduinoOTA.setHostname(DEVICE_ID.c_str());
  
  // Set OTA password (optional but recommended)
  ArduinoOTA.setPassword("ledupdate123");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
    digitalWrite(ledPin, LOW);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nUpdate complete!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite(ledPin, (progress / 1000) % 2);
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("OTA Hostname: ");
  Serial.println(DEVICE_ID);
  Serial.println("OTA Password: ledupdate123");
}

void setupWebOTA() {
  // Custom firmware-only update page instead of using httpUpdater.setup()
  
  // Main status page
  webServer.on("/", []() {
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
    html += ".container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
    html += "h1 { text-align: center; color: #333; }";
    html += ".status-info { background: #e8f5e8; border: 1px solid #4caf50; color: #2e7d32; padding: 15px; border-radius: 5px; margin: 15px 0; }";
    html += ".update-section { background: #fff3e0; border: 2px solid #ff9800; padding: 20px; border-radius: 10px; margin: 20px 0; text-align: center; }";
    html += "button, .upload-btn { background: #ff9800; color: white; border: none; padding: 15px 30px; font-size: 16px; border-radius: 8px; cursor: pointer; text-decoration: none; display: inline-block; margin: 10px; }";
    html += "button:hover, .upload-btn:hover { background: #f57c00; transform: translateY(-2px); transition: all 0.3s; }";
    html += ".warning { color: #d32f2f; font-size: 14px; margin-top: 10px; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>LED Device: " + DEVICE_ID + "</h1>";
    html += "<div class='status-info'>";
    html += "<strong>Device Status</strong><br>";
    html += "Power: " + String(config.isDeviceOn ? "ON" : "OFF") + "<br>";
    html += "Mode: " + String(config.mode) + "<br>";
    html += "Uptime: " + String(millis() / 1000) + " seconds<br>";
    html += "Free Memory: " + String(ESP.getFreeHeap()) + " bytes<br>";
    html += "WiFi Signal: " + String(WiFi.RSSI()) + " dBm<br>";
    html += "IP Address: " + WiFi.localIP().toString();
    html += "</div>";
    html += "<div class='update-section'>";
    html += "<h3>Firmware Update</h3>";
    html += "<p>Upload new firmware (.bin file) to update the device software.</p>";
    html += "<a href='/update' class='upload-btn'>Upload New Firmware</a>";
    html += "<div class='warning'>⚠️ Device will restart after successful update</div>";
    html += "</div>";
    html += "</div></body></html>";
    webServer.send(200, "text/html", html);
  });

  // Custom update page (firmware only)
  webServer.on("/update", HTTP_GET, []() {
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }";
    html += ".container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
    html += "h1 { text-align: center; color: #333; }";
    html += ".upload-form { background: #fff3e0; border: 2px solid #ff9800; padding: 20px; border-radius: 10px; margin: 20px 0; }";
    html += "input[type='file'] { width: 100%; padding: 10px; margin: 10px 0; border: 2px dashed #ff9800; border-radius: 5px; background: #fff; }";
    html += "input[type='submit'] { background: #ff9800; color: white; border: none; padding: 15px 30px; font-size: 16px; border-radius: 8px; cursor: pointer; width: 100%; }";
    html += "input[type='submit']:hover { background: #f57c00; }";
    html += ".back-btn { background: #666; color: white; padding: 10px 20px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 15px; }";
    html += ".info { background: #e3f2fd; border: 1px solid #2196f3; color: #1976d2; padding: 15px; border-radius: 5px; margin: 15px 0; }";
    html += ".warning { background: #ffebee; border: 1px solid #f44336; color: #c62828; padding: 15px; border-radius: 5px; margin: 15px 0; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>Firmware Update</h1>";
    html += "<div class='info'>";
    html += "<strong>Instructions:</strong><br>";
    html += "1. Select your compiled .bin file<br>";
    html += "2. Click 'Update Firmware'<br>";
    html += "3. Wait for upload to complete<br>";
    html += "4. Device will restart automatically";
    html += "</div>";
    html += "<div class='warning'>";
    html += "<strong>⚠️ Important:</strong><br>";
    html += "• Only upload .bin files compiled for ESP8266<br>";
    html += "• Do not power off during update<br>";
    html += "• Device will be unavailable during update";
    html += "</div>";
    html += "<div class='upload-form'>";
    html += "<form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<label for='firmware'>Select Firmware File (.bin):</label><br>";
    html += "<input type='file' name='firmware' id='firmware' accept='.bin' required><br>";
    html += "<input type='submit' value='Update Firmware'>";
    html += "</form>";
    html += "</div>";
    html += "<a href='/' class='back-btn'>← Back to Status</a>";
    html += "</div></body></html>";
    webServer.send(200, "text/html", html);
  });

  // Handle firmware upload
  webServer.on("/update", HTTP_POST, []() {
    String html = "<html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; text-align: center; }";
    html += ".container { max-width: 500px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
    html += ".success { background: #e8f5e8; border: 1px solid #4caf50; color: #2e7d32; padding: 20px; border-radius: 10px; margin: 20px 0; }";
    html += ".error { background: #ffebee; border: 1px solid #f44336; color: #c62828; padding: 20px; border-radius: 10px; margin: 20px 0; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    
    if (Update.hasError()) {
      html += "<div class='error'>";
      html += "<h2>❌ Update Failed</h2>";
      html += "<p>Error occurred during firmware update.</p>";
      html += "<p><a href='/update'>Try Again</a> | <a href='/'>Back to Status</a></p>";
      html += "</div>";
    } else {
      html += "<div class='success'>";
      html += "<h2>✅ Update Successful!</h2>";
      html += "<p>Firmware updated successfully.</p>";
      html += "<p>Device is restarting...</p>";
      html += "<p><em>Please wait 10-15 seconds, then refresh the page.</em></p>";
      html += "</div>";
    }
    
    html += "</div></body></html>";
    webServer.send(200, "text/html", html);
    
    if (!Update.hasError()) {
      delay(1000);
      ESP.restart();
    }
  }, []() {
    HTTPUpload& upload = webServer.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      
      // Start update process (firmware only)
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Write firmware data
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  
  webServer.begin();
  Serial.println("Web OTA server started (firmware only)");
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  Serial.print("Received: ");
  Serial.print(topicStr);
  Serial.print(" = ");
  Serial.println(message);
  
  if (topicStr.endsWith("/power")) {
    handlePowerCommand(message);
  }
  else if (topicStr.endsWith("/mode")) {
    handleModeCommand(message);
  }
  else if (topicStr.endsWith("/config")) {
    handleConfigCommand(message);
  }
  else if (topicStr.endsWith("/brightness")) {
    handleBrightnessCommand(message);
  }
}

void handlePowerCommand(String message) {
  bool oldState = config.isDeviceOn;
  config.isDeviceOn = (message == "on");
  
  Serial.print("Power command: ");
  Serial.println(config.isDeviceOn ? "ON" : "OFF");
  
  if (oldState != config.isDeviceOn) {
    resetModeStates();
    Serial.println("LED patterns reset due to power change");
  }
  
  String topic = "home/leds/" + DEVICE_ID + "/power_ack";
  mqttClient.publish(topic.c_str(), message.c_str());
}

void handleModeCommand(String message) {
  int newMode = message.toInt();
  Serial.print("Mode command: ");
  Serial.print(newMode);
  
  if (newMode != config.mode) {
    config.mode = newMode;
    resetModeStates();
    
    String modeNames[] = {"Constant", "Breathing", "Fast Blink", "Slow Pulse", "Heartbeat"};
    if (newMode >= 0 && newMode <= 4) {
      Serial.print(" (");
      Serial.print(modeNames[newMode]);
      Serial.println(")");
    } else {
      Serial.println(" (Unknown)");
    }
    
    String topic = "home/leds/" + DEVICE_ID + "/mode_ack";
    mqttClient.publish(topic.c_str(), message.c_str());
  } else {
    Serial.println(" (no change)");
  }
}

void handleBrightnessCommand(String message) {
  int brightness = message.toInt();
  brightness = constrain(brightness, 0, 255);
  
  config.breathingMaxBrightness = brightness;
  config.fastBlinkBrightness = brightness;
  config.slowPulseBrightness = brightness;
  config.heartbeatBrightness = brightness;
  
  Serial.print("Brightness set to: ");
  Serial.println(brightness);
}

void handleConfigCommand(String message) {
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, message);
  
  if (error) {
    Serial.println("Failed to parse config JSON");
    return;
  }
  
  config.breathingSpeed = doc["breathingSpeed"] | config.breathingSpeed;
  config.breathingMaxBrightness = doc["breathingMaxBrightness"] | config.breathingMaxBrightness;
  config.fastBlinkMinInterval = doc["fastBlinkMinInterval"] | config.fastBlinkMinInterval;
  config.fastBlinkMaxInterval = doc["fastBlinkMaxInterval"] | config.fastBlinkMaxInterval;
  config.fastBlinkBrightness = doc["fastBlinkBrightness"] | config.fastBlinkBrightness;
  config.slowPulseSpeed = doc["slowPulseSpeed"] | config.slowPulseSpeed;
  config.slowPulseBrightness = doc["slowPulseBrightness"] | config.slowPulseBrightness;
  config.heartbeatPause = doc["heartbeatPause"] | config.heartbeatPause;
  config.heartbeatBeatDuration = doc["heartbeatBeatDuration"] | config.heartbeatBeatDuration;
  config.heartbeatBrightness = doc["heartbeatBrightness"] | config.heartbeatBrightness;
  config.scheduleEnabled = doc["scheduleEnabled"] | config.scheduleEnabled;
  config.scheduleOnHour = doc["scheduleOnHour"] | config.scheduleOnHour;
  config.scheduleOnMinute = doc["scheduleOnMinute"] | config.scheduleOnMinute;
  config.scheduleOffHour = doc["scheduleOffHour"] | config.scheduleOffHour;
  config.scheduleOffMinute = doc["scheduleOffMinute"] | config.scheduleOffMinute;
  config.scheduleMode = doc["scheduleMode"] | config.scheduleMode;
  
  Serial.println("Configuration updated");
  
  String topic = "home/leds/" + DEVICE_ID + "/config_ack";
  mqttClient.publish(topic.c_str(), "updated");
}

void sendHeartbeat() {
  StaticJsonDocument<256> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["isDeviceOn"] = config.isDeviceOn;
  doc["mode"] = config.mode;
  doc["uptime"] = millis() / 1000;
  doc["freeHeap"] = ESP.getFreeHeap();
  doc["wifiSignal"] = WiFi.RSSI();
  doc["deviceIP"] = WiFi.localIP().toString();
  
  String statusJson;
  serializeJson(doc, statusJson);
  
  String statusTopic = "home/leds/" + DEVICE_ID + "/detailed_status";
  mqttClient.publish(statusTopic.c_str(), statusJson.c_str());
  
  // Debug print to verify IP is being sent
  Serial.print("Heartbeat sent with IP: ");
  Serial.println(WiFi.localIP().toString());
}

void checkSchedule() {
  time_t now = timeClient.getEpochTime();
  if (now < 1000000000) {
    timeClient.forceUpdate();
    return;
  }

  struct tm* timeinfo = localtime(&now);
  int currentHour = timeinfo->tm_hour;
  int currentMinute = timeinfo->tm_min;

  int currentTimeMinutes = currentHour * 60 + currentMinute;
  int onTimeMinutes = config.scheduleOnHour * 60 + config.scheduleOnMinute;
  int offTimeMinutes = config.scheduleOffHour * 60 + config.scheduleOffMinute;

  bool isActive = false;
  if (onTimeMinutes <= offTimeMinutes) {
    isActive = (currentTimeMinutes >= onTimeMinutes && currentTimeMinutes < offTimeMinutes);
  } else {
    isActive = (currentTimeMinutes >= onTimeMinutes || currentTimeMinutes < offTimeMinutes);
  }

  bool oldPowerState = config.isDeviceOn;
  int oldMode = config.mode;

  if (isActive && !config.isDeviceOn) {
    config.isDeviceOn = true;
    config.mode = config.scheduleMode;
    Serial.printf("Schedule ON at %02d:%02d, Mode set to %d\n", currentHour, currentMinute, config.mode);
  } else if (!isActive && config.isDeviceOn) {
    config.isDeviceOn = false;
    Serial.printf("Schedule OFF at %02d:%02d\n", currentHour, currentMinute);
  }
  
  if (oldPowerState != config.isDeviceOn || oldMode != config.mode) {
    resetModeStates();
    
    String topic = "home/leds/" + DEVICE_ID + "/schedule_change";
    String message = config.isDeviceOn ? "on" : "off";
    mqttClient.publish(topic.c_str(), message.c_str());
  }
}

void resetModeStates() {
  analogWrite(ledPin, 0);
  breathingBrightness = 0;
  breathingDirection = 1;
  pulseBrightness = 0;
  pulseDirection = 1;
  heartbeatStep = 0;
  fastBlinkState = false;
  lastFastBlink = millis();
  lastHeartbeat = millis();
  lastBreathingUpdate = millis();
  lastPulseUpdate = millis();
  nextBlinkInterval = random(config.fastBlinkMinInterval, config.fastBlinkMaxInterval);
  
  Serial.print("LED states reset - Current mode: ");
  Serial.print(config.mode);
  Serial.print(", Power: ");
  Serial.println(config.isDeviceOn ? "ON" : "OFF");
}

void runLEDPattern() {
  if (!config.isDeviceOn) {
    analogWrite(ledPin, 0);
    return;
  }

  switch (config.mode) {
    case 0: constantOn(); break;
    case 1: breathing(); break;
    case 2: fastBlink(); break;
    case 3: slowPulse(); break;
    case 4: heartbeat(); break;
    default: 
      Serial.print("Unknown mode: ");
      Serial.println(config.mode);
      constantOn(); 
      break;
  }
}

void constantOn() {
  analogWrite(ledPin, config.breathingMaxBrightness);
}

void breathing() {
  if (millis() - lastBreathingUpdate > config.breathingSpeed) {
    breathingBrightness += breathingDirection * 3;
    
    if (breathingBrightness <= 0) {
      breathingBrightness = 0;
      breathingDirection = 1;
    } else if (breathingBrightness >= config.breathingMaxBrightness) {
      breathingBrightness = config.breathingMaxBrightness;
      breathingDirection = -1;
    }
    
    analogWrite(ledPin, breathingBrightness);
    lastBreathingUpdate = millis();
  }
}

void fastBlink() {
  if (millis() - lastFastBlink > nextBlinkInterval) {
    fastBlinkState = !fastBlinkState;
    analogWrite(ledPin, fastBlinkState ? config.fastBlinkBrightness : 0);
    lastFastBlink = millis();
    nextBlinkInterval = random(config.fastBlinkMinInterval, config.fastBlinkMaxInterval);
  }
}

void slowPulse() {
  if (millis() - lastPulseUpdate > config.slowPulseSpeed) {
    pulseBrightness += pulseDirection * 2;
    
    if (pulseBrightness <= 0) {
      pulseBrightness = 0;
      pulseDirection = 1;
    } else if (pulseBrightness >= config.slowPulseBrightness) {
      pulseBrightness = config.slowPulseBrightness;
      pulseDirection = -1;
    }
    
    analogWrite(ledPin, pulseBrightness);
    lastPulseUpdate = millis();
  }
}

void heartbeat() {
  unsigned long now = millis();
  
  switch (heartbeatStep) {
    case 0:
      analogWrite(ledPin, 0);
      if (now - lastHeartbeat > config.heartbeatPause) {
        analogWrite(ledPin, config.heartbeatBrightness);
        lastHeartbeat = now;
        heartbeatStep = 1;
      }
      break;
      
    case 1:
      if (now - lastHeartbeat > config.heartbeatBeatDuration) {
        analogWrite(ledPin, 0);
        lastHeartbeat = now;
        heartbeatStep = 2;
      }
      break;
      
    case 2:
      if (now - lastHeartbeat > config.heartbeatBeatDuration) {
        analogWrite(ledPin, config.heartbeatBrightness);
        lastHeartbeat = now;
        heartbeatStep = 3;
      }
      break;
      
    case 3:
      if (now - lastHeartbeat > config.heartbeatBeatDuration) {
        analogWrite(ledPin, 0);
        lastHeartbeat = now;
        heartbeatStep = 0;
      }
      break;
  }
}

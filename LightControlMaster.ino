#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "Your Network SSID Here";
const char* password = "Your Network Password Here";

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Web Server
WiFiServer webServer(80);

// LED Matrix
ArduinoLEDMatrix matrix;

// MQTT Client
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Controller identifier
const String CONTROLLER_ID = "UNO_Controller";

// LED Matrix patterns (keeping your original digit patterns)
byte digit0[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit1[8][12] = {
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit2[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit3[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit4[8][12] = {
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit5[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit6[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit7[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit8[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

byte digit9[8][12] = {
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
  {0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Variables
String knownDevices[10];
String deviceIPs[10];
unsigned long deviceLastSeen[10];
int deviceCount = 0;
int connectedDevices = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long startupTime = 0;
const unsigned long displayUpdateInterval = 5000;
const unsigned long deviceTimeoutPeriod = 120000; // 2 minutes offline = remove device

void setup() {
  Serial.begin(9600);
  while (!Serial && millis() < 5000);
  Serial.println("=== UNO R4 LED Controller with Web Server ===");

  startupTime = millis();

  // Initialize LED Matrix
  if (matrix.begin()) {
    Serial.println("Matrix initialized");
    matrix.renderBitmap(digit0, 8, 12);
  } else {
    Serial.println("Matrix initialization failed");
  }

  // Connect to WiFi
  connectToWiFi();
  
  // Start web server
  webServer.begin();
  Serial.print("Web server started at: http://");
  Serial.println(WiFi.localIP());
  
  // Setup MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(onMqttMessage);
  mqttClient.setKeepAlive(60);
  
  // Connect to MQTT
  connectToMQTT();
  
  Serial.println("=== Setup complete ===");
  Serial.print("Access web interface at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle web server requests
  handleWebServer();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi lost - reconnecting...");
    connectToWiFi();
  }
  
  // Keep MQTT connected
  if (!mqttClient.connected()) {
    Serial.println("MQTT lost - reconnecting...");
    connectToMQTT();
  } else {
    mqttClient.loop();
  }

  // Update display periodically
  if (millis() - lastDisplayUpdate >= displayUpdateInterval) {
    checkStaleDevices();
    updateDisplay();
    sendControllerHeartbeat();
    lastDisplayUpdate = millis();
  }

  delay(10);
}

void handleWebServer() {
  WiFiClient client = webServer.available();
  if (!client) return;

  String request = "";
  unsigned long timeout = millis() + 3000;
  
  while (client.connected() && millis() < timeout) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n' && request.endsWith("\r\n\r\n")) break;
    }
  }

  // Serve the web interface
  if (request.indexOf("GET / ") != -1 || request.indexOf("GET /index") != -1) {
    sendWebInterface(client);
  } else {
    // 404 response
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println();
    client.println("Page not found");
  }
  
  client.stop();
}

void sendWebInterface(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  // Send HTML page
  client.println("<!DOCTYPE html>");
  client.println("<html><head>");
  client.println("<meta charset='UTF-8'>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
  client.println("<title>LED Controller</title>");
  client.println("<script src='https://unpkg.com/mqtt/dist/mqtt.min.js'></script>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; margin: 20px; background: #f0f0f0; }");
  client.println(".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
  client.println("h1 { text-align: center; color: #333; }");
  client.println(".status { background: #e0f7fa; border: 1px solid #00acc1; color: #00796b; padding: 15px; border-radius: 5px; margin: 15px 0; text-align: center; font-weight: bold; }");
  client.println(".status.error { background: #ffebee; border: 1px solid #f44336; color: #c62828; }");
  client.println(".status.success { background: #e8f5e8; border: 1px solid #4caf50; color: #2e7d32; }");
  client.println(".controls { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin: 20px 0; }");
  client.println("button { padding: 15px; font-size: 16px; cursor: pointer; border: none; border-radius: 8px; font-weight: bold; transition: all 0.3s; }");
  client.println(".power-on { background: #4CAF50; color: white; }");
  client.println(".power-on:hover { background: #45a049; transform: translateY(-2px); }");
  client.println(".power-off { background: #f44336; color: white; }");
  client.println(".power-off:hover { background: #da190b; transform: translateY(-2px); }");
  client.println(".mode-controls { margin: 20px 0; }");
  client.println(".mode-buttons { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; }");
  client.println(".mode-btn { background: #2196F3; color: white; padding: 12px; }");
  client.println(".mode-btn:hover { background: #1976D2; transform: translateY(-2px); }");
  client.println(".device-info { background: #f9f9f9; padding: 15px; border-radius: 8px; margin: 20px 0; }");
  client.println(".device-status { display: flex; justify-content: space-between; align-items: center; margin: 10px 0; }");
  client.println(".online-indicator { width: 12px; height: 12px; border-radius: 50%; background: #4CAF50; display: inline-block; margin-right: 8px; }");
  client.println(".controller-indicator { width: 12px; height: 12px; border-radius: 50%; background: #FF9800; display: inline-block; margin-right: 8px; }");
  client.println(".device-controls { background: #f0f8ff; border: 2px solid #2196F3; padding: 20px; border-radius: 10px; margin: 20px 0; }");
  client.println(".controller-controls { background: #fff3e0; border: 2px solid #FF9800; padding: 20px; border-radius: 10px; margin: 20px 0; }");
  client.println(".device-item { cursor: pointer; padding: 10px; border: 1px solid #ddd; border-radius: 5px; margin: 5px 0; background: white; transition: background 0.3s; }");
  client.println(".device-item:hover { background: #f0f8ff; }");
  client.println(".device-item.selected { background: #e3f2fd; border-color: #2196F3; }");
  client.println(".device-item.controller { background: #fff3e0; border-color: #FF9800; }");
  client.println(".device-item.controller.selected { background: #ffe0b2; border-color: #FF9800; }");
  client.println(".small-btn { padding: 5px 8px; font-size: 11px; margin: 0 2px; }");
  client.println(".controller-btn { background: #FF9800; color: white; }");
  client.println(".controller-btn:hover { background: #F57C00; transform: translateY(-2px); }");
  client.println(".warning-text { color: #f44336; font-size: 12px; margin-top: 5px; }");
  client.println("</style>");
  client.println("</head><body>");
  
  client.println("<div class='container'>");
  client.println("<h1>LED Controller</h1>");
  client.println("<div id='connectionStatus' class='status'>Connecting...</div>");
  
  client.println("<div class='controls'>");
  client.println("<button class='power-on' onclick='sendCommand(\"power\", \"on\")'>Turn All LEDs ON</button>");
  client.println("<button class='power-off' onclick='sendCommand(\"power\", \"off\")'>Turn All LEDs OFF</button>");
  client.println("</div>");
  
  client.println("<div class='mode-controls'>");
  client.println("<h3>Set Mode for All LEDs:</h3>");
  client.println("<div class='mode-buttons'>");
  client.println("<button class='mode-btn' onclick='sendCommand(\"mode\", \"0\")'>Constant</button>");
  client.println("<button class='mode-btn' onclick='sendCommand(\"mode\", \"1\")'>Breathing</button>");
  client.println("<button class='mode-btn' onclick='sendCommand(\"mode\", \"2\")'>Fast Blink</button>");
  client.println("<button class='mode-btn' onclick='sendCommand(\"mode\", \"3\")'>Slow Pulse</button>");
  client.println("<button class='mode-btn' onclick='sendCommand(\"mode\", \"4\")'>Heartbeat</button>");
  client.println("</div>");
  client.println("</div>");
  
  client.println("<div class='device-info'>");
  client.println("<h3>Connected Devices:</h3>");
  client.println("<div id='deviceList'>No devices found yet...</div>");
  client.println("</div>");
  
  // LED Device Controls
  client.println("<div class='device-controls' style='display:none;' id='deviceControlPanel'>");
  client.println("<h3>Control Selected Device: <span id='selectedDeviceName'></span></h3>");
  client.println("<div class='controls'>");
  client.println("<button class='power-on' onclick='sendSelectedDeviceCommand(\"power\", \"on\")'>Turn Device ON</button>");
  client.println("<button class='power-off' onclick='sendSelectedDeviceCommand(\"power\", \"off\")'>Turn Device OFF</button>");
  client.println("</div>");
  client.println("<div class='mode-controls'>");
  client.println("<h4>Device Modes:</h4>");
  client.println("<div class='mode-buttons'>");
  client.println("<button class='mode-btn' onclick='sendSelectedDeviceCommand(\"mode\", \"0\")'>Constant</button>");
  client.println("<button class='mode-btn' onclick='sendSelectedDeviceCommand(\"mode\", \"1\")'>Breathing</button>");
  client.println("<button class='mode-btn' onclick='sendSelectedDeviceCommand(\"mode\", \"2\")'>Fast Blink</button>");
  client.println("<button class='mode-btn' onclick='sendSelectedDeviceCommand(\"mode\", \"3\")'>Slow Pulse</button>");
  client.println("<button class='mode-btn' onclick='sendSelectedDeviceCommand(\"mode\", \"4\")'>Heartbeat</button>");
  client.println("</div>");
  client.println("</div>");
  client.println("<div style='margin: 20px 0;'>");
  client.println("<h4>Device Information:</h4>");
  client.println("<div id='deviceInfo' style='background: #f5f5f5; padding: 15px; border-radius: 5px; font-family: monospace; font-size: 12px;'>");
  client.println("Select a device to view information...");
  client.println("</div>");
  client.println("</div>");
  client.println("<div style='margin: 15px 0;'>");
  client.println("<button onclick='openSelectedDevicePage()' style='background: #9C27B0; color: white; width: 100%; padding: 12px; border: none; border-radius: 5px; font-size: 14px; font-weight: bold;'>Open Device Web Interface</button>");
  client.println("</div>");
  client.println("<button onclick='closeDeviceControls()' style='background: #666; color: white; width: 100%; margin-top: 15px;'>Close Device Controls</button>");
  client.println("</div>");
  
  // Controller Controls
  client.println("<div class='controller-controls' style='display:none;' id='controllerControlPanel'>");
  client.println("<h3>Controller Management</h3>");
  client.println("<div style='display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin: 20px 0;'>");
  client.println("<button class='controller-btn' onclick='restartController()'>Restart Controller</button>");
  client.println("<button class='controller-btn' onclick='refreshDevices()'>Refresh Devices</button>");
  client.println("</div>");
  client.println("<div style='margin: 20px 0;'>");
  client.println("<button class='controller-btn' onclick='clearDeviceList()' style='width: 100%;'>Clear Device List</button>");
  client.println("<div class='warning-text'>This will clear all tracked devices from memory</div>");
  client.println("</div>");
  client.println("<div style='margin: 20px 0;'>");
  client.println("<h4>Controller Information:</h4>");
  client.println("<div id='controllerInfo' style='background: #f5f5f5; padding: 15px; border-radius: 5px; font-family: monospace; font-size: 12px;'>");
  client.println("Device ID: UNO_Controller<br>");
  client.println("IP Address: " + WiFi.localIP().toString() + "<br>");
  client.println("Uptime: " + String((millis() - startupTime) / 60000) + " minutes<br>");
  client.println("Free Memory: " + String(getFreeMemory()) + " bytes<br>");
  client.println("WiFi Signal: " + String(WiFi.RSSI()) + " dBm<br>");
  client.println("Connected Devices: " + String(deviceCount) + "<br>");
  client.println("MQTT Status: " + String(mqttClient.connected() ? "Connected" : "Disconnected") + "<br>");
  client.println("Firmware: v1.0<br>");
  client.println("Total Memory: 32KB SRAM<br>");
  client.println("Board: Arduino UNO R4 WiFi");
  client.println("</div>");
  client.println("</div>");
  client.println("<button onclick='closeControllerControls()' style='background: #666; color: white; width: 100%; margin-top: 15px;'>Close Controller Controls</button>");
  client.println("</div>");
  
  client.println("</div>");
  
  // JavaScript
  client.println("<script>");
  client.println("let client; let connectedDevices = new Map(); let selectedDevice = null; let selectedDeviceType = 'device';");
  
  client.println("function updateStatus(message, type) {");
  client.println("  document.getElementById('connectionStatus').textContent = message;");
  client.println("  document.getElementById('connectionStatus').className = 'status ' + type;");
  client.println("}");
  
  client.println("function updateDeviceInfo(deviceId) {");
  client.println("  console.log('updateDeviceInfo called for:', deviceId);");
  client.println("  const device = connectedDevices.get(deviceId);");
  client.println("  console.log('Device data:', device);");
  client.println("  ");
  client.println("  if (!device) {");
  client.println("    document.getElementById('deviceInfo').innerHTML = 'Device not found...';");
  client.println("    return;");
  client.println("  }");
  client.println("  ");
  client.println("  // Show basic info even if detailed status isn't available yet");
  client.println("  let html = '';");
  client.println("  html += 'Device ID: ' + deviceId + '<br>';");
  client.println("  html += 'IP Address: ' + (device.ip || 'Unknown') + '<br>';");
  client.println("  html += 'Connection Status: ' + (device.status || 'Unknown') + '<br>';");
  client.println("  ");
  client.println("  if (device.details) {");
  client.println("    console.log('Device has details:', device.details);");
  client.println("    const details = device.details;");
  client.println("    const modeNames = ['Constant', 'Breathing', 'Fast Blink', 'Slow Pulse', 'Heartbeat'];");
  client.println("    const modeName = modeNames[details.mode] || 'Unknown';");
  client.println("    ");
  client.println("    html += 'Power Status: ' + (details.isDeviceOn ? 'ON' : 'OFF') + '<br>';");
  client.println("    html += 'Current Mode: ' + modeName + ' (' + details.mode + ')<br>';");
  client.println("    ");
  client.println("    if (details.uptime !== undefined) {");
  client.println("      html += 'Uptime: ' + Math.floor(details.uptime / 60) + ' minutes<br>';");
  client.println("    }");
  client.println("    ");
  client.println("    if (details.freeHeap !== undefined) {");
  client.println("      html += 'Free Memory: ' + details.freeHeap + ' bytes<br>';");
  client.println("    }");
  client.println("    ");
  client.println("    if (details.wifiSignal !== undefined) {");
  client.println("      html += 'WiFi Signal: ' + details.wifiSignal + ' dBm<br>';");
  client.println("    }");
  client.println("    ");
  client.println("    html += 'Board Type: ESP8266<br>';");
  client.println("    html += 'Firmware: LED Node v1.0<br>';");
  client.println("  } else {");
  client.println("    console.log('Device has no detailed status yet');");
  client.println("    html += 'Detailed Status: Waiting for device heartbeat...<br>';");
  client.println("    html += 'Board Type: ESP8266 (estimated)<br>';");
  client.println("    html += 'Firmware: LED Node v1.0<br>';");
  client.println("  }");
  client.println("  ");
  client.println("  html += 'Last Updated: ' + new Date().toLocaleTimeString();");
  client.println("  document.getElementById('deviceInfo').innerHTML = html;");
  client.println("  console.log('Device info updated with HTML:', html);");
  client.println("}");
  
  client.println("function updateControllerInfo(infoJson) {");
  client.println("  try {");
  client.println("    const info = JSON.parse(infoJson);");
  client.println("    let html = '';");
  client.println("    html += 'Device ID: ' + info.deviceId + '<br>';");
  client.println("    html += 'IP Address: ' + info.ip + '<br>';");
  client.println("    html += 'Uptime: ' + Math.floor(info.uptime / 60) + ' minutes<br>';");
  client.println("    html += 'Free Memory: ' + info.freeHeap + ' bytes<br>';");
  client.println("    html += 'WiFi Signal: ' + info.wifiSignal + ' dBm<br>';");
  client.println("    html += 'Connected Devices: ' + info.deviceCount + '<br>';");
  client.println("    html += 'MQTT Status: ' + (info.mqttConnected ? 'Connected' : 'Disconnected') + '<br>';");
  client.println("    html += 'Firmware: v1.0<br>';");
  client.println("    html += 'Total Memory: 32KB SRAM<br>';");
  client.println("    html += 'Board: Arduino UNO R4 WiFi<br>';");
  client.println("    html += 'Last Updated: ' + new Date().toLocaleTimeString();");
  client.println("    document.getElementById('controllerInfo').innerHTML = html;");
  client.println("  } catch (e) {");
  client.println("    console.log('Error parsing controller info:', e);");
  client.println("    document.getElementById('controllerInfo').innerHTML = 'Error parsing controller info';");
  client.println("  }");
  client.println("}");
  
  client.println("function handleMessage(topic, message) {");
  client.println("  const parts = topic.split('/');");
  client.println("  if (parts.length >= 3) {");
  client.println("    const deviceId = parts[2];");
  client.println("    // Skip the old 'controller' entry - only use 'UNO_Controller'");
  client.println("    if (deviceId === 'controller') return;");
  client.println("    if (topic.endsWith('/status')) {");
  client.println("      if (message === 'online') {");
  client.println("        connectedDevices.set(deviceId, { status: 'online', ip: 'Unknown', type: deviceId === 'UNO_Controller' ? 'controller' : 'device' });");
  client.println("        console.log('Device came online:', deviceId);");
  client.println("      } else if (message === 'offline') {");
  client.println("        connectedDevices.delete(deviceId);");
  client.println("        console.log('Device went offline:', deviceId);");
  client.println("      }");
  client.println("      updateDeviceList();");
  client.println("    } else if (topic.endsWith('/detailed_status')) {");
  client.println("      try {");
  client.println("        const details = JSON.parse(message);");
  client.println("        console.log('Received detailed status for', deviceId, ':', details);");
  client.println("        if (connectedDevices.has(deviceId)) {");
  client.println("          const device = connectedDevices.get(deviceId);");
  client.println("          device.details = details;");
  client.println("          if (details.deviceIP) device.ip = details.deviceIP;");
  client.println("          if (details.ip) device.ip = details.ip;");
  client.println("          connectedDevices.set(deviceId, device);");
  client.println("        } else {");
  client.println("          connectedDevices.set(deviceId, { status: 'online', details: details, ip: details.deviceIP || details.ip || 'Unknown', type: deviceId === 'UNO_Controller' ? 'controller' : 'device' });");
  client.println("        }");
  client.println("        updateDeviceList();");
  client.println("        // Update device info if this device is currently selected");
  client.println("        if (selectedDevice === deviceId && selectedDeviceType === 'device') {");
  client.println("          updateDeviceInfo(deviceId);");
  client.println("        }");
  client.println("      } catch (e) {");
  client.println("        console.log('Error parsing detailed status for', deviceId, ':', e);");
  client.println("      }");
  client.println("    } else if (topic.endsWith('/info')) {");
  client.println("      if (deviceId === 'UNO_Controller') {");
  client.println("        updateControllerInfo(message);");
  client.println("      }");
  client.println("    }");
  client.println("  }");
  client.println("}");
  
  client.println("function connectMQTT() {");
  client.println("  try {");
  client.println("    client = mqtt.connect('wss://broker.hivemq.com:8884/mqtt');");
  client.println("    client.on('connect', function() {");
  client.println("      updateStatus('Connected to MQTT broker', 'success');");
  client.println("      client.subscribe('home/leds/+/status');");
  client.println("      client.subscribe('home/leds/+/detailed_status');");
  client.println("      client.subscribe('home/leds/controller/info');");
  client.println("    });");
  client.println("    client.on('error', function(error) {");
  client.println("      updateStatus('Connection failed: ' + error.message, 'error');");
  client.println("    });");
  client.println("    client.on('message', function(topic, message) {");
  client.println("      handleMessage(topic, message.toString());");
  client.println("    });");
  client.println("  } catch (error) {");
  client.println("    updateStatus('Failed to connect: ' + error.message, 'error');");
  client.println("  }");
  client.println("}");
  
  client.println("function updateDeviceList() {");
  client.println("  const deviceListEl = document.getElementById('deviceList');");
  client.println("  if (connectedDevices.size === 0) {");
  client.println("    deviceListEl.innerHTML = 'No devices connected';");
  client.println("    return;");
  client.println("  }");
  client.println("  let html = '';");
  client.println("  connectedDevices.forEach((device, deviceId) => {");
  client.println("    const details = device.details || {};");
  client.println("    const isController = device.type === 'controller';");
  client.println("    const isSelected = selectedDevice === deviceId ? 'selected' : '';");
  client.println("    const deviceClass = isController ? 'controller' : '';");
  client.println("    html += '<div class=\"device-item ' + deviceClass + ' ' + isSelected + '\" onclick=\"selectDevice(\\'' + deviceId + '\\', \\'' + device.type + '\\')\">';");
  client.println("    html += '<div style=\"display: flex; justify-content: space-between; align-items: center;\">';");
  client.println("    if (isController) {");
  client.println("      html += '<div><span class=\"controller-indicator\"></span><strong>' + deviceId + ' (Controller)</strong><br>';");
  client.println("      html += '<small>Uptime: ' + (details.uptime ? Math.floor(details.uptime / 60) + 'm' : 'Unknown') + ' | Devices: ' + (details.deviceCount || 0) + '</small></div>';");
  client.println("      html += '<div><button class=\"controller-btn small-btn\" onclick=\"event.stopPropagation(); restartController();\">RESTART</button></div>';");
  client.println("    } else {");
  client.println("      const powerStatus = details.isDeviceOn ? 'ON' : 'OFF';");
  client.println("      const modeNames = ['Constant', 'Breathing', 'Fast Blink', 'Slow Pulse', 'Heartbeat'];");
  client.println("      const modeName = modeNames[details.mode] || 'Unknown';");
  client.println("      html += '<div><span class=\"online-indicator\"></span><strong>' + deviceId + '</strong><br>';");
  client.println("      html += '<small>Power: ' + powerStatus + ' | Mode: ' + modeName + '</small></div>';");
  client.println("      html += '<div>';");
  client.println("      html += '<button class=\"power-on small-btn\" onclick=\"event.stopPropagation(); sendDeviceCommand(\\'' + deviceId + '\\', \\'power\\', \\'on\\');\">ON</button>';");
  client.println("      html += '<button class=\"power-off small-btn\" onclick=\"event.stopPropagation(); sendDeviceCommand(\\'' + deviceId + '\\', \\'power\\', \\'off\\');\">OFF</button>';");
  client.println("      html += '</div>';");
  client.println("    }");
  client.println("    html += '</div></div>';");
  client.println("  });");
  client.println("  deviceListEl.innerHTML = html;");
  client.println("}");
  
  client.println("function sendCommand(command, value) {");
  client.println("  if (!client || !client.connected) {");
  client.println("    updateStatus('Not connected to MQTT broker', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  const topic = 'home/leds/all/' + command;");
  client.println("  client.publish(topic, value);");
  client.println("  const actions = { 'power': value === 'on' ? 'Turning all LEDs ON' : 'Turning all LEDs OFF', 'mode': 'Setting all LEDs to mode ' + value };");
  client.println("  updateStatus(actions[command] || 'Sending ' + command + ': ' + value, 'success');");
  client.println("}");
  
  client.println("function sendDeviceCommand(deviceId, command, value) {");
  client.println("  if (!client || !client.connected) {");
  client.println("    updateStatus('Not connected to MQTT broker', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  const topic = 'home/leds/' + deviceId + '/' + command;");
  client.println("  client.publish(topic, value);");
  client.println("  updateStatus('Sent ' + command + ' to ' + deviceId, 'success');");
  client.println("}");
  
  client.println("function selectDevice(deviceId, deviceType) {");
  client.println("  selectedDevice = deviceId;");
  client.println("  selectedDeviceType = deviceType;");
  client.println("  document.getElementById('deviceControlPanel').style.display = 'none';");
  client.println("  document.getElementById('controllerControlPanel').style.display = 'none';");
  client.println("  if (deviceType === 'controller') {");
  client.println("    document.getElementById('controllerControlPanel').style.display = 'block';");
  client.println("    updateStatus('Selected controller: ' + deviceId, 'success');");
  client.println("    requestControllerInfo();");
  client.println("  } else {");
  client.println("    document.getElementById('selectedDeviceName').textContent = deviceId;");
  client.println("    document.getElementById('deviceControlPanel').style.display = 'block';");
  client.println("    updateStatus('Selected device: ' + deviceId, 'success');");
  client.println("  }");
  client.println("  updateDeviceList();");
  client.println("}");
  
  client.println("function sendSelectedDeviceCommand(command, value) {");
  client.println("  if (!selectedDevice || selectedDeviceType === 'controller') {");
  client.println("    updateStatus('No LED device selected', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  sendDeviceCommand(selectedDevice, command, value);");
  client.println("}");
  
  client.println("function closeDeviceControls() {");
  client.println("  selectedDevice = null;");
  client.println("  selectedDeviceType = 'device';");
  client.println("  document.getElementById('deviceControlPanel').style.display = 'none';");
  client.println("  updateDeviceList();");
  client.println("  updateStatus('Device controls closed', 'success');");
  client.println("}");
  
  client.println("function closeControllerControls() {");
  client.println("  selectedDevice = null;");
  client.println("  selectedDeviceType = 'device';");
  client.println("  document.getElementById('controllerControlPanel').style.display = 'none';");
  client.println("  updateDeviceList();");
  client.println("  updateStatus('Controller controls closed', 'success');");
  client.println("}");
  
  client.println("function restartController() {");
  client.println("  if (!client || !client.connected) {");
  client.println("    updateStatus('Not connected to MQTT broker', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  if (confirm('Restart the controller? This will temporarily disconnect all device monitoring.')) {");
  client.println("    client.publish('home/leds/controller/restart', 'true');");
  client.println("    updateStatus('Restart command sent to controller', 'success');");
  client.println("    setTimeout(() => { closeControllerControls(); connectedDevices.delete('UNO_Controller'); updateDeviceList(); }, 1000);");
  client.println("  }");
  client.println("}");
  
  client.println("function refreshDevices() {");
  client.println("  if (!client || !client.connected) {");
  client.println("    updateStatus('Not connected to MQTT broker', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  client.publish('home/leds/controller/refresh', 'true');");
  client.println("  updateStatus('Device refresh requested', 'success');");
  client.println("}");
  
  client.println("function clearDeviceList() {");
  client.println("  if (!client || !client.connected) {");
  client.println("    updateStatus('Not connected to MQTT broker', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  if (confirm('Clear all tracked devices from controller memory?')) {");
  client.println("    client.publish('home/leds/controller/clear', 'true');");
  client.println("    updateStatus('Device list clear requested', 'success');");
  client.println("  }");
  client.println("}");
  
  client.println("function requestControllerInfo() {");
  client.println("  if (!client || !client.connected) return;");
  client.println("  client.publish('home/leds/controller/info_request', 'true');");
  client.println("}");
  
  client.println("function updateControllerInfo(infoJson) {");
  client.println("  try {");
  client.println("    const info = JSON.parse(infoJson);");
  client.println("    let html = '';");
  client.println("    html += 'Device ID: ' + info.deviceId + '<br>';");
  client.println("    html += 'IP Address: ' + info.ip + '<br>';");
  client.println("    html += 'Uptime: ' + Math.floor(info.uptime / 60) + ' minutes<br>';");
  client.println("    html += 'Free Memory: ' + info.freeHeap + ' bytes<br>';");
  client.println("    html += 'WiFi Signal: ' + info.wifiSignal + ' dBm<br>';");
  client.println("    html += 'Connected Devices: ' + info.deviceCount + '<br>';");
  client.println("    html += 'MQTT Status: ' + (info.mqttConnected ? 'Connected' : 'Disconnected');");
  client.println("    document.getElementById('controllerInfo').innerHTML = html;");
  client.println("  } catch (e) {");
  client.println("    document.getElementById('controllerInfo').innerHTML = 'Error parsing controller info';");
  client.println("  }");
  client.println("}");
  
  client.println("function openSelectedDevicePage() {");
  client.println("  if (!selectedDevice || selectedDeviceType === 'controller') {");
  client.println("    updateStatus('No LED device selected', 'error');");
  client.println("    return;");
  client.println("  }");
  client.println("  const device = connectedDevices.get(selectedDevice);");
  client.println("  if (device && device.ip && device.ip !== 'Unknown') {");
  client.println("    const deviceURL = 'http://' + device.ip + ':8080';");
  client.println("    window.open(deviceURL, '_blank');");
  client.println("    updateStatus('Opened device page for ' + selectedDevice, 'success');");
  client.println("  } else {");
  client.println("    updateStatus('Device IP not available yet', 'error');");
  client.println("  }");
  client.println("}");
  
  client.println("window.onload = function() { connectMQTT(); };");
  client.println("</script>");
  client.println("</body></html>");
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.disconnect();
  delay(1000);
  WiFi.begin(ssid, password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected! Waiting for IP assignment...");
    
    startTime = millis();
    while (WiFi.localIP() == IPAddress(0, 0, 0, 0) && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
      Serial.println("\nIP address assigned successfully!");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("\nIP assignment failed!");
    }
  } else {
    Serial.println("\nWiFi connection failed!");
    matrix.renderBitmap(digit0, 8, 12);
  }
}

void connectToMQTT() {
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println("No valid WiFi connection - skipping MQTT connection");
    return;
  }
  
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(mqtt_server);
  
  String clientId = "UNO_Controller_" + String(millis());
  String lwt_topic = "home/leds/" + CONTROLLER_ID + "/status";
  
  if (mqttClient.connect(clientId.c_str(), lwt_topic.c_str(), 1, true, "offline")) {
    Serial.println("MQTT connected successfully!");
    
    // Subscribe to device status topics
    mqttClient.subscribe("home/leds/+/status");
    mqttClient.subscribe("home/leds/+/detailed_status");
    
    // Subscribe to controller command topics
    mqttClient.subscribe("home/leds/controller/restart");
    mqttClient.subscribe("home/leds/controller/refresh");
    mqttClient.subscribe("home/leds/controller/clear");
    mqttClient.subscribe("home/leds/controller/info_request");
    
    // Publish controller online status
    mqttClient.publish(lwt_topic.c_str(), "online", true);
    
    Serial.println("Subscribed to device status and controller command topics");
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqttClient.state());
  }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  Serial.print("MQTT: ");
  Serial.print(topicStr);
  Serial.print(" = ");
  Serial.println(message);
  
  // Handle controller commands
  if (topicStr == "home/leds/controller/restart") {
    Serial.println("Restart command received - restarting controller...");
    delay(1000);
    NVIC_SystemReset(); // UNO R4 WiFi reset function
  }
  else if (topicStr == "home/leds/controller/refresh") {
    Serial.println("Refresh devices command received");
    checkStaleDevices();
    updateDisplay();
    sendControllerHeartbeat();
  }
  else if (topicStr == "home/leds/controller/clear") {
    Serial.println("Clear device list command received");
    deviceCount = 0;
    connectedDevices = 0;
    for (int i = 0; i < 10; i++) {
      knownDevices[i] = "";
      deviceIPs[i] = "";
      deviceLastSeen[i] = 0;
    }
    updateDisplay();
    Serial.println("Device list cleared");
  }
  else if (topicStr == "home/leds/controller/info_request") {
    Serial.println("Controller info request received");
    sendControllerInfo();
  }
  else if (topicStr.startsWith("home/leds/") && topicStr.endsWith("/status")) {
    handleDeviceStatus(topicStr, message);
  } 
  else if (topicStr.startsWith("home/leds/") && topicStr.endsWith("/detailed_status")) {
    // Update last seen time for detailed status messages
    int firstSlash = topicStr.indexOf('/', 0);
    int secondSlash = topicStr.indexOf('/', firstSlash + 1);
    int thirdSlash = topicStr.indexOf('/', secondSlash + 1);
    
    if (firstSlash != -1 && secondSlash != -1 && thirdSlash != -1) {
      String deviceId = topicStr.substring(secondSlash + 1, thirdSlash);
      
      for (int i = 0; i < deviceCount; i++) {
        if (knownDevices[i] == deviceId) {
          deviceLastSeen[i] = millis();
          break;
        }
      }
    }
  }
}

void sendControllerHeartbeat() {
  if (!mqttClient.connected()) return;
  
  StaticJsonDocument<256> doc;
  doc["deviceId"] = CONTROLLER_ID;
  doc["uptime"] = (millis() - startupTime) / 1000;
  doc["freeHeap"] = getFreeMemory();
  doc["wifiSignal"] = WiFi.RSSI();
  doc["deviceCount"] = deviceCount;
  doc["mqttConnected"] = mqttClient.connected();
  doc["ip"] = WiFi.localIP().toString();
  
  String statusJson;
  serializeJson(doc, statusJson);
  
  String statusTopic = "home/leds/" + CONTROLLER_ID + "/detailed_status";
  mqttClient.publish(statusTopic.c_str(), statusJson.c_str());
  
  Serial.print("Controller heartbeat sent - Devices: ");
  Serial.println(deviceCount);
}

void sendControllerInfo() {
  if (!mqttClient.connected()) return;
  
  StaticJsonDocument<256> doc;
  doc["deviceId"] = CONTROLLER_ID;
  doc["uptime"] = (millis() - startupTime) / 1000;
  doc["freeHeap"] = getFreeMemory();
  doc["wifiSignal"] = WiFi.RSSI();
  doc["deviceCount"] = deviceCount;
  doc["mqttConnected"] = mqttClient.connected();
  doc["ip"] = WiFi.localIP().toString();
  
  String infoJson;
  serializeJson(doc, infoJson);
  
  String infoTopic = "home/leds/controller/info";
  mqttClient.publish(infoTopic.c_str(), infoJson.c_str());
  
  Serial.println("Controller info sent");
}

// Simple memory estimation for UNO R4 WiFi
int getFreeMemory() {
  // UNO R4 WiFi has 32KB SRAM total
  // This is a simplified estimation since we can't easily get exact free memory
  return 32768 - 8192; // Estimate ~24KB available (conservative estimate)
}

void handleDeviceStatus(String topic, String message) {
  int firstSlash = topic.indexOf('/', 0);
  int secondSlash = topic.indexOf('/', firstSlash + 1);
  int thirdSlash = topic.indexOf('/', secondSlash + 1);
  
  if (firstSlash == -1 || secondSlash == -1 || thirdSlash == -1) {
    Serial.println("Invalid topic format");
    return;
  }
  
  String deviceId = topic.substring(secondSlash + 1, thirdSlash);
  
  // Skip controller messages, all messages, and the old "controller" entry
  if (deviceId == "controller" || deviceId == "all" || deviceId == "UNO_Controller") return;
  
  Serial.print("Device status update: ");
  Serial.print(deviceId);
  Serial.print(" = ");
  Serial.println(message);
  
  if (message == "online") {
    bool deviceExists = false;
    int deviceIndex = -1;
    
    for (int i = 0; i < deviceCount; i++) {
      if (knownDevices[i] == deviceId) {
        deviceExists = true;
        deviceIndex = i;
        break;
      }
    }
    
    if (deviceExists) {
      deviceLastSeen[deviceIndex] = millis();
      Serial.print("Device reconnected: ");
      Serial.println(deviceId);
    } else if (deviceCount < 10) {
      knownDevices[deviceCount] = deviceId;
      deviceLastSeen[deviceCount] = millis();
      deviceCount++;
      Serial.print("NEW Device added: ");
      Serial.print(deviceId);
      Serial.print(" - Total LED devices: ");
      Serial.println(deviceCount);
    }
  } 
  else if (message == "offline") {
    Serial.print("Device reported offline: ");
    Serial.println(deviceId);
  }
  
  connectedDevices = deviceCount;
  updateDisplay();
  
  Serial.print("Current LED device list (");
  Serial.print(deviceCount);
  Serial.print("): ");
  for (int i = 0; i < deviceCount; i++) {
    unsigned long timeSinceLastSeen = millis() - deviceLastSeen[i];
    Serial.print(knownDevices[i]);
    Serial.print(" (");
    Serial.print(timeSinceLastSeen / 1000);
    Serial.print("s ago)");
    if (i < deviceCount - 1) Serial.print(", ");
  }
  Serial.println();
}

void checkStaleDevices() {
  unsigned long currentTime = millis();
  
  for (int i = deviceCount - 1; i >= 0; i--) {
    unsigned long timeSinceLastSeen = currentTime - deviceLastSeen[i];
    
    if (timeSinceLastSeen > deviceTimeoutPeriod) {
      Serial.print("Removing stale device: ");
      Serial.print(knownDevices[i]);
      Serial.print(" (offline for ");
      Serial.print(timeSinceLastSeen / 1000);
      Serial.println(" seconds)");
      
      for (int j = i; j < deviceCount - 1; j++) {
        knownDevices[j] = knownDevices[j + 1];
        deviceLastSeen[j] = deviceLastSeen[j + 1];
        deviceIPs[j] = deviceIPs[j + 1];
      }
      deviceCount--;
      connectedDevices = deviceCount;
      
      Serial.print("Remaining devices: ");
      Serial.println(deviceCount);
    }
  }
}

void updateDisplay() {
  int displayNumber = min(connectedDevices, 9);
  
  Serial.print("Updating display to show: ");
  Serial.print(displayNumber);
  Serial.print(" (actual connected devices: ");
  Serial.print(connectedDevices);
  Serial.println(")");
  
  switch(displayNumber) {
    case 0: matrix.renderBitmap(digit0, 8, 12); break;
    case 1: matrix.renderBitmap(digit1, 8, 12); break;
    case 2: matrix.renderBitmap(digit2, 8, 12); break;
    case 3: matrix.renderBitmap(digit3, 8, 12); break;
    case 4: matrix.renderBitmap(digit4, 8, 12); break;
    case 5: matrix.renderBitmap(digit5, 8, 12); break;
    case 6: matrix.renderBitmap(digit6, 8, 12); break;
    case 7: matrix.renderBitmap(digit7, 8, 12); break;
    case 8: matrix.renderBitmap(digit8, 8, 12); break;
    case 9: matrix.renderBitmap(digit9, 8, 12); break;
    default: matrix.renderBitmap(digit0, 8, 12); break;
  }
  
  Serial.print("Display updated: ");
  Serial.print(connectedDevices);
  Serial.println(" devices");
}

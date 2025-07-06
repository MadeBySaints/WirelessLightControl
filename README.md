# Wireless Light Control System
![System Overview](https://img.shields.io/badge/Platform-Arduino-blue) ![ESP8266](https://img.shields.io/badge/ESP8266-Compatible-green) ![MQTT](https://img.shields.io/badge/Protocol-MQTT-orange) ![OTA](https://img.shields.io/badge/Updates-OTA-red)


A wireless LED control system using Arduino UNO R4 WiFi as a central controller and ESP8266 devices as LED nodes. Control multiple LED devices remotely through a web interface with MQTT communication.

## 🌟 Features

- **Centralized Control**: Single web interface to manage all LED devices
- **Real-time Monitoring**: Live device status, uptime, memory usage, and WiFi signal
- **Multiple LED Patterns**: Constant, Breathing, Fast Blink, Slow Pulse, Heartbeat
- **Wireless Updates**: Over-the-air (OTA) firmware updates for all devices
- **Device Management**: Add, remove, and monitor LED devices automatically
- **Mobile Responsive**: Works on phones, tablets, and desktops
- **Controller Management**: Restart, refresh, and manage the central controller
- **Individual Device Access**: Direct access to each LED device's web interface

## 🏗️ System Architecture

```
┌─────────────────┐    MQTT     ┌─────────────────┐
│  UNO R4 WiFi    │◄──────────► │  MQTT Broker    │
│  (Controller)   │             │  (HiveMQ)       │
│  Web Interface  │             └─────────────────┘
└─────────────────┘                      ▲
         ▲                               │
         │ WiFi Network                  │ MQTT
         ▼                               ▼
┌─────────────────┐             ┌─────────────────┐
│   Your Device   │             │   ESP8266 #1    │
│  (Web Browser)  │             │   LED Device    │
└─────────────────┘             └─────────────────┘
                                         │
                                   ┌─────────────────┐
                                   │   ESP8266 #2    │
                                   │   LED Device    │
                                   └─────────────────┘
                                         │
                                   ┌─────────────────┐
                                   │   ESP8266 #N    │
                                   │   LED Device    │
                                   └─────────────────┘
```

## 📋 Hardware Requirements

### Controller
- **Arduino UNO R4 WiFi** (1x)
- USB cable for initial programming

### LED Devices
- **ESP8266** boards (Wemos D1 Mini, NodeMCU, etc.)
- **LEDs** - meant for cheap led strings that everyone throws away when they die.
- **Resistors** (if needed for your LED setup)
- Power supplies for each device

## 📦 Software Dependencies

### Arduino IDE Libraries
Install these libraries through the Arduino IDE Library Manager:

**For UNO R4 WiFi Controller:**
- `WiFiS3` (included with UNO R4 WiFi board package)
- `Arduino_LED_Matrix` (included with UNO R4 WiFi board package)
- `ArduinoJson` by Benoit Blanchon
- `PubSubClient` by Nick O'Leary

**For ESP8266 LED Devices:**
- `ESP8266WiFi` (included with ESP8266 board package)
- `PubSubClient` by Nick O'Leary
- `ArduinoJson` by Benoit Blanchon
- `NTPClient` by Fabrice Weinberg
- `ArduinoOTA` (included with ESP8266 board package)

## 🚀 Quick Start

### 1. Setup MQTT Broker
This project uses HiveMQ's free public broker, but you can use any MQTT broker:
- **Public Broker**: `broker.hivemq.com:1883` (default)
- **Local Broker**: Install Mosquitto or similar locally

### 2. Configure WiFi
Update WiFi credentials in both code files:
```cpp
const char* ssid = "Your_WiFi_Name";
const char* password = "Your_WiFi_Password";
```

### 3. Upload Controller Code
1. Connect your UNO R4 WiFi to your computer
2. Open `UNO_R4_Controller.ino` in Arduino IDE
3. Select board: "Arduino UNO R4 WiFi"
4. Upload the code
5. Note the IP address from Serial Monitor

### 4. Upload LED Device Code
1. Connect your ESP8266 device to your computer
2. Open `ESP8266_LED_Device.ino` in Arduino IDE
3. Select your ESP8266 board (e.g., "LOLIN(WEMOS) D1 R2 & mini")
4. Modify `DEVICE_BASE_NAME` if desired (default: "Kitchen")
5. Connect your LED to pin D5 (GPIO14) or modify `ledPin`
6. Upload the code
7. Repeat for additional LED devices

### 5. Access Web Interface
1. Open a web browser
2. Navigate to the UNO R4 WiFi IP address
3. Start controlling your LED devices!

## 🎛️ Web Interface Features

### Main Control Panel
- **Global Controls**: Turn all LEDs on/off, set modes for all devices
- **Device List**: See all connected LED devices and controller
- **Real-time Status**: Live updates every 10 seconds

### Device Information Panels
**LED Devices show:**
- Device ID and IP address
- Power status (ON/OFF)
- Current LED mode
- Uptime, memory usage, WiFi signal
- Board type and firmware version

**Controller shows:**
- System uptime and memory
- Connected device count
- MQTT status
- Management controls (restart, refresh, clear)

### Individual Device Management
- Click any LED device to access detailed controls
- Power control and mode selection
- Direct access to device's web interface for OTA updates

## 🔧 Configuration

### LED Modes
- **Mode 0**: Constant on
- **Mode 1**: Breathing effect
- **Mode 2**: Fast random blink
- **Mode 3**: Slow pulse
- **Mode 4**: Heartbeat pattern

### OTA Updates
Each ESP8266 device provides a web interface for firmware updates:
- Access: `http://device-ip:8080`
- Credentials: `admin` / `ledupdate123`
- Upload compiled `.bin` files wirelessly

### Device Naming
Modify the device base name in ESP8266 code:
```cpp
const String DEVICE_BASE_NAME = "Kitchen"; // Change to your preference
```
Devices will appear as: `Kitchen_abc123`, `Bedroom_def456`, etc.

## 📡 MQTT Topics

The system uses the following MQTT topic structure:

### Device Control

- `home/leds/all/power` - Control all devices (on/off)
- `home/leds/all/mode` - Set mode for all devices (0-4)
- `home/leds/{device_id}/power` - Control specific device
- `home/leds/{device_id}/mode` - Set mode for specific device

### Device Status
- `home/leds/{device_id}/status` - Device online/offline status
- `home/leds/{device_id}/detailed_status` - JSON with device details

### Controller Commands
- `home/leds/controller/restart` - Restart controller
- `home/leds/controller/refresh` - Refresh device list
- `home/leds/controller/clear` - Clear tracked devices
- `home/leds/controller/info_request` - Request controller info

## 🔐 Security Notes

- **Default OTA Password**: `ledupdate123` (change in production)
- **Web OTA Credentials**: `admin` / `ledupdate123`
- **MQTT**: No authentication by default (configure for production use)
- **Network**: Devices use your local WiFi network

## 🛠️ Customization

### Adding More LED Patterns
Add new patterns in the ESP8266 code:
```cpp
void myCustomPattern() {
  // Your LED pattern code here
}

// Add to runLEDPattern() switch statement:
case 5: myCustomPattern(); break;
```

### Changing Update Intervals
Modify timing constants in the code:
```cpp
const unsigned long heartbeatInterval = 30000; // Device status updates
const unsigned long displayUpdateInterval = 5000; // Display refresh
```

### Custom MQTT Broker
Update the broker settings:
```cpp
const char* mqtt_server = "your-broker.com";
const int mqtt_port = 1883;
```

## 🐛 Troubleshooting

### Device Not Appearing in List
1. Check WiFi connection (devices on same network)
2. Verify MQTT broker connectivity
3. Check Serial Monitor for error messages
4. Wait up to 2 minutes for device timeout/refresh

### OTA Update Fails
1. Ensure `.bin` file is for correct platform (ESP8266)
2. Check available memory on device
3. Verify stable WiFi connection during update
4. Try smaller firmware if memory is limited

### Controller Shows Wrong Device Count
1. Use "Refresh Devices" button
2. Check for devices that went offline
3. Use "Clear Device List" to reset tracking

### Web Interface Not Loading
1. Verify controller IP address
2. Check if controller is connected to WiFi
3. Try accessing from different device/browser
4. Restart controller if needed

## Interface Screenshots
![Screenshot 2025-07-05 225459](https://github.com/user-attachments/assets/e4b0c883-b1e2-4f08-a695-32d149ee6d9a)
![Screenshot 2025-07-05 225414](https://github.com/user-attachments/assets/340f2f98-ea9f-496b-8008-c76be5bfdfb6)

## LED Matrix Screenshot
![PXL_20250706_025127542~2](https://github.com/user-attachments/assets/205b75a7-d098-45b6-8892-4a2508cd08d6)

## UNO R4 Serial Output Screenshot
![image](https://github.com/user-attachments/assets/25c4670a-4855-4940-8d67-ef7133f6e4c0)


## 📄 License

This project is open source and available under the MIT License.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For major changes, please open an issue first to discuss what you would like to change.

## 📞 Support

## 🙏 Acknowledgments

- **Arduino Community** for excellent libraries and documentation
- **HiveMQ** for providing a free public MQTT broker
- **ESP8266 Community** for comprehensive board support
- **ArduinoJson** library for easy JSON handling

## 📊 Project Status

- ✅ **Stable**: Core functionality working
- ✅ **OTA Updates**: Wireless firmware updates
- ✅ **Web Interface**: Complete control interface
- ✅ **Device Management**: Automatic device discovery
- 🔄 **Future**: Scheduling features?, More LED patterns?

---

**Made with ❤️ for the Arduino and IoT community**

# ESP32 Multi-Module & Hardware Tests

This repository contains standalone, self-contained test projects to verify hardware modules, pin connections, and sensor integration with the **ESP32** microcontroller.

---

## 📂 Project Structure

```text
├── esp32_blink_test/        # Verifies simple MCU operations and built-in LED functionality
├── esp32_w5500_test/        # Verifies W5500 SPI Ethernet module connectivity and DHCP internet access
└── esp32_sim900a_test/      # Verifies SIM900A GSM module serial communication, network registration, and SMS sending
```

---

## 1. ESP32 Simple Blink Test

This project performs a basic blink verification using the ESP32's built-in LED to confirm that code upload, serial communication, and the basic clock are working correctly.

### 📋 Hardware Stack
- **Exact microcontroller board**: ESP32 DevKit V1 (38-pin variant)
- **Exact module**: On-board built-in Blue LED.

### 🔌 Pinout Mapping
The on-board blue LED is hardwired to:
- **GPIO 2** (defined as `LED_BUILTIN` in the sketch).

### ⚙️ Testing Procedure
1. Compile and upload the sketch in `esp32_blink_test` to the ESP32.
2. Open the Serial Monitor at **115200** baud.
3. Observe the on-board blue LED on the ESP32 board. It will cycle:
   - **ON** for 1 second.
   - **OFF** for 1 second.
4. The Serial Monitor will print `LED ON` and `LED OFF` in sync with the physical LED.

### 🖼️ ESP32 38-Pin Pinout Diagram
![ESP32 DevKit V1 38-Pin Pinout Diagram](https://europe1.discourse-cdn.com/arduino/original/4X/9/a/b/9abcaad3fd7d164799a08a7bec725500c67472b9.jpeg)

---

## 2. ESP32 Wiznet W5500 SPI Ethernet Test

This project initializes the Wiznet W5500 Ethernet controller over the SPI bus, checks for hardware responsiveness via a version register diagnostic, requests an IP address via DHCP, and performs an HTTP GET request to verify internet connectivity.

### 📋 Hardware Stack
- **Exact microcontroller board**: [ESP32 DevKit V1 (38-pin variant)](https://europe1.discourse-cdn.com/arduino/original/4X/9/a/b/9abcaad3fd7d164799a08a7bec725500c67472b9.jpeg)
- **Exact module**: Wiznet W5500 SPI Ethernet Breakout Board (3.3V)

### 🔌 Pinout Mapping
The W5500 connects to the ESP32 using the standard **VSPI** bus and control pins:

| W5500 Breakout Pin | ESP32 Pin | Purpose | Wire Color Recommendation |
| :--- | :--- | :--- | :--- |
| **VCC / 3.3V** | **3.3V** | Power Supply (Ensure stable 3.3V, W5500 draws ~150mA) | Red |
| **GND** | **GND** | Common Ground | Black |
| **SCS / CS** | **GPIO 5** | SPI Chip Select (SS) | Yellow |
| **SCLK / SCK** | **GPIO 18** | SPI Clock | Orange |
| **MISO / SO** | **GPIO 19** | SPI Master In Slave Out | Green |
| **MOSI / SI** | **GPIO 23** | SPI Master Out Slave In | Blue |
| **RST** | **GPIO 26** | Hardware Reset Pin (active LOW) | Purple |
| **INT / IRQ** | *(Unused)* | Interrupt output (Set to `-1` in software) | - |

> [!WARNING]
> Do not connect the W5500 VCC to the ESP32 5V/VIN pin. Doing so can permanently damage the ESP32 GPIOs and the W5500 chip.

### ⚙️ Testing Procedure
1. Connect the W5500 pins to the ESP32 as defined in the Pinout Mapping.
2. Plug an active Ethernet LAN cable (connected to your router or switch) into the W5500 module.
3. Compile and upload the sketch in `esp32_w5500_test` to the ESP32.
4. Open the Serial Monitor at **115200** baud and reset the ESP32.
5. Observe the Serial Monitor logs. You should see:
   - **`Running SPI Hardware Diagnostic...`** -> Reading W5500 Version Register should print `0x04` and say `✅ SPI Communication with W5500 is working successfully!`.
   - **`Querying DHCP for IP Address...`** -> DHCP should successfully assign a local IP address (e.g. `10.81.100.156`).
   - **`Testing internet connectivity via Ethernet.h...`** -> The request to `api.ipify.org` should succeed and display your public IP address.

### 🖼️ W5500 Breakout Board Diagram
![Wiznet W5500 SPI Ethernet Pinout](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcS4qwLCZz0NghmSwr-9Fxo5HrWr2ck66eKnDAYgJ5CmN6vjGtEei5LU4Raq&s=10)

---

## 3. ESP32 SIM900A GSM Module Test

This project tests the SIM900A GSM module using standard Hardware Serial 2. It performs AT command diagnostics (checking power status, SIM card insertion, signal quality, and cellular network registration) and sets up a Serial passthrough for manual testing and sending custom SMS.

### 📋 Hardware Stack
- **Exact microcontroller board**: [ESP32 DevKit V1 (38-pin variant)](https://europe1.discourse-cdn.com/arduino/original/4X/9/a/b/9abcaad3fd7d164799a08a7bec725500c67472b9.jpeg)
- **Exact module**: [SIM900A GSM/GPRS Modem Module](https://forum.fritzing.org/uploads/default/original/3X/8/c/8c3771051a328fbee4878beb90bd97192a71a4e3.jpeg) (with 2x3 red level-shifter pin header).

### 🔌 Pinout Mapping
Based on your specific SIM900A module, connect the pins as follows:

1. **Power Connection (White Terminal Block)**:
   - Connect **VCC / 4-5V** to an external 5V 2A power supply.
   - Connect **GND / 0V** to the ground of the external supply.

2. **Signal Connection (2x3 Red Header)**:
   - Connect **VCC MCU** to the **3.3V** pin of the ESP32 (this references the level shifter to match the ESP32's 3.3V logic level).
   - Connect **GND** to the **GND** pin of the ESP32 (common ground reference).
   - Connect **TXD 3.3V** to **GPIO 16 (RX2)** of the ESP32.
   - Connect **RXD 3.3V** to **GPIO 17 (TX2)** of the ESP32.

| SIM900A Red Header Pin | ESP32 Pin | Purpose | Connection Note |
| :--- | :--- | :--- | :--- |
| **VCC MCU** | **3.3V** | Reference voltage for level shifter | **CRITICAL for 3.3V ESP32 logic** |
| **GND** | **GND** | Common Signal Ground | Shared reference |
| **TXD 3.3V** | **GPIO 16 (RX2)** | Serial Data Transmit | From SIM900A to ESP32 RX |
| **RXD 3.3V** | **GPIO 17 (TX2)** | Serial Data Receive | From ESP32 TX to SIM900A RX |
| **RST (Bottom Pads)** | **GPIO 4** *(Optional)*| Hardware Reset / KEY pulse | Toggle boot manually/via code |

> [!WARNING]
> Do NOT connect the ESP32 to **TXD 5V** or **RXD 5V** pins on the red header. The ESP32 is not 5V-tolerant and doing so may burn the serial pins on your ESP32.

### ⚙️ Testing Procedure
1. Insert an activated 2G-compatible SIM card (e.g. Grameenphone, Robi, or Banglalink with regular cash balance or active SMS bundle, and **SIM PIN lock disabled** in phone settings).
2. Wire the SIM900A to the ESP32 and external 5V 2A power supply as mapped. Make sure to screw in the GSM antenna.
3. Compile and upload the sketch in `esp32_sim900a_test` to the ESP32.
4. Open the Serial Monitor at **115200** baud and reboot the ESP32.
5. If the module LEDs do not light up, type **`PWR_KEY`** in the Serial Monitor input and press **Enter** to trigger a power pulse on GPIO 4, or press the physical key button on the SIM900A module.
6. The module's **STATUS** LED should light up, and the **NET** LED will start blinking rapidly. Once it connects to the network, the **NET** LED will blink slowly (once every 3 seconds).
7. View the diagnostic output on the Serial Monitor. It should show:
   - `AT` response: `OK`
   - `ATI` response: Module type info
   - `AT+CPIN?` response: `+CPIN: READY`
   - `AT+CSQ` response: Signal level (e.g. `+CSQ: 18,99`, higher is better)
   - `AT+CREG?` response: `+CREG: 0,1` (Home network) or `0,5` (Roaming)
8. **Interactive SMS Wizard**:
   - In the Serial Monitor input, type **`SEND`** and press **Enter**.
   - Type the receiver's phone number (in international format, e.g. `+88017XXXXXXXX`) and press **Enter**.
   - Type the message text you want to send and press **Enter**.
   - The ESP32 will send the commands and trigger SMS delivery!
9. **Direct Passthrough**: You can also type any custom AT command directly in the input bar and press Enter to query the modem manually.

### 🖼️ SIM900A GSM Module Diagram
![SIM900A GSM Module Diagram](https://forum.fritzing.org/uploads/default/original/3X/8/c/8c3771051a328fbee4878beb90bd97192a71a4e3.jpeg)

---

## 🚀 How to Run the Tests

### Option A: PlatformIO (Command Line)
1. Navigate to the desired project folder:
   ```bash
   cd esp32_blink_test
   # or
   cd esp32_w5500_test
   # or
   cd esp32_sim900a_test
   ```
2. Run the upload command:
   ```bash
   $env:PIP_USER="false"; python -m platformio run --target upload
   ```
3. Monitor the Serial output:
   ```bash
   python -m platformio device monitor -b 115200
   ```

### Option B: Arduino IDE (Graphical)
1. Open the `.ino` file in the Arduino IDE.
2. Select your ESP32 board and COM port under **Tools**.
3. Click **Upload**, and open the **Serial Monitor** at **115200** baud.

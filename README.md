# ESP32 Multi-Module & Hardware Tests

This repository contains standalone, self-contained test projects to verify hardware modules, pin connections, and sensor integration with the **ESP32** microcontroller.

---

## 📂 Project Structure

```text
├── esp32_blink_test/        # Verifies simple MCU operations and built-in LED functionality
└── esp32_w5500_test/        # Verifies W5500 SPI Ethernet module connectivity and DHCP internet access
```

---

## 1. ESP32 Simple Blink Test

This project performs a basic blink verification using the ESP32's built-in LED to confirm that code upload, serial communication, and the basic clock are working correctly.

### 📋 Hardware Stack
- **Exact microcontroller board**: [ESP32 DevKit V1 (38-pin variant)](https://europe1.discourse-cdn.com/arduino/original/4X/9/a/b/9abcaad3fd7d164799a08a7bec725500c67472b9.jpeg)
- **Exact module**: On-board built-in Blue LED.

### 🔌 Pinout Mapping
The on-board blue LED is hardwired to:
- **GPIO 2** (defined as `LED_BUILTIN` in the sketch).

---

## 2. ESP32 Wiznet W5500 SPI Ethernet Test

This project initializes the Wiznet W5500 Ethernet controller over the SPI bus, checks for hardware responsiveness via a version register diagnostic, requests an IP address via DHCP, and performs an HTTP GET request to verify internet connectivity.

### 📋 Hardware Stack
- **Exact microcontroller board**: [ESP32 DevKit V1 (38-pin variant)](https://europe1.discourse-cdn.com/arduino/original/4X/9/a/b/9abcaad3fd7d164799a08a7bec725500c67472b9.jpeg)
- **Exact module**: [Wiznet W5500 SPI Ethernet Breakout Board (3.3V)](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcS4qwLCZz0NghmSwr-9Fxo5HrWr2ck66eKnDAYgJ5CmN6vjGtEei5LU4Raq&s=10)

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

---

## 🚀 How to Run the Tests

### Option A: PlatformIO (Command Line)
1. Navigate to the desired project folder:
   ```bash
   cd esp32_blink_test
   # or
   cd esp32_w5500_test
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
2. If compiling the W5500 test on an ESP32 core version < 3.x, search for and install the **Ethernet** library by Arduino/Wiznet in the Library Manager.
3. Select your ESP32 board and COM port under **Tools**.
4. Click **Upload**, and open the **Serial Monitor** at **115200** baud.

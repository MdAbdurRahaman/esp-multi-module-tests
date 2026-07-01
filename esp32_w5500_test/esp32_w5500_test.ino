/**
 * @file esp32_w5500_test.ino
 * @brief Standalone Wiznet W5500 SPI Ethernet Test Sketch for ESP32.
 * 
 * This test sketch helps verify that your ESP32 can communicate with the Wiznet W5500
 * Ethernet module via the SPI bus, obtain an IP address via DHCP, and access the internet.
 * 
 * Hardware Connections:
 * - W5500 MOSI -> ESP32 GPIO 23 (VSPI MOSI)
 * - W5500 MISO -> ESP32 GPIO 19 (VSPI MISO)
 * - W5500 SCLK -> ESP32 GPIO 18 (VSPI SCK)
 * - W5500 CS   -> ESP32 GPIO 5  (VSPI SS / Chip Select)
 * - W5500 RST  -> ESP32 GPIO 26 (Reset - active LOW)
 * - W5500 INT  -> Unused / -1 (Interrupt - optional)
 * - W5500 VCC  -> ESP32 3.3V
 * - W5500 GND  -> ESP32 GND
 */

#include <SPI.h>
#include "esp_arduino_version.h"

// If on core v3 or newer, we can use Native ETH.h. Otherwise, use legacy/standard Ethernet.h.
#if defined(ESP_ARDUINO_VERSION) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0))
  #define USE_NATIVE_ETH_LIBRARY 1
#else
  #define USE_NATIVE_ETH_LIBRARY 0
#endif

// Pin definitions matching your hybrid network monitor reserved pins
#define ETH_SPI_MOSI 23
#define ETH_SPI_MISO 19
#define ETH_SPI_SCK  18
#define ETH_PHY_CS   5
#define ETH_PHY_RST  26
#define ETH_PHY_IRQ  -1  // Not using interrupts

#if USE_NATIVE_ETH_LIBRARY
  #include <ETH.h>
  #include <HTTPClient.h>
#else
  #include <Ethernet.h>
  // A dummy MAC address. Wiznet modules don't have built-in hardcoded MAC addresses.
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#endif

byte readW5500Version() {
    pinMode(ETH_PHY_CS, OUTPUT);
    digitalWrite(ETH_PHY_CS, HIGH);
    delay(10);
    
    digitalWrite(ETH_PHY_CS, LOW);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    
    // W5500 VERSIONR register is located at address 0x0039 in the Common Register Block.
    // Address: 0x0039
    SPI.transfer(0x00);
    SPI.transfer(0x39);
    
    // Control Phase: BSB=00000 (Common Reg), R/W=0 (Read), OP_MODE=00 (Variable Length) -> 0x00
    SPI.transfer(0x00);
    
    // Data Phase: Read 1 byte
    byte version = SPI.transfer(0x00);
    
    SPI.endTransaction();
    digitalWrite(ETH_PHY_CS, HIGH);
    return version;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n==================================================");
    Serial.println("       ESP32 Wiznet W5500 SPI Ethernet Test        ");
    Serial.println("==================================================");

    // Print pin configuration
    Serial.println("--- Hardcoded Pins Configuration ---");
    Serial.printf("  MOSI (TX) -> GPIO %d\n", ETH_SPI_MOSI);
    Serial.printf("  MISO (RX) -> GPIO %d\n", ETH_SPI_MISO);
    Serial.printf("  SCLK (CLK) -> GPIO %d\n", ETH_SPI_SCK);
    Serial.printf("  CS / SS   -> GPIO %d\n", ETH_PHY_CS);
    Serial.printf("  RST       -> GPIO %d\n", ETH_PHY_RST);
    Serial.println("------------------------------------");

    // Reset W5500 manually before starting
    if (ETH_PHY_RST >= 0) {
        Serial.println("Performing W5500 hardware reset...");
        pinMode(ETH_PHY_RST, OUTPUT);
        digitalWrite(ETH_PHY_RST, LOW);
        delay(50);
        digitalWrite(ETH_PHY_RST, HIGH);
        delay(300);
    }

    // Initialize SPI bus pins
    SPI.begin(ETH_SPI_SCK, ETH_SPI_MISO, ETH_SPI_MOSI, ETH_PHY_CS);

    // Run SPI Hardware Diagnostic
    Serial.println("Running SPI Hardware Diagnostic...");
    byte ver = readW5500Version();
    Serial.printf("SPI Diagnostic - W5500 Version Register: 0x%02X (Expected: 0x04)\n", ver);
    if (ver == 0x04) {
        Serial.println("✅ SPI Communication with W5500 is working successfully!");
    } else {
        Serial.println("❌ SPI Communication FAILED! Possible causes:");
        Serial.println("   1. Incorrect wiring (Check MOSI/MISO/SCK/CS pins).");
        Serial.println("   2. Insufficient power (W5500 needs stable 3.3V, don't use 5V).");
        Serial.println("   3. Faulty or unconnected Reset (RST) pin. Try setting ETH_PHY_RST to -1 in the code.");
        Serial.println("   4. Shared SPI bus collision.");
    }
    Serial.println("------------------------------------");

#if USE_NATIVE_ETH_LIBRARY
    Serial.println("Mode: Native ETH.h (lwIP-integrated)");
    Serial.println("Initializing Ethernet module driver...");
    bool success = ETH.begin(ETH_PHY_W5500, 1, ETH_PHY_CS, ETH_PHY_IRQ, ETH_PHY_RST, SPI);
    if (!success) {
        Serial.println("❌ Failed to initialize ETH driver!");
    } else {
        Serial.println("Driver initialized. Waiting for network link & DHCP IP address...");
    }

#else // Using standard Ethernet.h
    Serial.println("Mode: Legacy/Standard Ethernet.h");
    Ethernet.init(ETH_PHY_CS);
    
    Serial.println("Querying DHCP for IP Address...");
    if (Ethernet.begin(mac) == 0) {
        Serial.println("❌ Failed to configure Ethernet using DHCP!");
        if (Ethernet.hardwareStatus() == EthernetNoHardware) {
            Serial.println("❌ W5500 module was not found. Check wiring.");
        } else if (Ethernet.linkStatus() == LinkOFF) {
            Serial.println("❌ Ethernet cable is not connected.");
        }
    } else {
        Serial.print("✅ Got DHCP IP Address: ");
        Serial.println(Ethernet.localIP());
    }
#endif
}

void loop() {
#if USE_NATIVE_ETH_LIBRARY
    static bool was_connected = false;
    bool connected = ETH.linkUp() && (ETH.localIP() != IPAddress(0, 0, 0, 0));
    
    if (connected && !was_connected) {
        was_connected = true;
        Serial.println("\n🌐 Link is UP! Received IP: " + ETH.localIP().toString());
        Serial.println("Testing internet connectivity via HTTP Client...");
        
        HTTPClient http;
        http.begin("http://api.ipify.org");
        int httpCode = http.GET();
        if (httpCode > 0) {
            String payload = http.getString();
            Serial.printf("HTTP GET Response Code: %d\n", httpCode);
            Serial.printf("Your Public IP Address: %s\n", payload.c_str());
            Serial.println("🎉 Ethernet test successful!");
        } else {
            Serial.printf("❌ HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    
    if (!connected && was_connected) {
        was_connected = false;
        Serial.println("❌ Lost Ethernet connection.");
    }
#else
    // For legacy Ethernet.h, we run a check periodically
    static unsigned long last_check = 0;
    if (millis() - last_check > 15000 && Ethernet.localIP() != IPAddress(0,0,0,0)) {
        last_check = millis();
        Serial.println("\n🌐 Testing internet connectivity via Ethernet.h...");
        
        EthernetClient client;
        if (client.connect("api.ipify.org", 80)) {
            Serial.println("Connected to api.ipify.org. Sending request...");
            client.println("GET / HTTP/1.1");
            client.println("Host: api.ipify.org");
            client.println("Connection: close");
            client.println();
            
            // Read response headers and body
            unsigned long start = millis();
            String response = "";
            while (client.connected() && millis() - start < 3000) {
                while (client.available()) {
                    char c = client.read();
                    response += c;
                }
            }
            client.stop();
            
            // Find body (after double CRLF)
            int bodyIndex = response.indexOf("\r\n\r\n");
            if (bodyIndex != -1) {
                String ip = response.substring(bodyIndex + 4);
                ip.trim();
                Serial.printf("Your Public IP Address: %s\n", ip.c_str());
                Serial.println("🎉 Ethernet test successful!");
            } else {
                Serial.println("Could not parse response.");
            }
        } else {
            Serial.println("❌ Connection to api.ipify.org failed!");
        }
    }
#endif
    delay(100);
}

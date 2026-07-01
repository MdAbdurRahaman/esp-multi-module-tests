/**
 * @file esp32_sim900a_test.ino
 * @brief Standalone SIM900A GSM Module Test Sketch for ESP32.
 * 
 * Target Board: ESP32 DevKit V1 (38-pin variant)
 * 
 * Hardware Connections:
 * - SIM900A TXD -> ESP32 GPIO 16 (Hardware Serial 2 RX)
 * - SIM900A RXD -> ESP32 GPIO 17 (Hardware Serial 2 TX)
 * - SIM900A PWR -> ESP32 GPIO 4  (Optional Power Key trigger)
 * - SIM900A GND -> ESP32 GND     (CRITICAL: Share ground!)
 * 
 * ⚠️ POWER WARNING:
 * The SIM900A requires an external power supply (typically 5V, 2A). 
 * Do NOT power the SIM900A directly from the ESP32's 3.3V or 5V/VIN pin, 
 * as cellular transmission bursts can draw up to 2A and cause the ESP32 to reset or brown out.
 */

#include <Arduino.h>

#define SIM_RX_PIN  16   // RX2 (Connects to SIM900A TXD)
#define SIM_TX_PIN  17   // TX2 (Connects to SIM900A RXD)
#define SIM_PWR_PIN 4    // Connects to SIM900A PWRKEY / KEY pin (Optional)

// Setup HardwareSerial 2
HardwareSerial simSerial(2);

// Function prototypes
void pulsePowerKey();
void sendATCommand(const char* cmd, uint32_t timeout);
void sendSMS(const char* phoneNumber, const char* message);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n==================================================");
    Serial.println("         ESP32 SIM900A GSM Module Test            ");
    Serial.println("==================================================");

    // Initialize SIM900A PWR pin
    if (SIM_PWR_PIN >= 0) {
        pinMode(SIM_PWR_PIN, OUTPUT);
        digitalWrite(SIM_PWR_PIN, HIGH); // Standard idle high
    }

    // SIM900A usually defaults to 9600 baud (or auto-baud)
    Serial.println("Initializing Hardware Serial 2 at 9600 baud...");
    simSerial.begin(9600, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    delay(1000);

    Serial.println("--- Starting Automated Diagnostics ---");
    Serial.println("Sending standard ping 'AT'...");
    sendATCommand("AT", 2000);
    
    Serial.println("\nRetrieving module info (ATI)...");
    sendATCommand("ATI", 2000);

    Serial.println("\nChecking SIM Card Status (AT+CPIN?)...");
    sendATCommand("AT+CPIN?", 2000);

    Serial.println("\nChecking Signal Strength (AT+CSQ)...");
    sendATCommand("AT+CSQ", 2000);

    Serial.println("\nChecking Network Registration Status (AT+CREG?)...");
    sendATCommand("AT+CREG?", 2000);

    Serial.println("\nChecking Network Operator (AT+COPS?)...");
    sendATCommand("AT+COPS?", 2000);
    Serial.println("--------------------------------------\n");

    Serial.println("💡 Serial Passthrough Mode is now active.");
    Serial.println("💡 Type AT commands in the Serial Monitor (with Carriage Return + Line Feed).");
    Serial.println("💡 To send a test SMS, type 'SEND_SMS' and press enter.");
    Serial.println("--------------------------------------\n");
}

void loop() {
    // Read from SIM900A and output to Serial Monitor
    if (simSerial.available()) {
        while (simSerial.available()) {
            char c = simSerial.read();
            Serial.write(c);
        }
    }

    // Read from Serial Monitor and write to SIM900A
    if (Serial.available()) {
        String input = Serial.readString();
        input.trim();

        if (input == "SEND_SMS") {
            // Prompt SMS details
            Serial.println("\nPreparing to send SMS.");
            // REPLACE with your phone number and test message:
            sendSMS("+88017XXXXXXXX", "Hello from ESP32 & SIM900A!");
        } else if (input == "PWR_KEY") {
            pulsePowerKey();
        } else {
            // Send user typed AT command to SIM900A
            simSerial.println(input);
        }
    }
}

// Pulse PWRKEY pin to boot/shutdown the module
void pulsePowerKey() {
    if (SIM_PWR_PIN >= 0) {
        Serial.println("Toggling PWRKEY (pulsing LOW for 1.5 seconds)...");
        digitalWrite(SIM_PWR_PIN, LOW);
        delay(1500);
        digitalWrite(SIM_PWR_PIN, HIGH);
        delay(2000);
        Serial.println("PWRKEY toggle completed.");
    } else {
        Serial.println("SIM_PWR_PIN is not configured.");
    }
}

// Helper to send AT command and print output to Serial
void sendATCommand(const char* cmd, uint32_t timeout) {
    simSerial.println(cmd);
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while (simSerial.available()) {
            char c = simSerial.read();
            Serial.write(c);
        }
        delay(10);
    }
}

// Function to send SMS
void sendSMS(const char* phoneNumber, const char* message) {
    Serial.printf("Sending SMS to: %s...\n", phoneNumber);
    
    // Set SMS mode to text
    simSerial.println("AT+CMGF=1"); 
    delay(500);
    
    // Set destination phone number
    simSerial.printf("AT+CMGS=\"%s\"\r\n", phoneNumber);
    delay(500);
    
    // Send the message payload
    simSerial.print(message);
    delay(500);
    
    // Send the Ctrl+Z character (ASCII 26) to finish sending SMS
    simSerial.write(26); 
    
    Serial.println("SMS Command sent. Awaiting response from module...");
}

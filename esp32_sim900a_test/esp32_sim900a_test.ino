/**
 * @file esp32_sim900a_test.ino
 * @brief Standalone SIM900A GSM Module Test Sketch for ESP32 with Interactive Serial SMS interface.
 * 
 * Target Board: ESP32 DevKit V1 (38-pin variant)
 * 
 * Hardware Connections:
 * - SIM900A TXD -> ESP32 GPIO 16 (Hardware Serial 2 RX)
 * - SIM900A RXD -> ESP32 GPIO 17 (Hardware Serial 2 TX)
 * - SIM900A PWR -> ESP32 GPIO 4  (Optional Power Key trigger)
 * - SIM900A GND -> ESP32 GND     (CRITICAL: Share ground!)
 * - SIM900A VCC MCU -> ESP32 3.3V (Powers the logic level shifter on red header)
 * 
 * ⚠️ POWER WARNING:
 * The SIM900A requires an external power supply (typically 5V, 2A). 
 * Do NOT power the SIM900A directly from the ESP32's 3.3V or 5V/VIN pin.
 */

#include <Arduino.h>

#define SIM_RX_PIN  16   // RX2 (Connects to SIM900A TXD 3.3V)
#define SIM_TX_PIN  17   // TX2 (Connects to SIM900A RXD 3.3V)
#define SIM_PWR_PIN 4    // Connects to SIM900A PWRKEY / KEY pin (Optional)

// Setup HardwareSerial 2
HardwareSerial simSerial(2);

// Interactive CLI States
enum CLIState {
    STATE_PASSTHROUGH,
    STATE_AWAIT_PHONE,
    STATE_AWAIT_MSG
};

CLIState currentState = STATE_PASSTHROUGH;
String recipientPhone = "";
String smsMessage = "";

// Function prototypes
void pulsePowerKey();
void sendATCommand(const char* cmd, uint32_t timeout);
void sendSMS(const char* phoneNumber, const char* message);

void setup() {
    // USB Serial Monitor
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n==================================================");
    Serial.println("      ESP32 SIM900A GSM Module Interactive Test   ");
    Serial.println("==================================================");

    // Initialize SIM900A PWR pin
    if (SIM_PWR_PIN >= 0) {
        pinMode(SIM_PWR_PIN, OUTPUT);
        digitalWrite(SIM_PWR_PIN, HIGH); // Standard idle high
    }

    // SIM900A usually defaults to 9600 baud (or auto-baud)
    Serial.println("Initializing Hardware Serial 2 (UART) at 9600 baud...");
    simSerial.begin(9600, SERIAL_8N1, SIM_RX_PIN, SIM_TX_PIN);
    delay(1000);

    Serial.println("\n--- Step 1: Checking Device Status & Connectivity ---");
    Serial.print("Checking AT connection... ");
    sendATCommand("AT", 1000);
    
    Serial.print("Checking Module Info (ATI)... ");
    sendATCommand("ATI", 1000);

    Serial.print("Checking SIM Card Status (AT+CPIN?)... ");
    sendATCommand("AT+CPIN?", 1000);

    Serial.print("Checking Signal Strength (AT+CSQ)... ");
    sendATCommand("AT+CSQ", 1000);

    Serial.print("Checking Network Registration (AT+CREG?)... ");
    sendATCommand("AT+CREG?", 1000);

    Serial.print("Checking Network Operator (AT+COPS?)... ");
    sendATCommand("AT+COPS?", 1000);
    Serial.println("------------------------------------------------------\n");

    Serial.println("💡 Serial Passthrough Mode is active.");
    Serial.println("💡 Options:");
    Serial.println("   - Type AT commands directly to query the modem.");
    Serial.println("   - Type 'SEND' to run the interactive SMS sending wizard.");
    Serial.println("   - Type 'PWR_KEY' to toggle the PWRKEY pin.");
    Serial.println("------------------------------------------------------\n");
}

void loop() {
    // Read from SIM900A and display to user
    if (simSerial.available()) {
        while (simSerial.available()) {
            char c = simSerial.read();
            Serial.write(c);
        }
    }

    // Read from Serial Monitor
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.length() == 0) return;

        switch (currentState) {
            case STATE_PASSTHROUGH:
                if (input == "SEND") {
                    currentState = STATE_AWAIT_PHONE;
                    Serial.println("\n--- [SMS WIZARD: STEP 1] ---");
                    Serial.println("Please enter the receiver's phone number (in international format, e.g. +88017XXXXXXXX):");
                    Serial.print("> ");
                } else if (input == "PWR_KEY") {
                    pulsePowerKey();
                } else {
                    // Send normal AT command
                    simSerial.println(input);
                }
                break;

            case STATE_AWAIT_PHONE:
                recipientPhone = input;
                currentState = STATE_AWAIT_MSG;
                Serial.printf("\nPhone number set to: %s\n", recipientPhone.c_str());
                Serial.println("--- [SMS WIZARD: STEP 2] ---");
                Serial.println("Please enter the message you want to send:");
                Serial.print("> ");
                break;

            case STATE_AWAIT_MSG:
                smsMessage = input;
                Serial.printf("\nMessage set to: \"%s\"\n", smsMessage.c_str());
                Serial.println("Sending SMS...");
                
                sendSMS(recipientPhone.c_str(), smsMessage.c_str());
                
                // Clear and reset state
                recipientPhone = "";
                smsMessage = "";
                currentState = STATE_PASSTHROUGH;
                Serial.println("\nSMS Wizard completed. Returned to Serial Passthrough mode.");
                Serial.println("Type 'SEND' to send another message.");
                Serial.println("------------------------------------------------------\n");
                break;
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
    
    // Await response for up to 5 seconds
    unsigned long start = millis();
    while (millis() - start < 5000) {
        while (simSerial.available()) {
            char c = simSerial.read();
            Serial.write(c);
        }
        delay(10);
    }
    Serial.println("\nSMS trigger completed.");
}

/**
 * @file esp32_blink_test.ino
 * @brief Simple LED Blink Test for ESP32 DevKit V1.
 * 
 * Target Board: ESP32 DevKit V1 (30-pin or 36-pin)
 * On-Board LED Pin: GPIO 2 (Standard Blue LED)
 */

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==============================================");
    Serial.println("         ESP32 Simple Blink Test Start        ");
    Serial.println("==============================================");
    Serial.printf("Target pin: GPIO %d (Built-in LED)\n", LED_BUILTIN);
    
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    Serial.println("LED ON");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    
    Serial.println("LED OFF");
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}

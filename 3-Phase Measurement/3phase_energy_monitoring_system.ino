#define BLYNK_TEMPLATE_ID "TMPL3LPLsOXj2"
#define BLYNK_TEMPLATE_NAME "energy monitor"

#include <ZMPT101B.h>
#include <EmonLib.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Configuration
#define CURRENT_THRESHOLD 0.30  // Set your alert threshold (in Amps)
#define NOTIFICATION_INTERVAL 30000  // 30 seconds between alerts (avoid spamming)


// Blynk & WiFi Setup
char ssid[] = "GalaxyA1";
char pass[] = "12345611";
char auth[] = "_o_SE-rJLgwAPVz95U-mV0VkOZwyqTFM";

// Voltage Sensor Calibrations
#define SENSITIVITY_1 180.5f
#define SENSITIVITY_2 205.0f
#define SENSITIVITY_3 200.50f

// Pin Definitions
#define VOLTAGE_PIN_1 A0
#define VOLTAGE_PIN_2 A1
#define VOLTAGE_PIN_3 A2
#define CURRENT_PIN_1 A8
#define CURRENT_PIN_2 A9
#define CURRENT_PIN_3 A10

// Blynk Virtual Pins
#define V_VOLTAGE_1 V0
#define V_VOLTAGE_2 V1
#define V_VOLTAGE_3 V2
#define V_CURRENT_1 V3
#define V_CURRENT_2 V4
#define V_CURRENT_3 V5
#define V_POWER_1 V6
#define V_POWER_2 V7
#define V_POWER_3 V8

// Sensor Objects
ZMPT101B voltageSensor1(VOLTAGE_PIN_1, 50.0);
ZMPT101B voltageSensor2(VOLTAGE_PIN_2, 50.0);
ZMPT101B voltageSensor3(VOLTAGE_PIN_3, 50.0);
EnergyMonitor emon1, emon2, emon3;

// Alert tracking
unsigned long lastAlertTime = 0;

void checkCurrentAlert(float current, int phase) {
  if (current > CURRENT_THRESHOLD && millis() - lastAlertTime > NOTIFICATION_INTERVAL) {
    String message = "⚠️ Overcurrent! Phase " + String(phase) + ": " + String(current, 2) + "A";
    Blynk.logEvent("overcurrent", message);  // Requires Blynk 2.0+
    // Alternative for older Blynk:
    // Blynk.notify(message);
    
    lastAlertTime = millis();
    Serial.println("Alert sent: " + message);
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initialize sensors
  voltageSensor1.setSensitivity(SENSITIVITY_1);
  voltageSensor2.setSensitivity(SENSITIVITY_2);
  voltageSensor3.setSensitivity(SENSITIVITY_3);
  
  emon1.current(CURRENT_PIN_1, 0.6);
  emon2.current(CURRENT_PIN_2, 0.12);
  emon3.current(CURRENT_PIN_3, 0.12);

  // Connect to WiFi and Blynk
  WiFi.begin(ssid, pass);
  Blynk.config(auth);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Blynk.run();
  } else {
    WiFi.reconnect();
  }

  // Read sensors
  float v1 = voltageSensor1.getRmsVoltage();
  float v2 = voltageSensor2.getRmsVoltage();
  float v3 = voltageSensor3.getRmsVoltage();
  float i1 = emon1.calcIrms(1480);
  float i2 = emon2.calcIrms(1480);
  float i3 = emon3.calcIrms(1480);

  // Check alerts
  checkCurrentAlert(i1, 1);
  checkCurrentAlert(i2, 2);
  checkCurrentAlert(i3, 3);

  // Send to Blynk
  Blynk.virtualWrite(V_VOLTAGE_1, v1);
  Blynk.virtualWrite(V_VOLTAGE_2, v2);
  Blynk.virtualWrite(V_VOLTAGE_3, v3);
  Blynk.virtualWrite(V_CURRENT_1, i1);
  Blynk.virtualWrite(V_CURRENT_2, i2);
  Blynk.virtualWrite(V_CURRENT_3, i3);
  Blynk.virtualWrite(V_POWER_1, v1 * i1);
  Blynk.virtualWrite(V_POWER_2, v2 * i2);
  Blynk.virtualWrite(V_POWER_3, v3 * i3);

  // Serial Monitor Output
  Serial.printf("[Phase 1] %.1fV | %.3fA | %.1fW\n", v1, i1, v1*i1);
  Serial.printf("[Phase 2] %.1fV | %.3fA | %.1fW\n", v2, i2, v2*i2);
  Serial.printf("[Phase 3] %.1fV | %.3fA | %.1fW\n", v3, i3, v3*i3);
  Serial.println("-----------------------");

  delay(1000);  // Adjust based on your Blynk plan limits
}
/*
  ESP32 Diode Curve Tracer — Forward I-V Sweep
  -----------------------------------------------
  Hardware:
    MCP4725 DAC   -> sets forced voltage, I2C addr 0x60
    ADS1115 ADC   -> reads two nodes, I2C addr 0x48
    R_SENSE       -> series resistor between DAC output and DUT anode
    DUT (1N4148)  -> anode at node V2, cathode to GND

  Circuit:
    DAC_OUT --(AIN0=V1)--[ R_SENSE ]--(AIN1=V2)-- Diode anode
                                                    Diode cathode -- GND

  Current  = (V1 - V2) / R_SENSE
  V_diode  = V2

  Serial protocol:
    Host sends "RUN\n"
    Device streams:
      BEGIN
      dac_code,v_forced,v_diode,current_mA
      <data rows...>
      END
*/

#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <Adafruit_ADS1X15.h>

Adafruit_MCP4725 dac;
Adafruit_ADS1115 ads;

// ---- Configuration ----
const float R_SENSE = 1000;     //in ohms
const int   DAC_STEP = 8;       // DAC code step size, 0-4095 (smaller = more points, slower)
const int   SETTLE_MS = 3;      // settle time after each DAC write, ms
const int   SAMPLES_TO_AVG = 4; // ADC oversampling per reading (bigger = less noise, slower)

// Confirm GPIO pins on ESP32
const int SDA_PIN = 21;
const int SCL_PIN = 22;

float readAvgVolts(uint8_t channel) {
  long sum = 0;
  for (int i = 0; i < SAMPLES_TO_AVG; i++) {
    sum += ads.readADC_SingleEnded(channel);
  }
  int16_t raw = sum / SAMPLES_TO_AVG;
  return ads.computeVolts(raw);
}

void runSweep() {
  Serial.println("BEGIN");
  Serial.println("dac_code,v_forced,v_diode,current_mA");

  for (int code = 0; code <= 4095; code += DAC_STEP) {
    dac.setVoltage(code, false);
    delay(SETTLE_MS);

    float v1 = readAvgVolts(0); // node before resistor (DAC output)
    float v2 = readAvgVolts(1); // node after resistor (diode anode)
    // (I = V / R) * 1000 for milliamps
    float current_mA = (v1 - v2) / R_SENSE * 1000.0;

    Serial.print(code);
    Serial.print(",");
    Serial.print(v1, 4);
    Serial.print(",");
    Serial.print(v2, 4);
    Serial.print(",");
    Serial.println(current_mA, 4);
  }

  dac.setVoltage(0, false); // return DAC to 0V for safety
  Serial.println("END");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  if (!dac.begin(0x60)) {
    Serial.println("ERROR: MCP4725 not found at 0x60");
    while (1) delay(1000);
  }

  if (!ads.begin(0x48)) {
    Serial.println("ERROR: ADS1115 not found at 0x48");
    while (1) delay(1000);
  }

  ads.setGain(GAIN_ONE); // +/-4.096V range — matches 3.3V DAC swing

  dac.setVoltage(0, false);
  Serial.println("READY");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "RUN") {
      runSweep();
    }
  }
}

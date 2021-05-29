// This program will:
// At start confirm BME280 presence and perform 10 consecutive readings in 5 seconds
// Confirm presence of RTC and Charger chip
// Turn on RPI
// Show Lora triggers sent from modified Bushwell camera
// Show logic level of the status line
// Echo back UART from RPi
// Print button presses

#include <LibLacuna.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

lsLoraWANParams relay_loraWANParams;
lsLoraTxParams relayParams;

void setup_lacuna(void)
{
    // Keys where we will be receiving relay
    uint8_t relay_networkKey[] = { 0xF3, 0x2C, 0x89, 0xCB, 0xFC, 0xD3, 0x82, 0x23, 0x6E, 0x73, 0x74, 0xAC, 0x6A, 0xCF, 0x10, 0x04 };
    uint8_t relay_appKey[] = { 0xBF, 0x5E, 0xE6, 0x81, 0x8B, 0x12, 0x3E, 0x78, 0x3F, 0x57, 0x36, 0xF8, 0x18, 0x3E, 0x95, 0xC5 };
    uint8_t relay_deviceAddress[] = { 0x26, 0x01, 0x12, 0xAB };

    // SX1262 configuration for lacuna LS200 board
    lsSX126xConfig cfg;
    lsCreateDefaultSX126xConfig(&cfg);

    cfg.nssPin = PB12;
    cfg.resetPin = -1;           // Not needed
    cfg.antennaSwitchPin = PB2;
    cfg.busyPin = PH1;             
    cfg.dio1Pin = PB7;             
    cfg.osc = lsSX126xOscTCXO;      // for Xtal: lsSX126xOscXtal
    cfg.type = lsSX126xTypeSX1262;  // for SX1261: lsSX126xTypeSX1261

    // Initialize SX1262
    int result = lsInitSX126x(&cfg);

    Serial.print("E22/SX1262: ");
    Serial.println(lsErrorToString(result));

    // LoRaWAN parameters for relay
    lsCreateDefaultLoraWANParams(&relay_loraWANParams, 
                                 relay_networkKey, 
                                 relay_appKey, 
                                 relay_deviceAddress);

    // LoRa parameters for relay (receive) 
    lsCreateDefaultLoraTxParams(&relayParams);

    // Make sure that SF and bandwith match with device that we will relay.
    relayParams.spreadingFactor = lsLoraSpreadingFactor_9;
    relayParams.invertIq = false;
    relayParams.frequency = 868500000;
    relayParams.bandwidth = lsLoraBandwidth_125_khz;
    relayParams.syncWord = LS_LORA_SYNCWORD_PUBLIC;
}

byte global_relay_payload[255];
void receive_lacuna() {
  int32_t rxlength =
      lsReceiveLora(&relay_loraWANParams, &relayParams, global_relay_payload);

  if (rxlength) {
    /* valid relay data received */
    Serial.println("Valid uplink received!)");
    Serial.print("        SNR: ");
    Serial.println(relayParams.snr);
    Serial.print("        RSSI: ");
    Serial.println(relayParams.rssi);
    Serial.print("        SignalRSSI: ");
    Serial.println(relayParams.signalrssi);
    Serial.print("        Payload in hex: ");

    for (uint8_t n = 0; n < rxlength; n++) {
      Serial.print(global_relay_payload[n], HEX);
      Serial.write(" ");
    }

    uint8_t *device_id = getDeviceId();

    Serial.print("\n        Device ID in hex: ");
    for (uint8_t n = 0; n < 4; n++) {
      Serial.print(device_id[n], HEX);
      Serial.write(" ");
    }
    Serial.println();
  }
}

uint8_t lacuna_power_en = PH0;
uint8_t rpi_power_en = PB6;
uint8_t rpi_status = PA11;
uint8_t buttons = PA5;

Adafruit_BME280 bme; // comm over I2C

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);


    pinMode(lacuna_power_en, OUTPUT);
    pinMode(rpi_power_en, OUTPUT);
    pinMode(rpi_status, INPUT);
    pinMode(buttons, INPUT);

    digitalWrite(lacuna_power_en, HIGH);
    digitalWrite(rpi_power_en, HIGH);

    delay(1000);
    byte error;
    Wire.begin();
    Wire.beginTransmission(0x6F);
    error = Wire.endTransmission();
 
    setup_lacuna();

    if (error == 0)
    {
      Serial.println("RTC chip: OK");
    }
    else  {
      Serial.println("ERROR: RTC chip was not found at address 0x6F");
    }

    Wire.beginTransmission(0x77);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.println("BME chip: OK");
    }
    else  {
      Serial.println("ERROR: BME chip was not found at address 0x77");
    }

    Wire.beginTransmission(0x6B);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.println("Charger chip: OK");
    }
    else  {
      Serial.println("ERROR: Charger chip was not found at address 0x6B");
    }

    uint8_t status = bme.begin();  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        while (1) delay(10);
    }

    for (int i = 0; i < 10; i++) {
      Serial.print("Temperature = ");
      Serial.print(bme.readTemperature());
      Serial.println(" *C");
      delay(1000);
    }
}

static uint8_t status_value;
static bool status_state = false;
static byte incomingByte;

void loop()
{

    receive_lacuna();

    // Echo back what you get from RPi
    if (Serial1.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial1.read();

      Serial.print("Murata received: ");
      Serial.println((char)incomingByte);

      // say what you got:
      Serial1.print("Murata received: ");
      Serial1.println((char)incomingByte);
    }

    if (!digitalRead(buttons)) {
        Serial.println("One of buttons is pressed");
        delay(1000);
    }

    status_value = digitalRead(rpi_status);
    if (status_value) {
        if (!status_state) {
            Serial.println("Rpi status went high");
            status_state = true;
        }
    }
    else {
        if (status_state) {
            Serial.println("Rpi status went low");
            status_state = false;
        }
    }
}

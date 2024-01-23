// Includes
#include "multiplexer.hpp"
#include <WiFi.h>
#include <mqtt_client.h>

// Types
struct MultiplexerIndexes {
    const uint8_t* mainIndex;
    const uint8_t* subIndex;
};

// Variables
const uint8_t lanes = 40;
const uint8_t size = 8;
const uint8_t amount = 3;
uint16_t matrix[lanes * lanes] = {};

const uint8_t outputMultiplexerPins[amount] = {33, 12, 16};
const uint8_t outputMultiplexerConnectPin = 14;
const uint8_t outputSubMultiplexerPins[amount] = {27, 26, 32};
const uint8_t outputSubMultiplexerConnectPin = 0;

const uint8_t inputMultiplexerPins[amount] = {21, 19, 18};
const uint8_t inputMultiplexerConnectPin = 36; // 15;
const uint8_t inputSubMultiplexerPins[amount] = {5, 4, 2};
const uint8_t inputSubMultiplexerConnectPin = 0;

const uint8_t channels[size][amount] = {
        {0, 0, 0},
        {0, 0, 1},
        {0, 1, 0},
        {0, 1, 1},
        {1, 0, 0},
        {1, 0, 1},
        {1, 1, 0},
        {1, 1, 1}
};

Multiplexer outputMultiplexer(amount, OUTPUT, outputMultiplexerPins, outputMultiplexerConnectPin);
Multiplexer outputSubMultiplexer(amount, OUTPUT, outputSubMultiplexerPins, outputSubMultiplexerConnectPin);
Multiplexer inputMultiplexer(amount, INPUT, inputMultiplexerPins, inputMultiplexerConnectPin);
Multiplexer inputSubMultiplexer(amount, INPUT, inputSubMultiplexerPins, inputSubMultiplexerConnectPin);

const esp_mqtt_client_config_t mqttConf = {
        .host = "172.20.10.9",
        .port = 1883,
        .client_id = "esp-wroom-32",
        .buffer_size = 1600
};
esp_mqtt_client_handle_t mqttClient;

// Methods
MultiplexerIndexes getMultiplexerIndexes(const uint8_t& count) {
    const auto multiplexerIndex = (uint8_t)((count + 1) / size);
    const uint8_t subMultiplexerIndex = count - (multiplexerIndex * size);
    return {channels[multiplexerIndex], channels[subMultiplexerIndex]};
}

void updateMatrix() {
    for (uint8_t i = 0; i < lanes; ++i) {
        const MultiplexerIndexes outputMultiplexerIndexes = getMultiplexerIndexes(i);

        outputMultiplexer.select(outputMultiplexerIndexes.mainIndex);
        outputSubMultiplexer.select(outputMultiplexerIndexes.subIndex);

        for (uint8_t j = 0; j < lanes; ++j) {
            const MultiplexerIndexes inputMultiplexerIndexes = getMultiplexerIndexes(j);

            inputSubMultiplexer.select(inputMultiplexerIndexes.subIndex);
            matrix[(j * lanes) + i] = inputMultiplexer.read(inputMultiplexerIndexes.mainIndex);
        }
    }
}

void setupWiFi() {
    Serial.println("Starting 3 second grace period...");
    delay(3000);
    Serial.println("Finished 3 second grace period.");

    Serial.println("Setting up WiFi...");
    WiFi.begin("Stargate Command", "IFnq-5OB4-sMyS-QY29");

    while (WiFiClass::status() != WL_CONNECTED) {
        Serial.println("Trying to connect...");
        delay(1000);
    }
    Serial.println("Successfully set up WiFi!");
}

void setupMQTT() {
    Serial.println("Setting up MQTT...");
    mqttClient = esp_mqtt_client_init(&mqttConf);
    esp_mqtt_client_start(mqttClient);
    Serial.println("Successfully set up MQTT!");
}

void setupMultiplexers() {
    Serial.println("Setting up Multiplexers...");
    outputMultiplexer.powerOn();
    Serial.println("Successfully set up Multiplexers!");
}

void setup() {
    Serial.begin(921600);
    setupWiFi();
    setupMQTT();
    setupMultiplexers();
    Serial.println("Setup done, beginning runtime cycle.");
}

void loop() {
    updateMatrix();
    esp_mqtt_client_publish(
            mqttClient,
            "stressTest",
            (const char*)matrix,
            sizeof(matrix),
            0,
            0
    );
}
// Includes
#include "multiplexer.hpp"
#include "utilities.hpp"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/adc.h>

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

WiFiUDP udpClient;
adc_digi_configuration_t adc1Config = {
        .sample_freq_hz = 83333
};

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

void setup() {
    Serial.begin(921600);

    adc_digi_controller_configure(&adc1Config);
    adc1_config_width(ADC_WIDTH_BIT_9);
    delay(3000);

    WiFi.begin("", "");
    while (WiFiClass::status() != WL_CONNECTED) delay(1000);

    outputMultiplexer.powerOn();
}

void loop() {
    updateMatrix();

    udpClient.beginPacket("", 8888);
    for (uint16_t value : matrix)
        for (char i : integral_to_bytes<uint16_t>(value))
            udpClient.write(i);
    udpClient.endPacket();
}

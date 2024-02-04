#ifndef PRESSUREPLATE_MULTIPLEXER_HPP
#define PRESSUREPLATE_MULTIPLEXER_HPP

// Includes
#include <cstdint>
#include <Arduino.h>
#include <driver/adc.h>

// Class
class Multiplexer {
private:
    const uint8_t _size;
    const uint8_t* _channelPins;
    const uint8_t _connectPin;

public:
    Multiplexer(const uint8_t& size, const uint8_t& mode, const uint8_t* channelPins, const uint8_t& connectPin)
            : _size(size), _channelPins(channelPins), _connectPin(connectPin) {
        for (uint8_t i = 0; i < _size; ++i)
            pinMode(_channelPins[i], mode);

        pinMode(_connectPin, mode);
    };

    void select(const uint8_t* channelSelection) {
        for (uint8_t i = 0; i < _size; ++i)
            digitalWrite(_channelPins[i], channelSelection[i]);
    };

    uint16_t read(const uint8_t* channelSelection) {
        select(channelSelection);
        return (uint16_t) adc1_get_raw(ADC1_CHANNEL_0);
    };

    void powerOn() {
        digitalWrite(_connectPin, HIGH);
    };

    void powerOff() {
        digitalWrite(_connectPin, LOW);
    }
};

#endif

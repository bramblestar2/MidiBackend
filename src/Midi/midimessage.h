#pragma once
#include <cstdint>


struct MidiMessage {
    short int status;
    short int key;
    short int velocity;
    double timestamp;

    enum Type {
        UNKNOWN = 0x00,
        NoteOff = 0x80,
        NoteOn = 0x90,
    };

    int8_t channel() noexcept;
    Type type() noexcept;
};
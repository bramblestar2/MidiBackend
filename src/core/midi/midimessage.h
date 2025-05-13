#pragma once
#include <cstdint>


struct MidiMessage {
    int status;
    int key;
    int velocity;

    enum Type {
        UNKNOWN = 0x00,
        NoteOff = 0x80,
        NoteOn = 0x90,
    };

    int8_t channel() noexcept;
    Type type() noexcept;
};
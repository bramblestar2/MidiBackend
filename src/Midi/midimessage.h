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
        PolyphonicKeyPressure = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelAftertouch = 0xD0,
        PitchBend = 0xE0
    };

    int8_t channel() noexcept;
    Type type() noexcept;
};
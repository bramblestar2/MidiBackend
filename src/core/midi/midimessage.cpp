#include "midimessage.h"

int8_t MidiMessage::channel() noexcept {
    // Channel statuses start on 0x80 (128) and end on 0xEF (239)
    // The event type with the channels change every 0x0F (15)
    // 0x80 -> 0x8F / 0x90 -> 0x9F / 0xA0 -> 0xAF / Etc..

    constexpr int8_t increment = 0x10;
    constexpr int8_t initial = 0x80;

    if (this->status >= 0x80 && this->status <= 0xEF) {
        const int8_t channel = this->status % increment + 1;

        return channel; // Minimum Channel 1
    }

    return -1;
}


MidiMessage::Type MidiMessage::type() noexcept {

    if (this->status >= Type::NoteOff && this->status < Type::NoteOn) {
        return Type::NoteOff;
    } else {
        return Type::NoteOn;
    }

    return Type::UNKNOWN;
}
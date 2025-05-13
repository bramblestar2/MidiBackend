#include "midimessage.h"

int8_t MidiMessage::channel() noexcept {
    // Channel statuses start on 0x80 (128) and end on 0xEF (239)
    // The event type with the channels change every 0x0F (15)
    // 0x80 -> 0x8F / 0x90 -> 0x9F / 0xA0 -> 0xAF / Etc..
    
    if (this->status >= 0x80 && this->status <= 0xEF) {
        return this->status & 0x0F + 1;
    }

    return -1;
}


MidiMessage::Type MidiMessage::type() noexcept {
    switch (this->status & 0xF0) {
        case Type::NoteOff:
            return Type::NoteOff;
        case Type::NoteOn:
            return Type::NoteOn;
        default:
            return Type::UNKNOWN;
    }
}

#pragma once
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>

class MidiDevice {
public:
    struct PortPair {
        std::shared_ptr<RtMidiIn> portIn;
        std::shared_ptr<RtMidiOut> portOut;
    };

    MidiDevice();

    std::vector<unsigned char> identity();
};
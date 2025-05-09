#pragma once
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include <string>

class MidiDevice {
private:
    std::vector<unsigned char> m_identity;
    std::string m_name;

public:
    struct PortPair {
        std::shared_ptr<RtMidiIn> portIn;
        std::shared_ptr<RtMidiOut> portOut;
    };

    MidiDevice(std::string deviceName, std::vector<unsigned char> identity);

    const std::vector<unsigned char>& identity() const { return m_identity; }
    const std::string& name() const { return m_name; }
};
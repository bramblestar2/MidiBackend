#pragma once
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>

class MidiManager {
public:
    struct PortPair {
        std::shared_ptr<RtMidiIn> portIn;
        std::shared_ptr<RtMidiOut> portOut;
    };

private:
    MidiManager();
    
    std::vector<PortPair> m_ports;
    std::vector<PortPair> m_availableDevices;

    std::vector<PortPair> getPortPairs();

    bool verifyIdentity(unsigned int inPort,
                        const std::vector<unsigned char>& targetId);
public:

};
#pragma once
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include "mididevice.h"

class MidiManager {
private:
    MidiManager();
    
    std::vector<MidiDevice::PortPair> m_ports;
    std::vector<MidiDevice::PortPair> m_availableDevices;

    std::vector<MidiDevice::PortPair> getPortPairs();

    bool verifyIdentity(unsigned int inPort,
                        const std::vector<unsigned char>& targetId);

    RtMidiIn _midiin;
    RtMidiOut _midiout;
public:
    void refresh();

    const std::vector<MidiDevice::PortPair>& getAvailableDevices() const { return m_availableDevices; }
};
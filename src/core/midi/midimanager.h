#pragma once
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include "mididevice.h"

class MidiManager {
private:

    std::vector<std::pair<int, int>> m_ports;
    std::vector<std::shared_ptr<MidiDevice>> m_availableDevices;
    
    bool portsMatch(std::string in, std::string out);
    bool verifyIdentity(unsigned int inPort, unsigned int outPort,
        const std::vector<unsigned char>& targetId);
    
    void identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);

    RtMidiIn _midiin;
    RtMidiOut _midiout;
public:
    MidiManager();
    ~MidiManager();
    
    void refresh();

    std::vector<std::pair<int, int>> getPortPairs() const { return m_ports; }
    const std::vector<std::shared_ptr<MidiDevice>>& getAvailableDevices() const { return m_availableDevices; }
};
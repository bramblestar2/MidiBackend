#pragma once
#include <chrono>
#include <vector>
#include <rtmidi/RtMidi.h>

#include "mididevice.h"

#include <memory>
#include <string> 


namespace std {
    template<>
    struct hash<std::vector<unsigned char>> {
        std::size_t operator()(const std::vector<unsigned char> &v) const noexcept {
            size_t h = 0;
            for (auto byte : v) {
                h = h * 31 + byte;
            }
            return h;
        }
    };
}


class MidiManager {
private:

    struct ValidationSession {
        ~ValidationSession() { 
            midiIn->cancelCallback(); 
            midiIn->closePort(); 
            midiOut->closePort(); 
        }

        std::shared_ptr<RtMidiIn> midiIn;
        std::shared_ptr<RtMidiOut> midiOut;
        std::vector<unsigned char> targetId;
        std::chrono::steady_clock::time_point startTime;
        unsigned int inPort;
        unsigned int outPort;
        MidiManager* manager;
    };

    std::vector<std::pair<int, int>> m_ports;
    std::vector<std::shared_ptr<MidiDevice>> m_devices;

    bool portsMatch(std::string in, std::string out);
    bool verifyIdentity(unsigned int inPort, unsigned int outPort,
        const std::vector<unsigned char>& targetId);

    RtMidiIn _midiin;
    RtMidiOut _midiout;
public:
    MidiManager();
    ~MidiManager();
    
    void refresh();

    std::vector<std::pair<int, int>> getPortPairs() const { return m_ports; }
    const std::vector<std::shared_ptr<MidiDevice>>& getAvailableDevices() const { return m_devices; }
};
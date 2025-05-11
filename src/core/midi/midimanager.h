#pragma once
#include <chrono>
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include "mididevice.h"

class MidiManager {
private:

    struct ValidationSession {
        std::shared_ptr<RtMidiIn> midiIn;
        std::shared_ptr<RtMidiOut> midiOut;
        std::vector<unsigned char> targetId;
        std::chrono::steady_clock::time_point startTime;
        unsigned int inPort;
        unsigned int outPort;
        MidiManager* manager;
    };

    std::vector<std::pair<int, int>> m_ports;
    std::vector<std::shared_ptr<MidiDevice>> m_availableDevices;
    std::vector<std::unique_ptr<ValidationSession>> m_activeSessions;
    
    bool portsMatch(std::string in, std::string out);
    bool verifyIdentity(unsigned int inPort, unsigned int outPort,
        const std::vector<unsigned char>& targetId);
    
    static void identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);
    void handleIdentityResponse(ValidationSession* session, std::vector<unsigned char> *message);
    void cleanupSession(ValidationSession* session);

    RtMidiIn _midiin;
    RtMidiOut _midiout;
public:
    MidiManager();
    ~MidiManager();
    
    void refresh();
    void checkTimeouts();

    std::vector<std::pair<int, int>> getPortPairs() const { return m_ports; }
    const std::vector<std::shared_ptr<MidiDevice>>& getAvailableDevices() const { return m_availableDevices; }
};
#pragma once
#include <chrono>
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include <string> 
#include <functional>
#include <optional>

#include "mididevice.h"
#include "midimessage.h"



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

    std::vector<std::shared_ptr<MidiDevice>> m_devices;
    
    RtMidiIn m_midiin;
    RtMidiOut m_midiout;

    std::function<void(MidiDevice*, MidiMessage)> m_midiCallback;

    std::function<void()> m_devicesRefreshCallback;

    bool portsMatch(std::string in, std::string out);
    void verifyIdentity(unsigned int inPort, unsigned int outPort,
        const std::vector<unsigned char>& targetId);

    void setupDeviceCallbacks();

public:
    MidiManager();
    ~MidiManager();
    
    void refresh();

    std::vector<MidiDevice*> getDevices() const;
    std::vector<MidiDevice*> getAvailableDevices() const;

    void setMidiCallback(std::function<void(MidiDevice*, MidiMessage)> function);
    void setDevicesRefreshCallback(std::function<void()> &&callback);
};
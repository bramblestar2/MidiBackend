#pragma once
#include <unordered_map>
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <functional>
#include <optional>

#include "midimessage.h"


class MidiDevice {
public:
    enum Availability {
        UNAVAILABLE = 0x00,
        VERIFYING = 0x01,
        AVAILABLE = 0x02
    };

private:
    std::mutex m_mutex;

    std::vector<unsigned char> m_identity;
    std::string m_name;

    std::atomic<Availability> m_available = Availability::UNAVAILABLE;

    std::unique_ptr<RtMidiIn> m_midiIn;
    std::unique_ptr<RtMidiOut> m_midiOut;

    int m_inPort;
    int m_outPort;

    std::mutex m_recordingMutex;
    bool m_isRecording = false;
    std::vector<MidiMessage> m_recording;

    std::function<void(MidiMessage)> m_keyCallback;
    std::function<void()> m_verifyCallback;

    void attemptVerify();

    static void identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);
    static void midiCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);
    void handleIdentityResponse(std::vector<unsigned char> *message);
    void handleButtonResponse(double time, std::vector<unsigned char> *message);

public:
    // Upon creation of midi device, it will attempt to verify the 
    // type of midi device it is to see if it is supported.
    // If it is verified, is_available() will return true
    MidiDevice(const int& inPort, const int& outPort);
    ~MidiDevice(); // Clean midi ports

    void verify();
    void open(const int& inPort, const int& outPort);
    void close();
    const Availability is_available() const noexcept { return m_available; }
    const std::vector<unsigned char>& identity() const { return m_identity; }
    const std::string& name() const;

    void setKeyCallback(std::function<void(MidiMessage)> callback);
    void setVerifyCallback(std::function<void()> callback);

    void startRecording();
    void stopRecording();

    const std::vector<MidiMessage>& getRecording() const { return m_recording; }
};


struct DeviceIdentifier {
    std::string manufacturerId;
    std::vector<unsigned char> deviceCode;
};


namespace MidiDeviceDB {
    const std::unordered_map<std::string, DeviceIdentifier> KNOWN_DEVICES = {
        { "Novation Launchpad Pro", {{0x00, 0x20, 0x29}, { 0x51 }}}
    };
}
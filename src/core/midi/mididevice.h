#pragma once
#include <unordered_map>
#include <vector>
#include <rtmidi/RtMidi.h>
#include <memory>
#include <string>
#include <unordered_map>

// class MidiDevice {
// private:
//     std::vector<unsigned char> m_identity;
//     std::string m_name;

// public:
//     struct PortPair {
//         std::shared_ptr<RtMidiIn> portIn;
//         std::shared_ptr<RtMidiOut> portOut;
//     };

//     MidiDevice(std::string deviceName, std::vector<unsigned char> identity);

//     const std::vector<unsigned char>& identity() const { return m_identity; }
//     const std::string& name() const { return m_name; }
// };


class MidiDevice {
private:
    std::vector<unsigned char> m_identity;
    std::string m_name;

    bool m_available = false;

    std::unique_ptr<RtMidiIn> m_portIn;
    std::unique_ptr<RtMidiOut> m_portOut;

    int m_inPort;
    int m_outPort;

    void attemptVerify();

    static void identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData);
    void handleIdentityResponse(std::vector<unsigned char> *message);

public:
    // Upon creation of midi device, it will attempt to verify the 
    // type of midi device it is to see if it is supported.
    // If it is verified, is_available() will return true
    MidiDevice(const int& inPort, const int& outPort);
    ~MidiDevice(); // Clean midi ports

    void verify();
    void open(const int& inPort, const int& outPort);
    void close();
    const bool& is_available() const { return m_available; }
    const std::vector<unsigned char>& identity() const { return m_identity; }
    const std::string& name() const { return m_name; }
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
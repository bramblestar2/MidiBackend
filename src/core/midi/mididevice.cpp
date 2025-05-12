#include "mididevice.h"
#include <rtmidi/RtMidi.h>

MidiDevice::MidiDevice(const int& inPort, const int& outPort) {
    this->open(inPort, outPort);
}


MidiDevice::~MidiDevice() {
    this->close();
}


void MidiDevice::open(const int& inPort, const int& outPort) {
    try {
        this->close();

        m_portIn.reset(new RtMidiIn);
        m_portOut.reset(new RtMidiOut);

        m_portIn->openPort(inPort);
        m_portOut->openPort(outPort);
    } catch (RtMidiError &error) {
        error.printMessage();
    }
}


void MidiDevice::close() {
    try {
        if (m_portIn) if (m_portIn->isPortOpen()) {
            m_portIn->cancelCallback();
            m_portIn->closePort();
            m_portIn.reset();
        }

        if (m_portOut) if (m_portOut->isPortOpen()) {
            m_portOut->closePort();
            m_portOut.reset();
        }
    } catch (RtMidiError &error) {
        error.printMessage();
    }
}


void MidiDevice::identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    if (message->size() < 6 ||
        (*message)[0] != 0xF0 ||
        (*message)[1] != 0x7E ||
        (*message)[3] != 0x06 ||
        (*message)[4] != 0x02) return;
    
    auto device = static_cast<MidiDevice*>(userData);

    device->handleIdentityResponse(message);
}


void MidiDevice::handleIdentityResponse(std::vector<unsigned char> *message) {
    for (auto device_type : MidiDeviceDB::KNOWN_DEVICES) {
        auto &identifier = device_type.second;

        // - Manufacturer (NOVATION)
        // * Device ID
        //                -------- **
        // f0 7e 00 06 02 00 20 29 51 00 00 00 00 63 66 79 f7
        //                5  6  7  8

        bool match = true;
        int check_index = 5;
        for (int i = 0; i < identifier.manufacturerId.size(); i++) {
            if (identifier.manufacturerId.at(i) == message->at(check_index + i)) {
                check_index++;
            } else {
                match = false;
                break;
            }
        }

        for (int i = 0; i < identifier.deviceCode.size(); i++) {
            if (identifier.manufacturerId.at(i) == message->at(check_index + i)) {
                check_index++;
            } else {
                match = false;
                break;
            }
        }

        if (match) {
            std::cout << "Device matches: " << device_type.first << '\n';
        } else {
            std::cout << "Device does not match: " << device_type.first << '\n';
        }
    }
}
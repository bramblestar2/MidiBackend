#include "mididevice.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <rtmidi/RtMidi.h>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>

MidiDevice::MidiDevice(const int& inPort, const int& outPort)
    : m_inPort(-1)
    , m_outPort(-1)
{
    this->open(inPort, outPort);
    this->verify();
}


MidiDevice::~MidiDevice() {
    this->close();
}


void MidiDevice::verify() {
    std::vector<unsigned char> sysex_identity_request = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};
    m_portOut->sendMessage(&sysex_identity_request);
}


void MidiDevice::open(const int& inPort, const int& outPort) {
    try {
        this->close();

        m_portIn.reset(new RtMidiIn);
        m_portOut.reset(new RtMidiOut);

        m_portIn->setCallback(MidiDevice::identityCallback, this);

        m_portIn->ignoreTypes(false, true, true);

        m_portIn->openPort(inPort);
        m_portOut->openPort(outPort);

        m_inPort = inPort;
        m_outPort = outPort;
    } catch (RtMidiError &error) {
        spdlog::error(error.getMessage());
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

        m_inPort = -1;
        m_outPort = -1;
    } catch (RtMidiError &error) {
        spdlog::error(error.getMessage());
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
    // - Manufacturer (NOVATION)
    // * Device ID
    //                -------- **
    // f0 7e 00 06 02 00 20 29 51 00 00 00 00 63 66 79 f7
    //                5  6  7  8

    {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (auto byte : *message) {
            oss << std::setw(2) << static_cast<int>(byte) << ' ';
        }

        spdlog::debug(oss.str());
    }

    constexpr size_t headerSize = 5, footerSize = 1;
    if (message->size() < headerSize + footerSize) {
        spdlog::error("Message too short.");
        return;
    }

    auto payloadBegin = message->begin() + headerSize;
    auto payloadEnd = message->end() - footerSize;

    for (auto const& [deviceName, id] : MidiDeviceDB::KNOWN_DEVICES) {
        const auto& man = id.manufacturerId;
        const auto& dev = id.deviceCode;

        if (std::distance(payloadBegin, payloadEnd) <
            static_cast<std::ptrdiff_t>(man.size() + dev.size())) {
            spdlog::debug(deviceName + ": payload too small");
            continue;
        }

        bool manOk = std::equal(
            payloadBegin,
            payloadBegin + man.size(),
            man.begin()
        );

        if (!manOk) {
            spdlog::warn(deviceName + ": manufacturer mismatch\n");
            continue;
        }

        bool devOk = std::equal(
            payloadBegin + man.size(),
            payloadBegin + man.size() + dev.size(),
            dev.begin()
        );

        devOk 
            ? spdlog::debug("Device matches: " + deviceName) 
            : spdlog::warn(deviceName + ": device code mismatch");
    }
}
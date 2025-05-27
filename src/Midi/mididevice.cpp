#include "mididevice.h"
#include "midimessage.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <mutex>
#include <regex>
#include <rtmidi/RtMidi.h>
#include <iomanip>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <unordered_map>

namespace {
    std::unordered_map<std::string, int> device_type_count;

    std::mutex ns_mutex;


    void AddDeviceCount(std::string name) {
        std::lock_guard<std::mutex> lock(ns_mutex);
        
        if (device_type_count.find(name) == device_type_count.end()) device_type_count[name] = 0;

        device_type_count[name] += 1;
    }

    void RemoveDeviceCount(std::string name) {
        std::lock_guard<std::mutex> lock(ns_mutex);

        name = std::regex_replace(name, std::regex(" \\(\\d\\)+$"), "");

        if (device_type_count.find(name) == device_type_count.end()) return;

        if (device_type_count[name] > 0)
            device_type_count[name]--;

        if (device_type_count[name] == 0)
            device_type_count.erase(name);
    }


    const std::string GetNameWithCount(std::string name) {
        if (device_type_count.find(name) == device_type_count.end()) return name;

        return name + " (" + std::to_string(device_type_count.at(name)) + ")";
    }
    
    
    void ResetDeviceCount() noexcept {
        device_type_count.clear();
    }
}


MidiDevice::MidiDevice(const int& inPort, const int& outPort)
    : m_inPort(-1)
    , m_outPort(-1)
{
    this->open(inPort, outPort);
    this->verify();
}


MidiDevice::~MidiDevice() {
    this->close();

    RemoveDeviceCount(this->m_name);
}


void MidiDevice::verify() {
    m_available = Availability::VERIFYING;

    std::vector<unsigned char> sysex_identity_request = {0xF0, 0x7E, 0x7F, 0x06, 0x01, 0xF7};
    m_midiOut->sendMessage(&sysex_identity_request);
}


void MidiDevice::open(const int& inPort, const int& outPort) {
    try {
        this->close();

        m_midiIn.reset(new RtMidiIn);
        m_midiOut.reset(new RtMidiOut);

        m_midiIn->setCallback(MidiDevice::identityCallback, this);

        m_midiIn->ignoreTypes(false, true, true);

        m_midiIn->openPort(inPort);
        m_midiOut->openPort(outPort);

        m_inPort = inPort;
        m_outPort = outPort;
    } catch (RtMidiError &error) {
        spdlog::error(error.getMessage());
    }
}


void MidiDevice::close() {
    try {
        if (m_midiIn) if (m_midiIn->isPortOpen()) {
            m_midiIn->cancelCallback();
            m_midiIn->closePort();
            m_midiIn.reset();
        }

        if (m_midiOut) if (m_midiOut->isPortOpen()) {
            m_midiOut->closePort();
            m_midiOut.reset();
        }

        m_inPort = -1;
        m_outPort = -1;
    } catch (RtMidiError &error) {
        spdlog::error(error.getMessage());
    }
}


void MidiDevice::identityCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    
    auto device = static_cast<MidiDevice*>(userData);

    if (message->size() == 3) return;

    if (message->size() < 6 ||
        (*message)[0] != 0xF0 ||
        (*message)[1] != 0x7E ||
        (*message)[3] != 0x06 ||
        (*message)[4] != 0x02) {
        std::lock_guard<std::mutex> lock(device->m_mutex);
        device->m_available = Availability::UNAVAILABLE;
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(device->m_mutex);
        device->m_identity = *message;
    }

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

        if (devOk) {
            std::lock_guard<std::mutex> lock(m_mutex);

            this->m_available = Availability::AVAILABLE;
            
            this->m_name = GetNameWithCount(deviceName);
            AddDeviceCount(deviceName);

            spdlog::debug("Device matches: " + this->m_name);

            m_midiIn->cancelCallback();
            m_midiIn->setCallback(MidiDevice::midiCallback, this);

            if (this->m_verifyCallback) {
                (*m_verifyCallback)();
            }

            break;
        }
    }
}


void MidiDevice::midiCallback(double deltaTime, std::vector<unsigned char> *message, void *userData) {
    if (message->size() != 3) {
        return;
    }

    auto device = static_cast<MidiDevice*>(userData);
    device->handleButtonResponse(message);
}


void MidiDevice::handleButtonResponse(std::vector<unsigned char> *message) {
    if (this->m_keyCallback) {
        MidiMessage msg;
        msg.status = message->at(0);
        msg.key = message->at(1);
        msg.velocity = message->at(2);
        (*m_keyCallback)(msg);
    }
}


const std::string& MidiDevice::name() const {
    return this->m_name;
}


void MidiDevice::setKeyCallback(std::function<void(MidiMessage)> callback) {
    m_keyCallback.reset();
    m_keyCallback = callback;
}


void MidiDevice::setVerifyCallback(std::function<void()> callback)  {
    m_verifyCallback.reset();
    m_verifyCallback = callback;
}
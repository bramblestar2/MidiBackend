#include "midimanager.h"
#include "mididevice.h"
#include "midimessage.h"
#include <cctype>
#include <iterator>
#include <memory>
#include <regex>
#include <rtmidi/RtMidi.h>
#include <spdlog/spdlog.h>
#include <regex>
#include <algorithm>
#include <vector>


namespace {
    constexpr int VERIFICATION_TIMEOUT = 2;
}

MidiManager::MidiManager() {
}


MidiManager::~MidiManager() {
    m_devices.clear();

    if (m_midiin.isPortOpen()) m_midiin.closePort();
    if (m_midiout.isPortOpen()) m_midiout.closePort();
}


void MidiManager::verifyIdentity(unsigned int inPort, unsigned int outPort,
                                 const std::vector<unsigned char>& targetId) 
{
    try {
        std::shared_ptr<MidiDevice> device = std::make_shared<MidiDevice>(inPort, outPort);

        m_devices.push_back(device);
    } catch (RtMidiError &error) {
        error.printMessage();
    }
}


bool MidiManager::portsMatch(std::string in, std::string out) {
    // Go to Uppercase
    std::transform(in.begin(), in.end(), in.begin(), ::toupper);
    std::transform(out.begin(), out.end(), out.begin(), ::toupper);
    
    //Remove index number at the end
    in = std::regex_replace(in, std::regex("\\d+$"), "");
    out = std::regex_replace(out, std::regex("\\d+$"), "");

    // Remove IN AND OUT from the names
    in = std::regex_replace(in, std::regex("IN"), "");
    out = std::regex_replace(out, std::regex("OUT"), "");

    return in == out;
}


void MidiManager::refresh() { 
    spdlog::debug("MidiManager: Refreshing Midi Devices");

    try {
        int inPortCount = m_midiin.getPortCount();
        int outPortCount = m_midiout.getPortCount();

        for (int i = 0; i < inPortCount; i++) {
            std::string inPortName = m_midiin.getPortName(i);

            for (int j = 0; j < outPortCount; j++) {
                std::string outPortName = m_midiout.getPortName(j);

                if (this->portsMatch(inPortName, outPortName)) {
                    spdlog::debug("Ports Match! (" + std::to_string(i) + ", " + std::to_string(j) + ", " + inPortName + ", " + outPortName + ")");
                    this->verifyIdentity(i, j, {});
                }
            }
        }
    } catch (const RtMidiError &error) {
        spdlog::error("Midi Error: {}", error.getMessage());
        return;
    }


    this->setupDeviceCallbacks();
}


const std::vector<std::shared_ptr<MidiDevice>> MidiManager::getAvailableDevices() const {
    std::vector<std::shared_ptr<MidiDevice>> devices;
    devices.reserve(m_devices.size());

    std::copy_if(m_devices.begin(), m_devices.end(), std::back_inserter(devices),  
    [](std::shared_ptr<MidiDevice> a) {
        return a->is_available() == MidiDevice::AVAILABLE;
    });
    
    return devices;
}


void MidiManager::setupDeviceCallbacks() {
    for (int i = 0; i < m_devices.size(); i++) {
        auto &device = m_devices.at(i);

        device->setKeyCallback([&device, this](MidiMessage msg) {
            if (m_midiCallback) {
                (*this->m_midiCallback)(device, msg);
            }
        });
    }
}


void MidiManager::setMidiCallback(std::function<void(std::shared_ptr<MidiDevice>, MidiMessage)> function) {
    this->m_midiCallback = function;
}
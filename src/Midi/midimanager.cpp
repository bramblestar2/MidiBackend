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

        // Makes sure callback is called only when all devices have finished verifying
        device->setVerifyCallback([this] () {
            bool all_finished = true;
            for (auto &d : m_devices) {
                if (d->is_available() == MidiDevice::Availability::VERIFYING) {
                    all_finished = false;
                    break;
                }
            }

            if (all_finished && this->m_devicesRefreshCallback) {
                this->m_devicesRefreshCallback();
            }
        });

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
    // in = std::regex_replace(in, std::regex("\\d+$"), "");
    // out = std::regex_replace(out, std::regex("\\d+$"), "");

    // Remove IN AND OUT from the names
    in = std::regex_replace(in, std::regex("IN"), "");
    out = std::regex_replace(out, std::regex("OUT"), "");

    // TODO: This is a temporary fix to windows midi matching
    // a possible issue im thinking of thatll happen is of multiple devices sharing the same name being plugged in


#ifdef _WIN32
    in = std::regex_replace(in, std::regex("\\d$"), "");
    out = std::regex_replace(out, std::regex("\\d$"), "");
#endif

    

    // std::cout << "Device Match: " << in << " - " << out << "\n";

    return in == out;
}


void MidiManager::refresh() { 
    spdlog::debug("MidiManager: Refreshing Midi Devices");
    this->m_devices.clear();

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


std::vector<MidiDevice*> MidiManager::getDevices() const {
    std::vector<MidiDevice*> result;
    result.reserve(m_devices.size());
    std::transform(m_devices.begin(), m_devices.end(), std::back_inserter(result), 
    [](std::shared_ptr<MidiDevice> a) {
        return a.get();
    });
    
    return result;
}


std::vector<MidiDevice*> MidiManager::getAvailableDevices() const {
    std::vector<MidiDevice*> result;
    
    for (auto &device : m_devices) {
        if (device->is_available()) {
            result.push_back(device.get());
        }
    }
    
    return result;
}


void MidiManager::setupDeviceCallbacks() {
    for (int i = 0; i < m_devices.size(); i++) {
        auto &device = m_devices.at(i);

        device->setKeyCallback([&device, this](MidiMessage msg) {

            if (m_midiCallback) {
                this->m_midiCallback(device.get(), msg);
            }

        });
    }
}


void MidiManager::setMidiCallback(std::function<void(MidiDevice*, MidiMessage)> function) {
    this->m_midiCallback = function;
}


void MidiManager::setDevicesRefreshCallback(std::function<void()> &&callback) { 
    m_devicesRefreshCallback = std::move(callback);
}

void MidiManager::startRecording() {
    for (auto &device : this->getAvailableDevices()) {
        device->startRecording();
    }
}

void MidiManager::stopRecording() {
    for (auto &device : this->getAvailableDevices()) {
        device->stopRecording();
    }
}

std::vector<std::pair<std::string, std::vector<MidiMessage>>> MidiManager::getRecordings() const
{
    std::vector<std::pair<std::string, std::vector<MidiMessage>>> result;
    for (auto &device : m_devices) {
        const auto& recordings = device->getRecording();
        if (recordings.empty()) continue;
        
        result.push_back(std::make_pair(device->name(), recordings));
    }
    return result;
}

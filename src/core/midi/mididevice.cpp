#include "mididevice.h"

MidiDevice::MidiDevice(std::string deviceName, std::vector<unsigned char> identity)
    : m_identity(identity)
    , m_name(deviceName)
{
}

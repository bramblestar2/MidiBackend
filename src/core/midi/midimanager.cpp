#include "midimanager.h"

MidiManager::MidiManager() {}

std::vector<MidiDevice::PortPair> MidiManager::getPortPairs() { return m_ports; }

bool MidiManager::verifyIdentity(unsigned int inPort,
                                 const std::vector<unsigned char>& targetId) {
    std::vector<unsigned char> id;
    // m_ports[inPort].portIn->getPortName(id);
    return id == targetId;
}

void MidiManager::refresh() { m_availableDevices = getPortPairs(); }
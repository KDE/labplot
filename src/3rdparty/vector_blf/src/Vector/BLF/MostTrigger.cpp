// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostTrigger.h>

namespace Vector {
namespace BLF {

MostTrigger::MostTrigger() :
    ObjectHeader2(ObjectType::MOST_TRIGGER) {
}

void MostTrigger::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMostTrigger), sizeof(reservedMostTrigger));
    is.read(reinterpret_cast<char *>(&mode), sizeof(mode));
    is.read(reinterpret_cast<char *>(&hw), sizeof(hw));
    is.read(reinterpret_cast<char *>(&previousTriggerValue), sizeof(previousTriggerValue));
    is.read(reinterpret_cast<char *>(&currentTriggerValue), sizeof(currentTriggerValue));
}

void MostTrigger::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMostTrigger), sizeof(reservedMostTrigger));
    os.write(reinterpret_cast<char *>(&mode), sizeof(mode));
    os.write(reinterpret_cast<char *>(&hw), sizeof(hw));
    os.write(reinterpret_cast<char *>(&previousTriggerValue), sizeof(previousTriggerValue));
    os.write(reinterpret_cast<char *>(&currentTriggerValue), sizeof(currentTriggerValue));
}

uint32_t MostTrigger::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMostTrigger) +
        sizeof(mode) +
        sizeof(hw) +
        sizeof(previousTriggerValue) +
        sizeof(currentTriggerValue);
}

}
}

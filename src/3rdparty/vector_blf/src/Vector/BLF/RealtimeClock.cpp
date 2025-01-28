// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/RealtimeClock.h>

namespace Vector {
namespace BLF {

RealtimeClock::RealtimeClock() :
    ObjectHeader(ObjectType::REALTIMECLOCK) {
}

void RealtimeClock::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&time), sizeof(time));
    is.read(reinterpret_cast<char *>(&loggingOffset), sizeof(loggingOffset));
}

void RealtimeClock::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&time), sizeof(time));
    os.write(reinterpret_cast<char *>(&loggingOffset), sizeof(loggingOffset));
}

uint32_t RealtimeClock::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(time) +
        sizeof(loggingOffset);
}

}
}

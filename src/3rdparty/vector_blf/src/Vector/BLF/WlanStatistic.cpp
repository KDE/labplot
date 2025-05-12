// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/WlanStatistic.h>

namespace Vector {
namespace BLF {

WlanStatistic::WlanStatistic() :
    ObjectHeader(ObjectType::WLAN_STATISTIC) {
}

void WlanStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&rxPacketCount), sizeof(rxPacketCount));
    is.read(reinterpret_cast<char *>(&rxByteCount), sizeof(rxByteCount));
    is.read(reinterpret_cast<char *>(&txPacketCount), sizeof(txPacketCount));
    is.read(reinterpret_cast<char *>(&txByteCount), sizeof(txByteCount));
    is.read(reinterpret_cast<char *>(&collisionCount), sizeof(collisionCount));
    is.read(reinterpret_cast<char *>(&errorCount), sizeof(errorCount));
    is.read(reinterpret_cast<char *>(&reservedWlanStatistic), sizeof(reservedWlanStatistic));
    // @note might be extended in future versions
}

void WlanStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&rxPacketCount), sizeof(rxPacketCount));
    os.write(reinterpret_cast<char *>(&rxByteCount), sizeof(rxByteCount));
    os.write(reinterpret_cast<char *>(&txPacketCount), sizeof(txPacketCount));
    os.write(reinterpret_cast<char *>(&txByteCount), sizeof(txByteCount));
    os.write(reinterpret_cast<char *>(&collisionCount), sizeof(collisionCount));
    os.write(reinterpret_cast<char *>(&errorCount), sizeof(errorCount));
    os.write(reinterpret_cast<char *>(&reservedWlanStatistic), sizeof(reservedWlanStatistic));
}

uint32_t WlanStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(rxPacketCount) +
        sizeof(rxByteCount) +
        sizeof(txPacketCount) +
        sizeof(txByteCount) +
        sizeof(collisionCount) +
        sizeof(errorCount) +
        sizeof(reservedWlanStatistic);
}

}
}

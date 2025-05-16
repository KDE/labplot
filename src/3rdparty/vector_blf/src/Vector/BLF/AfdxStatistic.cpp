// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxStatistic.h>

namespace Vector {
namespace BLF {

AfdxStatistic::AfdxStatistic() :
    ObjectHeader(ObjectType::AFDX_STATISTIC) {
}

void AfdxStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&rxPacketCount), sizeof(rxPacketCount));
    is.read(reinterpret_cast<char *>(&rxByteCount), sizeof(rxByteCount));
    is.read(reinterpret_cast<char *>(&txPacketCount), sizeof(txPacketCount));
    is.read(reinterpret_cast<char *>(&txByteCount), sizeof(txByteCount));
    is.read(reinterpret_cast<char *>(&collisionCount), sizeof(collisionCount));
    is.read(reinterpret_cast<char *>(&errorCount), sizeof(errorCount));
    is.read(reinterpret_cast<char *>(&statDroppedRedundantPacketCount), sizeof(statDroppedRedundantPacketCount));
    is.read(reinterpret_cast<char *>(&statRedundantErrorPacketCount), sizeof(statRedundantErrorPacketCount));
    is.read(reinterpret_cast<char *>(&statIntegrityErrorPacketCount), sizeof(statIntegrityErrorPacketCount));
    is.read(reinterpret_cast<char *>(&statAvrgPeriodMsec), sizeof(statAvrgPeriodMsec));
    is.read(reinterpret_cast<char *>(&statAvrgJitterMysec), sizeof(statAvrgJitterMysec));
    is.read(reinterpret_cast<char *>(&vlid), sizeof(vlid));
    is.read(reinterpret_cast<char *>(&statDuration), sizeof(statDuration));
    // @note might be extended in future versions
}

void AfdxStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&rxPacketCount), sizeof(rxPacketCount));
    os.write(reinterpret_cast<char *>(&rxByteCount), sizeof(rxByteCount));
    os.write(reinterpret_cast<char *>(&txPacketCount), sizeof(txPacketCount));
    os.write(reinterpret_cast<char *>(&txByteCount), sizeof(txByteCount));
    os.write(reinterpret_cast<char *>(&collisionCount), sizeof(collisionCount));
    os.write(reinterpret_cast<char *>(&errorCount), sizeof(errorCount));
    os.write(reinterpret_cast<char *>(&statDroppedRedundantPacketCount), sizeof(statDroppedRedundantPacketCount));
    os.write(reinterpret_cast<char *>(&statRedundantErrorPacketCount), sizeof(statRedundantErrorPacketCount));
    os.write(reinterpret_cast<char *>(&statIntegrityErrorPacketCount), sizeof(statIntegrityErrorPacketCount));
    os.write(reinterpret_cast<char *>(&statAvrgPeriodMsec), sizeof(statAvrgPeriodMsec));
    os.write(reinterpret_cast<char *>(&statAvrgJitterMysec), sizeof(statAvrgJitterMysec));
    os.write(reinterpret_cast<char *>(&vlid), sizeof(vlid));
    os.write(reinterpret_cast<char *>(&statDuration), sizeof(statDuration));
}

uint32_t AfdxStatistic::calculateObjectSize() const {
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
        sizeof(statDroppedRedundantPacketCount) +
        sizeof(statRedundantErrorPacketCount) +
        sizeof(statIntegrityErrorPacketCount) +
        sizeof(statAvrgPeriodMsec) +
        sizeof(statAvrgJitterMysec) +
        sizeof(vlid) +
        sizeof(statDuration);
}

}
}

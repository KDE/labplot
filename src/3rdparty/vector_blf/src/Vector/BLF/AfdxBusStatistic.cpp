// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxBusStatistic.h>

namespace Vector {
namespace BLF {

AfdxBusStatistic::AfdxBusStatistic() :
    ObjectHeader(ObjectType::A429_BUS_STATISTIC) {
}

void AfdxBusStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&statDuration), sizeof(statDuration));
    is.read(reinterpret_cast<char *>(&statRxPacketCountHW), sizeof(statRxPacketCountHW));
    is.read(reinterpret_cast<char *>(&statTxPacketCountHW), sizeof(statTxPacketCountHW));
    is.read(reinterpret_cast<char *>(&statRxErrorCountHW), sizeof(statRxErrorCountHW));
    is.read(reinterpret_cast<char *>(&statTxErrorCountHW), sizeof(statTxErrorCountHW));
    is.read(reinterpret_cast<char *>(&statRxBytesHW), sizeof(statRxBytesHW));
    is.read(reinterpret_cast<char *>(&statTxBytesHW), sizeof(statTxBytesHW));
    is.read(reinterpret_cast<char *>(&statRxPacketCount), sizeof(statRxPacketCount));
    is.read(reinterpret_cast<char *>(&statTxPacketCount), sizeof(statTxPacketCount));
    is.read(reinterpret_cast<char *>(&statDroppedPacketCount), sizeof(statDroppedPacketCount));
    is.read(reinterpret_cast<char *>(&statInvalidPacketCount), sizeof(statInvalidPacketCount));
    is.read(reinterpret_cast<char *>(&statLostPacketCount), sizeof(statLostPacketCount));
    is.read(reinterpret_cast<char *>(&line), sizeof(line));
    is.read(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    is.read(reinterpret_cast<char *>(&linkSpeed), sizeof(linkSpeed));
    is.read(reinterpret_cast<char *>(&linkLost), sizeof(linkLost));
    is.read(reinterpret_cast<char *>(&reservedAfdxBusStatistic1), sizeof(reservedAfdxBusStatistic1));
    is.read(reinterpret_cast<char *>(&reservedAfdxBusStatistic2), sizeof(reservedAfdxBusStatistic2));
    // @note might be extended in future versions
}

void AfdxBusStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&statDuration), sizeof(statDuration));
    os.write(reinterpret_cast<char *>(&statRxPacketCountHW), sizeof(statRxPacketCountHW));
    os.write(reinterpret_cast<char *>(&statTxPacketCountHW), sizeof(statTxPacketCountHW));
    os.write(reinterpret_cast<char *>(&statRxErrorCountHW), sizeof(statRxErrorCountHW));
    os.write(reinterpret_cast<char *>(&statTxErrorCountHW), sizeof(statTxErrorCountHW));
    os.write(reinterpret_cast<char *>(&statRxBytesHW), sizeof(statRxBytesHW));
    os.write(reinterpret_cast<char *>(&statTxBytesHW), sizeof(statTxBytesHW));
    os.write(reinterpret_cast<char *>(&statRxPacketCount), sizeof(statRxPacketCount));
    os.write(reinterpret_cast<char *>(&statTxPacketCount), sizeof(statTxPacketCount));
    os.write(reinterpret_cast<char *>(&statDroppedPacketCount), sizeof(statDroppedPacketCount));
    os.write(reinterpret_cast<char *>(&statInvalidPacketCount), sizeof(statInvalidPacketCount));
    os.write(reinterpret_cast<char *>(&statLostPacketCount), sizeof(statLostPacketCount));
    os.write(reinterpret_cast<char *>(&line), sizeof(line));
    os.write(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    os.write(reinterpret_cast<char *>(&linkSpeed), sizeof(linkSpeed));
    os.write(reinterpret_cast<char *>(&linkLost), sizeof(linkLost));
    os.write(reinterpret_cast<char *>(&reservedAfdxBusStatistic1), sizeof(reservedAfdxBusStatistic1));
    os.write(reinterpret_cast<char *>(&reservedAfdxBusStatistic2), sizeof(reservedAfdxBusStatistic2));
}

uint32_t AfdxBusStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(statDuration) +
        sizeof(statRxPacketCountHW) +
        sizeof(statTxPacketCountHW) +
        sizeof(statRxErrorCountHW) +
        sizeof(statTxErrorCountHW) +
        sizeof(statRxBytesHW) +
        sizeof(statTxBytesHW) +
        sizeof(statRxPacketCount) +
        sizeof(statTxPacketCount) +
        sizeof(statDroppedPacketCount) +
        sizeof(statInvalidPacketCount) +
        sizeof(statLostPacketCount) +
        sizeof(line) +
        sizeof(linkStatus) +
        sizeof(linkSpeed) +
        sizeof(linkLost) +
        sizeof(reservedAfdxBusStatistic1) +
        sizeof(reservedAfdxBusStatistic2);
}

}
}

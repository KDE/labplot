// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/EthernetStatistic.h>

namespace Vector {
namespace BLF {

EthernetStatistic::EthernetStatistic() :
    ObjectHeader(ObjectType::ETHERNET_STATISTIC) {
}

void EthernetStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedEthernetStatistic1), sizeof(reservedEthernetStatistic1));
    is.read(reinterpret_cast<char *>(&reservedEthernetStatistic2), sizeof(reservedEthernetStatistic2));
    is.read(reinterpret_cast<char *>(&rcvOk_HW), sizeof(rcvOk_HW));
    is.read(reinterpret_cast<char *>(&xmitOk_HW), sizeof(xmitOk_HW));
    is.read(reinterpret_cast<char *>(&rcvError_HW), sizeof(rcvError_HW));
    is.read(reinterpret_cast<char *>(&xmitError_HW), sizeof(xmitError_HW));
    is.read(reinterpret_cast<char *>(&rcvBytes_HW), sizeof(rcvBytes_HW));
    is.read(reinterpret_cast<char *>(&xmitBytes_HW), sizeof(xmitBytes_HW));
    is.read(reinterpret_cast<char *>(&rcvNoBuffer_HW), sizeof(rcvNoBuffer_HW));
    is.read(reinterpret_cast<char *>(&sqi), sizeof(sqi));
    is.read(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    is.read(reinterpret_cast<char *>(&reservedEthernetStatistic3), sizeof(reservedEthernetStatistic3));
}

void EthernetStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedEthernetStatistic1), sizeof(reservedEthernetStatistic1));
    os.write(reinterpret_cast<char *>(&reservedEthernetStatistic2), sizeof(reservedEthernetStatistic2));
    os.write(reinterpret_cast<char *>(&rcvOk_HW), sizeof(rcvOk_HW));
    os.write(reinterpret_cast<char *>(&xmitOk_HW), sizeof(xmitOk_HW));
    os.write(reinterpret_cast<char *>(&rcvError_HW), sizeof(rcvError_HW));
    os.write(reinterpret_cast<char *>(&xmitError_HW), sizeof(xmitError_HW));
    os.write(reinterpret_cast<char *>(&rcvBytes_HW), sizeof(rcvBytes_HW));
    os.write(reinterpret_cast<char *>(&xmitBytes_HW), sizeof(xmitBytes_HW));
    os.write(reinterpret_cast<char *>(&rcvNoBuffer_HW), sizeof(rcvNoBuffer_HW));
    os.write(reinterpret_cast<char *>(&sqi), sizeof(sqi));
    os.write(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    os.write(reinterpret_cast<char *>(&reservedEthernetStatistic3), sizeof(reservedEthernetStatistic3));
}

uint32_t EthernetStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedEthernetStatistic1) +
        sizeof(reservedEthernetStatistic2) +
        sizeof(rcvOk_HW) +
        sizeof(xmitOk_HW) +
        sizeof(rcvError_HW) +
        sizeof(xmitError_HW) +
        sizeof(rcvBytes_HW) +
        sizeof(xmitBytes_HW) +
        sizeof(rcvNoBuffer_HW) +
        sizeof(sqi) +
        sizeof(hardwareChannel) +
        sizeof(reservedEthernetStatistic3);
}

}
}

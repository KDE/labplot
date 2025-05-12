// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/EthernetStatus.h>

namespace Vector {
namespace BLF {

EthernetStatus::EthernetStatus() :
    ObjectHeader(ObjectType::ETHERNET_STATUS) {
}

void EthernetStatus::read(AbstractFile & is) {
    apiMajor = 1;
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    is.read(reinterpret_cast<char *>(&ethernetPhy), sizeof(ethernetPhy));
    is.read(reinterpret_cast<char *>(&duplex), sizeof(duplex));
    is.read(reinterpret_cast<char *>(&mdi), sizeof(mdi));
    is.read(reinterpret_cast<char *>(&connector), sizeof(connector));
    is.read(reinterpret_cast<char *>(&clockMode), sizeof(clockMode));
    is.read(reinterpret_cast<char *>(&pairs), sizeof(pairs));
    is.read(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    is.read(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));

    /* the following variables are only available in Version 2 and above */
    if (objectSize > calculateObjectSize()) {
        apiMajor = 2;
        is.read(reinterpret_cast<char *>(&reservedEthernetStatus1), sizeof(reservedEthernetStatus1));
        is.read(reinterpret_cast<char *>(&reservedEthernetStatus2), sizeof(reservedEthernetStatus2));
    }
}

void EthernetStatus::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&linkStatus), sizeof(linkStatus));
    os.write(reinterpret_cast<char *>(&ethernetPhy), sizeof(ethernetPhy));
    os.write(reinterpret_cast<char *>(&duplex), sizeof(duplex));
    os.write(reinterpret_cast<char *>(&mdi), sizeof(mdi));
    os.write(reinterpret_cast<char *>(&connector), sizeof(connector));
    os.write(reinterpret_cast<char *>(&clockMode), sizeof(clockMode));
    os.write(reinterpret_cast<char *>(&pairs), sizeof(pairs));
    os.write(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    os.write(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));

    if (apiMajor < 2)
        return;
    os.write(reinterpret_cast<char *>(&reservedEthernetStatus1), sizeof(reservedEthernetStatus1));
    os.write(reinterpret_cast<char *>(&reservedEthernetStatus2), sizeof(reservedEthernetStatus2));
}

uint32_t EthernetStatus::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(linkStatus) +
        sizeof(ethernetPhy) +
        sizeof(duplex) +
        sizeof(mdi) +
        sizeof(connector) +
        sizeof(clockMode) +
        sizeof(pairs) +
        sizeof(hardwareChannel) +
        sizeof(bitrate);
}

}
}

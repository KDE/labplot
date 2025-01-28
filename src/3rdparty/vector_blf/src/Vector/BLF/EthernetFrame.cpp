// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/EthernetFrame.h>

namespace Vector {
namespace BLF {

EthernetFrame::EthernetFrame() :
    ObjectHeader(ObjectType::ETHERNET_FRAME) {
}

void EthernetFrame::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(sourceAddress.data()), static_cast<std::streamsize>(sourceAddress.size()));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(destinationAddress.data()), static_cast<std::streamsize>(destinationAddress.size()));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&tpid), sizeof(tpid));
    is.read(reinterpret_cast<char *>(&tci), sizeof(tci));
    is.read(reinterpret_cast<char *>(&payLoadLength), sizeof(payLoadLength));
    is.read(reinterpret_cast<char *>(&reservedEthernetFrame), sizeof(reservedEthernetFrame));
    payLoad.resize(payLoadLength);
    is.read(reinterpret_cast<char *>(payLoad.data()), payLoadLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void EthernetFrame::write(AbstractFile & os) {
    /* pre processing */
    payLoadLength = static_cast<uint16_t>(payLoad.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(sourceAddress.data()), static_cast<std::streamsize>(sourceAddress.size()));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(destinationAddress.data()), static_cast<std::streamsize>(destinationAddress.size()));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&tpid), sizeof(tpid));
    os.write(reinterpret_cast<char *>(&tci), sizeof(tci));
    os.write(reinterpret_cast<char *>(&payLoadLength), sizeof(payLoadLength));
    os.write(reinterpret_cast<char *>(&reservedEthernetFrame), sizeof(reservedEthernetFrame));
    os.write(reinterpret_cast<char *>(payLoad.data()), payLoadLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t EthernetFrame::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        static_cast<uint32_t>(sourceAddress.size()) +
        sizeof(channel) +
        static_cast<uint32_t>(destinationAddress.size()) +
        sizeof(dir) +
        sizeof(type) +
        sizeof(tpid) +
        sizeof(tci) +
        sizeof(payLoadLength) +
        sizeof(reservedEthernetFrame) +
        payLoadLength;
}

}
}

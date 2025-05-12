// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostEthernetPkt.h>

namespace Vector {
namespace BLF {

MostEthernetPkt::MostEthernetPkt() :
    ObjectHeader2(ObjectType::MOST_ETHERNET_PKT) {
}

void MostEthernetPkt::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPkt1), sizeof(reservedMostEthernetPkt1));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPkt2), sizeof(reservedMostEthernetPkt2));
    is.read(reinterpret_cast<char *>(&sourceMacAdr), sizeof(sourceMacAdr));
    is.read(reinterpret_cast<char *>(&destMacAdr), sizeof(destMacAdr));
    is.read(reinterpret_cast<char *>(&transferType), sizeof(transferType));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPkt3), sizeof(reservedMostEthernetPkt3));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    is.read(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPkt4), sizeof(reservedMostEthernetPkt4));
    is.read(reinterpret_cast<char *>(&pktDataLength), sizeof(pktDataLength));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPkt5), sizeof(reservedMostEthernetPkt5));
    pktData.resize(pktDataLength);
    is.read(reinterpret_cast<char *>(pktData.data()), pktDataLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void MostEthernetPkt::write(AbstractFile & os) {
    /* pre processing */
    pktDataLength = static_cast<uint32_t>(pktData.size());

    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPkt1), sizeof(reservedMostEthernetPkt1));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPkt2), sizeof(reservedMostEthernetPkt2));
    os.write(reinterpret_cast<char *>(&sourceMacAdr), sizeof(sourceMacAdr));
    os.write(reinterpret_cast<char *>(&destMacAdr), sizeof(destMacAdr));
    os.write(reinterpret_cast<char *>(&transferType), sizeof(transferType));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPkt3), sizeof(reservedMostEthernetPkt3));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    os.write(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPkt4), sizeof(reservedMostEthernetPkt4));
    os.write(reinterpret_cast<char *>(&pktDataLength), sizeof(pktDataLength));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPkt5), sizeof(reservedMostEthernetPkt5));
    os.write(reinterpret_cast<char *>(pktData.data()), pktDataLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t MostEthernetPkt::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedMostEthernetPkt1) +
        sizeof(reservedMostEthernetPkt2) +
        sizeof(sourceMacAdr) +
        sizeof(destMacAdr) +
        sizeof(transferType) +
        sizeof(state) +
        sizeof(ackNack) +
        sizeof(reservedMostEthernetPkt3) +
        sizeof(crc) +
        sizeof(pAck) +
        sizeof(cAck) +
        sizeof(reservedMostEthernetPkt4) +
        sizeof(pktDataLength) +
        sizeof(reservedMostEthernetPkt5) +
        pktDataLength;
}

}
}

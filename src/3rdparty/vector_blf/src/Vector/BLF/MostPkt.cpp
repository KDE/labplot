// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostPkt.h>

namespace Vector {
namespace BLF {

MostPkt::MostPkt() :
    ObjectHeader(ObjectType::MOST_PKT) {
}

void MostPkt::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedMostPkt1), sizeof(reservedMostPkt1));
    is.read(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    is.read(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    is.read(reinterpret_cast<char *>(&arbitration), sizeof(arbitration));
    is.read(reinterpret_cast<char *>(&timeRes), sizeof(timeRes));
    is.read(reinterpret_cast<char *>(&quadsToFollow), sizeof(quadsToFollow));
    is.read(reinterpret_cast<char *>(&reservedMostPkt2), sizeof(reservedMostPkt2));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&priority), sizeof(priority));
    is.read(reinterpret_cast<char *>(&transferType), sizeof(transferType));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedMostPkt3), sizeof(reservedMostPkt3));
    is.read(reinterpret_cast<char *>(&reservedMostPkt4), sizeof(reservedMostPkt4));
    is.read(reinterpret_cast<char *>(&pktDataLength), sizeof(pktDataLength));
    is.read(reinterpret_cast<char *>(&reservedMostPkt5), sizeof(reservedMostPkt5));
    pktData.resize(pktDataLength);
    is.read(reinterpret_cast<char *>(pktData.data()), pktDataLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void MostPkt::write(AbstractFile & os) {
    /* pre processing */
    pktDataLength = static_cast<uint32_t>(pktData.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedMostPkt1), sizeof(reservedMostPkt1));
    os.write(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    os.write(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    os.write(reinterpret_cast<char *>(&arbitration), sizeof(arbitration));
    os.write(reinterpret_cast<char *>(&timeRes), sizeof(timeRes));
    os.write(reinterpret_cast<char *>(&quadsToFollow), sizeof(quadsToFollow));
    os.write(reinterpret_cast<char *>(&reservedMostPkt2), sizeof(reservedMostPkt2));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&priority), sizeof(priority));
    os.write(reinterpret_cast<char *>(&transferType), sizeof(transferType));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedMostPkt3), sizeof(reservedMostPkt3));
    os.write(reinterpret_cast<char *>(&reservedMostPkt4), sizeof(reservedMostPkt4));
    os.write(reinterpret_cast<char *>(&pktDataLength), sizeof(pktDataLength));
    os.write(reinterpret_cast<char *>(&reservedMostPkt5), sizeof(reservedMostPkt5));
    os.write(reinterpret_cast<char *>(pktData.data()), pktDataLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t MostPkt::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedMostPkt1) +
        sizeof(sourceAdr) +
        sizeof(destAdr) +
        sizeof(arbitration) +
        sizeof(timeRes) +
        sizeof(quadsToFollow) +
        sizeof(reservedMostPkt2) +
        sizeof(crc) +
        sizeof(priority) +
        sizeof(transferType) +
        sizeof(state) +
        sizeof(reservedMostPkt3) +
        sizeof(reservedMostPkt4) +
        sizeof(pktDataLength) +
        sizeof(reservedMostPkt5) +
        pktDataLength;
}

}
}

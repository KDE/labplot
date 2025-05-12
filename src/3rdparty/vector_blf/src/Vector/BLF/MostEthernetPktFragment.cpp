// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostEthernetPktFragment.h>

namespace Vector {
namespace BLF {

MostEthernetPktFragment::MostEthernetPktFragment() :
    ObjectHeader2(ObjectType::MOST_ETHERNET_PKT_FRAGMENT) {
}

void MostEthernetPktFragment::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPktFragment1), sizeof(reservedMostEthernetPktFragment1));
    is.read(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    is.read(reinterpret_cast<char *>(&validMask), sizeof(validMask));
    is.read(reinterpret_cast<char *>(&sourceMacAdr), sizeof(sourceMacAdr));
    is.read(reinterpret_cast<char *>(&destMacAdr), sizeof(destMacAdr));
    is.read(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    is.read(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPktFragment2), sizeof(reservedMostEthernetPktFragment2));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    is.read(reinterpret_cast<char *>(&dataLenAnnounced), sizeof(dataLenAnnounced));
    is.read(reinterpret_cast<char *>(&firstDataLen), sizeof(firstDataLen));
    is.read(reinterpret_cast<char *>(&reservedMostEthernetPktFragment3), sizeof(reservedMostEthernetPktFragment3));
    firstData.resize(firstDataLen);
    is.read(reinterpret_cast<char *>(firstData.data()), firstDataLen);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void MostEthernetPktFragment::write(AbstractFile & os) {
    /* pre processing */
    firstDataLen = static_cast<uint32_t>(firstData.size());

    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPktFragment1), sizeof(reservedMostEthernetPktFragment1));
    os.write(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    os.write(reinterpret_cast<char *>(&validMask), sizeof(validMask));
    os.write(reinterpret_cast<char *>(&sourceMacAdr), sizeof(sourceMacAdr));
    os.write(reinterpret_cast<char *>(&destMacAdr), sizeof(destMacAdr));
    os.write(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    os.write(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPktFragment2), sizeof(reservedMostEthernetPktFragment2));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    os.write(reinterpret_cast<char *>(&dataLenAnnounced), sizeof(dataLenAnnounced));
    os.write(reinterpret_cast<char *>(&firstDataLen), sizeof(firstDataLen));
    os.write(reinterpret_cast<char *>(&reservedMostEthernetPktFragment3), sizeof(reservedMostEthernetPktFragment3));
    os.write(reinterpret_cast<char *>(firstData.data()), firstDataLen);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t MostEthernetPktFragment::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMostEthernetPktFragment1) +
        sizeof(ackNack) +
        sizeof(validMask) +
        sizeof(sourceMacAdr) +
        sizeof(destMacAdr) +
        sizeof(pAck) +
        sizeof(cAck) +
        sizeof(reservedMostEthernetPktFragment2) +
        sizeof(crc) +
        sizeof(dataLen) +
        sizeof(dataLenAnnounced) +
        sizeof(firstDataLen) +
        sizeof(reservedMostEthernetPktFragment3) +
        firstDataLen;
}

}
}

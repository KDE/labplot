// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/Most150MessageFragment.h>

namespace Vector {
namespace BLF {

Most150MessageFragment::Most150MessageFragment() :
    ObjectHeader2(ObjectType::MOST_150_MESSAGE_FRAGMENT) {
}

void Most150MessageFragment::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMost150MessageFragment1), sizeof(reservedMost150MessageFragment1));
    is.read(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    is.read(reinterpret_cast<char *>(&validMask), sizeof(validMask));
    is.read(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    is.read(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    is.read(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    is.read(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    is.read(reinterpret_cast<char *>(&priority), sizeof(priority));
    is.read(reinterpret_cast<char *>(&pIndex), sizeof(pIndex));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    is.read(reinterpret_cast<char *>(&dataLenAnnounced), sizeof(dataLenAnnounced));
    is.read(reinterpret_cast<char *>(&firstDataLen), sizeof(firstDataLen));
    is.read(reinterpret_cast<char *>(&reservedMost150MessageFragment2), sizeof(reservedMost150MessageFragment2));
    firstData.resize(firstDataLen);
    is.read(reinterpret_cast<char *>(firstData.data()), firstDataLen);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void Most150MessageFragment::write(AbstractFile & os) {
    /* pre processing */
    firstDataLen = static_cast<uint32_t>(firstData.size());

    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMost150MessageFragment1), sizeof(reservedMost150MessageFragment1));
    os.write(reinterpret_cast<char *>(&ackNack), sizeof(ackNack));
    os.write(reinterpret_cast<char *>(&validMask), sizeof(validMask));
    os.write(reinterpret_cast<char *>(&sourceAdr), sizeof(sourceAdr));
    os.write(reinterpret_cast<char *>(&destAdr), sizeof(destAdr));
    os.write(reinterpret_cast<char *>(&pAck), sizeof(pAck));
    os.write(reinterpret_cast<char *>(&cAck), sizeof(cAck));
    os.write(reinterpret_cast<char *>(&priority), sizeof(priority));
    os.write(reinterpret_cast<char *>(&pIndex), sizeof(pIndex));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dataLen), sizeof(dataLen));
    os.write(reinterpret_cast<char *>(&dataLenAnnounced), sizeof(dataLenAnnounced));
    os.write(reinterpret_cast<char *>(&firstDataLen), sizeof(firstDataLen));
    os.write(reinterpret_cast<char *>(&reservedMost150MessageFragment2), sizeof(reservedMost150MessageFragment2));
    os.write(reinterpret_cast<char *>(firstData.data()), firstDataLen);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t Most150MessageFragment::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMost150MessageFragment1) +
        sizeof(ackNack) +
        sizeof(validMask) +
        sizeof(sourceAdr) +
        sizeof(destAdr) +
        sizeof(pAck) +
        sizeof(cAck) +
        sizeof(priority) +
        sizeof(pIndex) +
        sizeof(crc) +
        sizeof(dataLen) +
        sizeof(dataLenAnnounced) +
        sizeof(firstDataLen) +
        sizeof(reservedMost150MessageFragment2) +
        firstDataLen;
}

}
}
